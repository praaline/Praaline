#include <QDebug>
#include <QList>
#include <QString>
#include <QStringList>
#include <QProcess>
#include <QTemporaryDir>
#include <QTextStream>
#include <QCoreApplication>

#include "pncore/annotation/IntervalTier.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/interfaces/praat/PraatTextGrid.h"
using namespace Praaline::Core;

#include "pnlib/media/AudioSegmenter.h"
using namespace Praaline::Media;

#include "SpeechToken.h"
#include "HTKForcedAligner.h"
#include "phonetiser/ExternalPhonetiser.h"

namespace Praaline {
namespace ASR {

struct HTKForcedAlignerData {
    HTKForcedAlignerData() :
        useAlternativePronunciations(true), useOptionalElision(true)
    {}

    QString filenameCFG;
    QString filenameHMM;
    QString filenamePhoneList;
    uint sampleRateAM;
    uint beamThreshold;
    QHash<QString, QString> phonemeTranslations;
    QHash<QString, QString> phonemeReverseTranslations;
    QString pathTemp;
    QStringList phonemes;
    QRegExp regexMatchPhoneme;
    bool useAlternativePronunciations;
    bool useOptionalElision;
    QString tokenPhonetisationAttributeID;
    QString phonetisationSeparatorForAlternate;
    QString phonetisationSeparatorForPhonemes;
};


HTKForcedAligner::HTKForcedAligner(QObject *parent)
    : QObject(parent), d(new HTKForcedAlignerData)
{
    // DIRECTORY:
    QString modelsPath = QDir::homePath() + "/Praaline/tools/htk/";
    d->filenameCFG = modelsPath + "fra.cfg";
    d->filenameHMM = modelsPath + "fra.hmm";
    d->filenamePhoneList = modelsPath + "fra_phone.list";
    d->sampleRateAM = 16000;
    d->beamThreshold = 500;

    // The order is important. Start with the longest phonemes.
    d->phonemes << "9~" << "a~" << "e~" << "o~"
                << "2" << "9" << "A" << "@" << "E" << "H" << "O" << "R" << "S" << "Z"
                << "a" << "b" << "d" << "e" << "f" << "g" << "i" << "j" << "k" << "l"
                << "m" << "n" << "o" << "p" << "s" << "t" << "u" << "v" << "w" << "y" << "z";
    QString regex;
    foreach (QString phoneme, d->phonemes)
        regex = regex.append(QString("%1\\*|%1|").arg(phoneme));
    if (!regex.isEmpty()) regex.chop(1);
    d->regexMatchPhoneme = QRegExp(regex);

    d->phonemeTranslations.insert("9~", "oe~");
    d->phonemeTranslations.insert("2", "eu");
    d->phonemeTranslations.insert("9", "oe");
    d->phonemeTranslations.insert("A", "a");

    d->phonemeReverseTranslations.insert("oe~", "9~");
    d->phonemeReverseTranslations.insert("eu", "2");
    d->phonemeReverseTranslations.insert("oe", "9");

    d->pathTemp = QDir::homePath() + "/Praaline/aligner_temp/";

    d->tokenPhonetisationAttributeID = "phonetisation";
    d->phonetisationSeparatorForAlternate = "|";
    d->phonetisationSeparatorForPhonemes = " ";
}

HTKForcedAligner::~HTKForcedAligner()
{
    delete d;
}

QString HTKForcedAligner::tokenPhonetisationAttributeID() const
{
    return d->tokenPhonetisationAttributeID;
}

void HTKForcedAligner::setTokenPhonetisationAttributeID(const QString &attributeID)
{
    d->tokenPhonetisationAttributeID = attributeID;
}

// Encode UTF entities (used for accented characters in dictionary and transcription files)
// private
QString HTKForcedAligner::encodeEntities(const QString &src, const QString &force)
{
    QString tmp(src);
    uint len = tmp.length();
    uint i = 0;
    while (i < len) {
        if (tmp[i].unicode() > 128 || force.contains(tmp[i])) {
            QString rp = QString("&#%1;").arg(tmp[i].unicode());
            tmp.replace(i, 1, rp);
            len += rp.length() - 1;
            i += rp.length();
        } else {
            ++i;
        }
    }
    return tmp;
}

// Decode UTF entities (used for accented characters in dictionary and transcription files)
// private
QString HTKForcedAligner::decodeEntities(const QString &src)
{
    QString ret(src);
    QRegExp re("&#([0-9]+);");
    re.setMinimal(true);
    int pos = 0;
    while ((pos = re.indexIn(src, pos)) != -1) {
        ret = ret.replace(re.cap(0), QChar(re.cap(1).toInt(0, 10)));
        pos += re.matchedLength();
    }
    return ret;
}

// private
QString HTKForcedAligner::translatePhonemes(const QString &input)
{
    QString translated(input);
    translated = translated.append(" ");
    foreach (QString original, d->phonemeTranslations.keys()) {
        translated = translated.replace(original + " ", d->phonemeTranslations.value(original) + " ");
    }
    return translated.trimmed();
}

// private
QList<SpeechToken> HTKForcedAligner::alignerTokensFromIntervalTier(bool insertLeadingAndTrailingPauses, IntervalTier *tierTokens, int indexFrom, int indexTo)
{
    QList<SpeechToken> alignerTokens;
    if (!tierTokens) return alignerTokens;
    if (tierTokens->count() == 0) return alignerTokens;
    if (indexTo < 0) indexTo = tierTokens->count() - 1;
    if (indexTo >= tierTokens->count()) indexTo = tierTokens->count() - 1;
    if (indexFrom < 0) indexFrom = 0;
    if (indexTo >= tierTokens->count()) indexTo = tierTokens->count() - 1;
    if (indexTo < indexFrom) return alignerTokens;

    QSharedPointer<IntervalTier> tierTokensCopy(new IntervalTier(tierTokens));

    // On the working copy: Insert silent pauses at the beginning and end, if authorised to do so.
    if (insertLeadingAndTrailingPauses) {
        if (!tierTokensCopy->at(indexFrom)->isPauseSilent()) {
            // Insert pause at the beginning of the utterance:
            // [xxx] becomes [ _ ][xxx]
            tierTokensCopy->split(indexFrom, tierTokensCopy->at(indexFrom)->tCenter(), true);
            tierTokensCopy->at(indexFrom)->setText("_");
            indexTo++;
        }
        if (!tierTokensCopy->at(indexTo)->isPauseSilent()) {
            // Insert pause at the end of the utterance
            // [xxx] becomes [xxx][ _ ]
            tierTokensCopy->split(indexTo, tierTokensCopy->at(indexTo)->tCenter(), false);
            indexTo++;
            tierTokensCopy->at(indexTo)->setText("_");
        }
    }
    // On the working copy: Remove pauses from within the utterance
    for (int i = indexTo - 1; i >= indexFrom + 1; --i) {
        if (tierTokensCopy->at(i)->isPauseSilent()) {
            tierTokensCopy->removeInterval(i);
            indexTo--;
        }
    }
    // Create tokens for alignment
    for (int index = indexFrom; index <= indexTo; ++index) {
        Interval *token = tierTokensCopy->at(index);
        SpeechToken atoken(index, index, token->text());
        QString phonetisationAttributeText = token->attribute(d->tokenPhonetisationAttributeID).toString();
        if (!d->useAlternativePronunciations) {
            phonetisationAttributeText = phonetisationAttributeText.replace("*", "");
        }
        phonetisationAttributeText = phonetisationAttributeText.trimmed();
        // Split the given phonetisation text into proposed alternates (e.g. abc | def)
        QStringList phonetisationsProposed = phonetisationAttributeText.split(d->phonetisationSeparatorForAlternate, QString::SkipEmptyParts);
        // Create list of alternate phonetisations
        foreach (QString phonetisationProposed, phonetisationsProposed) {
            QStringList phonemesSeparated;
            // Separate phonemes
            if (d->phonetisationSeparatorForPhonemes.isEmpty()) {
                int pos = 0;
                while ((pos = d->regexMatchPhoneme.indexIn(phonetisationProposed, pos)) != -1) {
                    phonemesSeparated << d->regexMatchPhoneme.cap(0);
                    pos += d->regexMatchPhoneme.matchedLength();
                }
            }
            else {
                phonemesSeparated = phonetisationProposed.split(d->phonetisationSeparatorForPhonemes, QString::SkipEmptyParts);
            }
            // Process alternative phonetisations
            QList<QStringList> phonetisations;
            phonetisations << QStringList();
            foreach (QString phoneme, phonemesSeparated) {
                if (phoneme.endsWith("*")) {
                    QList<QStringList> additionalPhonetisations;
                    for (int i = 0; i < phonetisations.count(); ++i) {
                        additionalPhonetisations << phonetisations[i];      // without
                        phonetisations[i].append(phoneme.replace("*", "")); // with
                    }
                    phonetisations << additionalPhonetisations;
                }
                else {
                    for (int i = 0; i < phonetisations.count(); ++i) {
                        phonetisations[i].append(phoneme);
                    }
                }
            }
            foreach (QStringList phonetisation, phonetisations) {
                atoken.phonetisations.append(phonetisation.join(" "));
                // qDebug() << atoken.orthographic << phonetisation.join(" ");
            }
        }
        alignerTokens << atoken;
    }
    return alignerTokens;
}

// private
bool HTKForcedAligner::createFilesDCTandLAB(const QString &filenameBase, QList<SpeechToken> &atokens)
{
//    sil	sil
//    bon	b o~ sp
//    euh	eu sp
//    le	l @ sp
//    vrai	v R E sp
//    est	e sp
//    la	l a sp
//    r\351ponse	R e p o~ s sp
//    a	a
    // Create LAB file
    QFile fileLAB(filenameBase + ".lab");
    if ( !fileLAB.open( QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text ) ) return false;
    QTextStream lab(&fileLAB);
    lab.setCodec("ISO 8859-1");
    // Insert all tokens into LAB file and populate a hash table with (unique) pronunciation variants
    QList<QPair<QString, QString> > pronunciations;
    foreach (SpeechToken atoken, atokens) {
        if (atoken.orthographic == "_") continue;
        if (atoken.orthographic.trimmed() == "") continue;
        QString orthoEncoded = encodeEntities(atoken.orthographic);
        lab << orthoEncoded << "\n";
        foreach (QString phonetisation, atoken.phonetisations) {
            QPair<QString, QString> pronunciation(orthoEncoded, phonetisation);
            if (!pronunciations.contains(pronunciation))
                pronunciations << pronunciation;
        }
    }
    fileLAB.close();
    // Create DCT file with pronunciation variants
    QFile fileDCT(filenameBase + ".dct");
    if ( !fileDCT.open( QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text ) ) return false;
    QTextStream dct(&fileDCT);
    dct.setCodec("ISO 8859-1");
    dct << "sil\tsil\n";
    for (int i = 0; i < pronunciations.count(); ++i) {
        QString pron = translatePhonemes(pronunciations[i].second);
        dct << pronunciations[i].first << "\t" << pron << " sp\n";
        // if (i != pronunciations.count() - 1)
    }
    fileDCT.close();
    return true;
}

// private
bool HTKForcedAligner::runAligner(const QString &filenameBase, QList<SpeechToken> &atokens, QList<SpeechPhone> &aphones,
                                  QString &alignerOutput)
{
    aphones.clear();
    QFile::remove(filenameBase + ".rec");
    if (!createFilesDCTandLAB(filenameBase, atokens)) return false;

    // Parameters for HTK Viterbi recogniser
    QStringList     hviteParameters;
    hviteParameters << "-A";                    // print arguments into log file
    hviteParameters << "-a";                    // perform alignment
    hviteParameters << "-b" << "sil";           // sil model inserted at the start and end of network
    hviteParameters << "-m";                    // generate model level output transcriptions = indicate phoneme boundaries
    hviteParameters << "-C" << d->filenameCFG;   // configuration file for acoustic model
    hviteParameters << "-H" << d->filenameHMM;   // HMM acoustic model
    hviteParameters << "-t" << QString::number(d->beamThreshold); // beam search threshld
    hviteParameters << filenameBase + ".dct";   // dictionary file of utterance to align
    hviteParameters << d->filenamePhoneList;     // phoneme list for given acoustic model
    hviteParameters << filenameBase + ".wav";   // wave file of utterance to align
    // Run HTK Viterbi recogniser
    // DIRECTORY:
    QString htkPath = QDir::homePath() + "/Praaline/tools/htk/";
    QString htkExecutable = htkPath + "HVite";
    QProcess hvite;
    hvite.setWorkingDirectory(htkPath);
    hvite.start(htkExecutable , hviteParameters);
    if (!hvite.waitForStarted(-1)) {
        alignerOutput = QString(hvite.readAllStandardOutput() + hvite.readAllStandardError());
        return false;
    }
    if (!hvite.waitForFinished(-1)) {
        alignerOutput = QString(hvite.readAllStandardOutput() + hvite.readAllStandardError());
        return false;
    }
    alignerOutput = QString(hvite.readAllStandardOutput());

    QFile fileREC(filenameBase + ".rec");
    if ( !fileREC.open( QIODevice::ReadOnly | QIODevice::Truncate | QIODevice::Text ) ) return false;
    QTextStream rec(&fileREC);
    rec.setCodec("ISO 8859-1");
    long long start = 0, end = 0;
    QString phone, token;
    double scoreAM = 0.0;
    while (!rec.atEnd()) {
        QString line = rec.readLine();
        if (line.isEmpty()) continue;
        QStringList fields = line.split(" ");
        if (fields.count() < 4) break;
        // Format: 0 700000 sil -428.240662 sil
        start = fields[0].toLongLong() * 100;
        end = fields[1].toLongLong() * 100;
        if (start >= end)
            continue;
        phone = fields[2];
        // Phone reverse translation
        if (phone == "sil") phone = "_";
        if (d->phonemeReverseTranslations.contains(phone)) phone = d->phonemeReverseTranslations.value(phone);
        // Acoustic model score
        scoreAM = fields[3].toDouble();
        // Beginning of a token
        bool isTokenStart(false);
        if (fields.count() > 4) {
            token = fields[4];
            isTokenStart = true;
        }
        aphones << SpeechPhone(phone, RealTime::fromNanoseconds(start), RealTime::fromNanoseconds(end), scoreAM, token, isTokenStart);
    }
    return true;
}

// private
void HTKForcedAligner::cleanUpTempFiles(const QString &waveResampledBase)
{
    QFile::remove(waveResampledBase + ".wav");
    QFile::remove(waveResampledBase + ".lab");
    QFile::remove(waveResampledBase + ".dct");
    QFile::remove(waveResampledBase + ".rec");
}

// public
bool HTKForcedAligner::alignTokens(const QString &waveFilepath, RealTime timeFrom, RealTime timeTo,
                                   Praaline::Core::IntervalTier *tierTokens, int &indexFrom, int &indexTo,
                                   bool insertLeadingAndTrailingPauses,
                                   QList<Praaline::Core::Interval *> &outPhonesList, QString &outAlignerOutput)
{
    QList<SpeechToken> atokens;
    QList<SpeechPhone> aphones;
    QList<RealTime> updatedTokenBoundaries;
    if (!tierTokens) return false;
    if (indexFrom < 0) return false;
    if (indexFrom >= tierTokens->count()) return false;
    if (indexTo < 0) return false;
    if (indexTo >= tierTokens->count()) return false;
    if (indexTo < indexFrom) return false;

    // Create temp path, if it does not exist
    QDir tempDir(d->pathTemp);
    if (!tempDir.exists()) {
        tempDir.mkpath(".");
    }
    // Create resampled wave file in temporary directory
    QString waveResampledBase = d->pathTemp + QFileInfo(waveFilepath).baseName();
    if (QFile::exists(waveResampledBase + ".wav")) QFile::remove(waveResampledBase + ".wav");
    if ((timeFrom > RealTime::zeroTime) && (timeTo > timeFrom)) {
        AudioSegmenter::segment(waveFilepath, waveResampledBase + ".wav", timeFrom, timeTo, d->sampleRateAM, false, 1);
    } else {
        AudioSegmenter::resample(waveFilepath, waveResampledBase + ".wav", d->sampleRateAM, false, 1);
    }  
    // Simulate run on alignment tokens
    // Create alignment tokens for the entire utterance
    atokens = alignerTokensFromIntervalTier(insertLeadingAndTrailingPauses, tierTokens, indexFrom, indexTo);
    bool alignerOK = runAligner(waveResampledBase, atokens, aphones, outAlignerOutput);
    // Check that the aligner came up with a proper solution
    if ((!alignerOK) || aphones.isEmpty()) {
        cleanUpTempFiles(waveResampledBase);
        return false;
    }
    // Apply changes to actual tokens tier
    // Insert silent pauses at the beginning and end, if authorised to do so.
    if (insertLeadingAndTrailingPauses) {
        if (!tierTokens->at(indexFrom)->isPauseSilent()) {
            // Insert pause at the beginning of the utterance:
            // [xxx] becomes [ _ ][xxx]
            tierTokens->split(indexFrom, tierTokens->at(indexFrom)->tCenter(), true);
            tierTokens->at(indexFrom)->setText("_");
            indexTo++;
        }
        if (!tierTokens->at(indexTo)->isPauseSilent()) {
            // Insert pause at the end of the utterance
            // [xxx] becomes [xxx][ _ ]
            tierTokens->split(indexTo, tierTokens->at(indexTo)->tCenter(), false);
            indexTo++;
            tierTokens->at(indexTo)->setText("_");
        }
    }
    // Remove pauses from within the utterance
    for (int i = indexTo - 1; i >= indexFrom + 1; --i) {
        if (tierTokens->at(i)->isPauseSilent()) {
            tierTokens->removeInterval(i);
            indexTo--;
        }
    }
    // Verify micro-pauses inserted before occlusive consonants
    QStringList occlusives;
    occlusives << "p" << "t" << "k";
    for (int i = aphones.count() - 2; i > 1; --i) {
        SpeechPhone aphone = aphones.at(i);
        if ((aphone.phone == "sp") && (occlusives.contains(aphones.at(i + 1).phone))) {
            if (aphone.duration() < RealTime::fromMilliseconds(80)) {
                // Remove pre-occlusive micro-pause and give all its duration to the occlusive
                aphones[i + 1].start = aphone.start;
                aphones.removeAt(i);
            }
        }
        else if ((aphone.phone == "sp") && (aphone.duration() < RealTime::fromMilliseconds(80))) {
            // Remove micro-pause and split its duration into two equal parts
            aphones[i + 1].start = aphones[i + 1].start - aphone.duration() / 2;
            aphones[i - 1].end = aphones[i + 1].start;
            aphones.removeAt(i);
        }
    }
    // Add micro-pauses where necessary
    int indexToken(indexFrom);
    for (int i = 0; i < aphones.count(); ++i) {
        if (aphones[i].phone == "sp") {
            aphones[i].phone = "_";
            if ((indexToken > 0) && !aphones[i].isTokenStart) {
                aphones[i].isTokenStart = true;
                // Normal micro-pause between words. Add a pause token to the tokens tier.
                tierTokens->splitToEqual(indexToken - 1, 2).last()->setText("_");
                indexTo++;
            }
        }
        if (aphones[i].isTokenStart) {
            indexToken++;
        }
    }
    // Create list of phones
    RealTime offset = (timeFrom > RealTime::zeroTime) ? timeFrom : RealTime::zeroTime;
    foreach (SpeechPhone phone, aphones) {
        outPhonesList << new Interval(offset + phone.start, offset + phone.end, phone.phone);
        if (phone.isTokenStart) {
            updatedTokenBoundaries << offset + phone.start;
        }
    }
    updatedTokenBoundaries << ((timeTo > RealTime::zeroTime) ? timeTo : (offset + aphones.last().end));
    // Modify boundaries of tokens
    bool ok = tierTokens->realignIntervals(indexFrom, updatedTokenBoundaries);
    bool shouldCleanUpTempFiles(false);
    // Clean-up
    if (ok && shouldCleanUpTempFiles) {
        cleanUpTempFiles(waveResampledBase);
    }
    // For testing purposes only
    if (!ok || !shouldCleanUpTempFiles) {
        // We must copy the phones because the AnnotationTierGroup will seize ownership of the pointers
        // and will delete them. If that happened, the output of this method would be a list of stale pointers.
        QList<Interval *> phones;
        foreach (Interval *p, outPhonesList) phones << new Interval(p);
        IntervalTier *tierPhones = new IntervalTier("phone", phones);
        AnnotationTierGroup *txg = new AnnotationTierGroup();
        txg->addTier(tierPhones);
        txg->addTier(new IntervalTier(tierTokens));
        PraatTextGrid::save(waveResampledBase + ".TextGrid", txg);
        delete txg;
    }
    return ok;
}

// public
bool HTKForcedAligner::alignAllTokens(const QString &waveFilepath, Praaline::Core::IntervalTier *tierTokens,
                                      QList<Praaline::Core::Interval *> &outPhonesList, QString &outAlignerOutput)
{
    if (!tierTokens) return false;
    if (tierTokens->count() == 0) return false;
    int indexFrom = 0;
    int indexTo = tierTokens->count() - 1;
    return alignTokens(waveFilepath, RealTime(-1, 0), RealTime(-1, 0),
                       tierTokens, indexFrom, indexTo, true,
                       outPhonesList, outAlignerOutput);
}

// public
bool HTKForcedAligner::alignUtterance(const QString &waveFilepath,
                                      Praaline::Core::IntervalTier *tierUtterances, int &indexUtteranceToAlign,
                                      Praaline::Core::IntervalTier *tierTokens, Praaline::Core::IntervalTier *tierPhones,
                                      QString &outAlignerOutput, bool insertLeadingAndTrailingPauses)
{
    if (!tierUtterances) return false;
    if (!tierTokens) return false;
    if (!tierPhones) return false;
    if (indexUtteranceToAlign < 0) return false;
    if (indexUtteranceToAlign >= tierUtterances->count()) return false;
    Interval *utterance = tierUtterances->at(indexUtteranceToAlign);

    RealTime timeFrom = utterance->tMin();
    RealTime timeTo = utterance->tMax();
    QPair<int, int> tokenIndices = tierTokens->getIntervalIndexesContainedIn(tierUtterances->at(indexUtteranceToAlign));
    int tokenIndexFrom = tokenIndices.first;
    int tokenIndexTo = tokenIndices.second;
    QList<Interval *> phonesList;

    bool ok = alignTokens(waveFilepath, timeFrom, timeTo, tierTokens, tokenIndexFrom, tokenIndexTo,
                          insertLeadingAndTrailingPauses, phonesList, outAlignerOutput);
    if (ok) {
        int i(-1);
        i = tokenIndexFrom;
        while ((i < tierTokens->count()) && (tierTokens->at(i)->isPauseSilent())) i++;
        if ((i < tierTokens->count()) && (tierTokens->at(i)->tMin() != utterance->tMin())) {
            tierUtterances->split(indexUtteranceToAlign, tierTokens->at(i)->tMin(), true);
            tierUtterances->at(indexUtteranceToAlign)->setText("_");
            indexUtteranceToAlign++;
        }
        i = tokenIndexTo;
        while ((i - 1 >= 0) && (tierTokens->at(i - 1)->isPauseSilent())) i--;
        if ((i >= 0) && (tierTokens->at(i)->isPauseSilent())) {
            tierUtterances->split(indexUtteranceToAlign, tierTokens->at(i)->tMin(), false);
            tierUtterances->at(indexUtteranceToAlign + 1)->setText("_");
        }
        tierPhones->patchIntervals(phonesList, timeFrom, timeTo);
    }
    return ok;
}

bool HTKForcedAligner::alignAllUtterances(const QString &waveFilepath,
                                          Praaline::Core::IntervalTier *tierUtterances,
                                          Praaline::Core::IntervalTier *tierTokens, Praaline::Core::IntervalTier *tierPhones,
                                          bool insertLeadingAndTrailingPauses)
{
    if (!tierUtterances) return false;
    if (!tierTokens) return false;
    if (!tierPhones) return false;
    int indexUtterance = tierUtterances->count() - 1;
    while (indexUtterance >= 0) {
        if (tierUtterances->at(indexUtterance)->isPauseSilent()) { indexUtterance--; continue; }
        QString alignerOutput;
        bool result(false);
        result = alignUtterance(waveFilepath, tierUtterances, indexUtterance, tierTokens, tierPhones,
                                alignerOutput, insertLeadingAndTrailingPauses);
        if (!result) qDebug() << result << tierUtterances->at(indexUtterance)->text() << alignerOutput;
        alignerMessage(alignerOutput);
        indexUtterance--;
    }
    tierTokens->mergeIdenticalAnnotations("_");
    tierUtterances->mergeIdenticalAnnotations("_");
    tierPhones->fillEmptyWith("", "_");
    tierPhones->mergeIdenticalAnnotations("_");
    return true;
}

} // namespace ASR
} // namespace Praaline
