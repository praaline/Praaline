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

#include "pnlib/mediautil/AudioSegmenter.h"
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
};


HTKForcedAligner::HTKForcedAligner(QObject *parent)
    : QObject(parent), d(new HTKForcedAlignerData)
{
    // DIRECTORY:
    QString appPath = QCoreApplication::applicationDirPath();
    QString modelsPath = appPath + "/tools/htk/";
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

    d->pathTemp = "/home/george/aligner_test/";
}

HTKForcedAligner::~HTKForcedAligner()
{
    delete d;
}

// Encode UTF entities (used for accented characters in dictionary and transcription files)
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

QString HTKForcedAligner::translatePhonemes(const QString &input)
{
    QString translated(input);
    translated = translated.append(" ");
    foreach (QString original, d->phonemeTranslations.keys()) {
        translated = translated.replace(original + " ", d->phonemeTranslations.value(original) + " ");
    }
    return translated.trimmed();
}

QList<SpeechToken> HTKForcedAligner::alignerTokensFromIntervalTier(IntervalTier *tier_tokens, int indexFrom, int indexTo)
{
    QList<SpeechToken> alignerTokens;
    if (!tier_tokens) return alignerTokens;
    if (tier_tokens->count() == 0) return alignerTokens;
    if (indexTo < 0) indexTo = tier_tokens->count() - 1;
    if (indexTo >= tier_tokens->count()) indexTo = tier_tokens->count() - 1;
    if (indexFrom < 0) indexFrom = 0;
    if (indexTo >= tier_tokens->count()) indexTo = tier_tokens->count() - 1;

    for (int index = indexFrom; index <= indexTo; ++index) {
        Interval *token = tier_tokens->at(index);
        SpeechToken atoken(index, index, token->text());
        QString phonetisationGiven = token->attribute("phonetisation").toString();
        if (!d->useAlternativePronunciations) {
            phonetisationGiven = phonetisationGiven.replace("*", "");
        }
        phonetisationGiven = phonetisationGiven.trimmed();
        // Separate phonemes
        QStringList phonetisationSeparated;
        int pos = 0;
        while ((pos = d->regexMatchPhoneme.indexIn(phonetisationGiven, pos)) != -1) {
            phonetisationSeparated << d->regexMatchPhoneme.cap(0);
            pos += d->regexMatchPhoneme.matchedLength();
        }
        // Process alternative phonetisations
        QList<QStringList> phonetisations;
        phonetisations << QStringList();
        foreach (QString phoneme, phonetisationSeparated) {
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
        alignerTokens << atoken;
    }
    return alignerTokens;
}

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

bool HTKForcedAligner::runAligner(const QString &filenameBase, QList<SpeechToken> &atokens, QList<SpeechPhone> &aphones,
                                  QString &alignerOutput)
{
    if (!createFilesDCTandLAB(filenameBase, atokens)) return false;
    aphones.clear();

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
    QString appPath = QCoreApplication::applicationDirPath();
    QString htkPath = appPath + "/tools/htk/";
#ifdef Q_OS_WIN
    QString htkExecutable = htkPath + "hvite.exe";
#else
#ifdef Q_OS_MAC
    QString htkExecutable = htkPath + "HVite";
#else
    QString htkExecutable = htkPath + "HVite";
#endif
#endif
    QProcess hvite;
    hvite.setWorkingDirectory(htkPath);
    hvite.start(htkExecutable , hviteParameters);
    if (!hvite.waitForStarted(-1))  return false;
    if (!hvite.waitForFinished(-1)) return false;
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


bool HTKForcedAligner::alignUtterance(const QString &waveFile, IntervalTier *tier_tokens, QList<Interval *> &list_phones,
                                      QString &alignerOutput)
{
    QList<SpeechToken> atokens;
    QList<SpeechPhone> aphones;
    QList<RealTime> updatedTokenBoundaries;
    int indexFrom = 0;
    int indexTo = tier_tokens->count() - 1;

    // Create resampled wave file in temporary directory
    QString waveResampledBase = d->pathTemp + QFileInfo(waveFile).baseName();
    if (QFile::exists(waveResampledBase + ".wav")) QFile::remove(waveResampledBase + ".wav");
    AudioSegmenter::resample(waveFile, waveResampledBase + ".wav", d->sampleRateAM);
    // Insert silent pauses at the beginning and end
    if (!tier_tokens->at(indexFrom)->isPauseSilent()) {
        tier_tokens->splitToEqual(indexFrom, 2);
        tier_tokens->at(indexFrom)->setText("_");
        indexTo++;
    }
    if (!tier_tokens->at(indexTo)->isPauseSilent()) {
        tier_tokens->splitToEqual(indexTo, 2);
        indexTo++;
        tier_tokens->at(indexTo)->setText("_");
    }
    // Remove pauses from within the utterance
    for (int i = indexTo - 1; i >= indexFrom + 1; --i) {
        if (tier_tokens->at(i)->isPauseSilent()) {
            tier_tokens->removeInterval(i);
            indexTo--;
        }
    }
    // Create alignment tokens for the entire utterance
    atokens = alignerTokensFromIntervalTier(tier_tokens, indexFrom, indexTo);
    runAligner(waveResampledBase, atokens, aphones, alignerOutput);
    // Check that the aligner came up with a proper solution
    if (aphones.isEmpty()) return false;
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
    int indexToken(0);
    for (int i = 0; i < aphones.count(); ++i) {
        if (aphones[i].phone == "sp") {
            aphones[i].phone = "_";
            if ((indexToken > 0) && !aphones[i].isTokenStart) {
                aphones[i].isTokenStart = true;
                // normal micro-pause between words
                tier_tokens->splitToEqual(indexToken - 1, 2).last()->setText("_");
            }
        }
        if (aphones[i].isTokenStart) {
            indexToken++;
        }
    }
    // Create list of phones
    RealTime offset = tier_tokens->tMin();
    foreach (SpeechPhone phone, aphones) {
        list_phones << new Interval(offset + phone.start, offset + phone.end, phone.phone);
        if (phone.isTokenStart) {
            updatedTokenBoundaries << offset + phone.start;
        }
    }
    updatedTokenBoundaries << offset + aphones.last().end;
    // Modify boundaries of tokens
    bool ok = tier_tokens->realignIntervals(0, updatedTokenBoundaries);
    bool cleanUpTempFiles(false);
    // Clean-up
    if (ok && cleanUpTempFiles) {
        QFile::remove(waveResampledBase + ".wav");
        QFile::remove(waveResampledBase + ".lab");
        QFile::remove(waveResampledBase + ".dct");
        QFile::remove(waveResampledBase + ".rec");
    }
    // For testing
    if (!ok || !cleanUpTempFiles) {
        IntervalTier *tier_phones = new IntervalTier("phone", list_phones);
        AnnotationTierGroup *txg = new AnnotationTierGroup();
        txg->addTier(tier_phones);
        txg->addTier(new IntervalTier(tier_tokens));
        PraatTextGrid::save(waveResampledBase + ".TextGrid", txg);
    }
    return ok;
}


void HTKForcedAligner::alignUtterances(const QString &waveFile, IntervalTier *tier_utterances, IntervalTier *tier_tokens, IntervalTier *tier_phones)
{
    QList<SpeechToken> atokens;
    QList<SpeechPhone> aphones;
    QString alignerOutput;

    QList<Interval *> list_utterances;
    int i = 0;
    foreach (Interval *intv, tier_utterances->intervals()) {
        if (intv->isPauseSilent()) { i++; continue; }
        Interval *utt = new Interval(intv);
        utt->setAttribute("utteranceID", QString::number(i));
        list_utterances << utt;
        i++;
    }
    // AudioSegmenter::segment(waveFile, path, list_utterances, "utteranceID", m_sampleRateAM);
    QList<Interval *> list_phones;
    foreach (Interval *utt, list_utterances) {
        QList<Interval *> utt_tokens = tier_tokens->getIntervalsContainedIn(utt);
        if (utt_tokens.isEmpty()) continue;
        atokens = ExternalPhonetiser::phonetiseList(utt_tokens);
        runAligner(QString("%1/%2").arg(d->pathTemp).arg(utt->attribute("utteranceID").toString()), atokens, aphones, alignerOutput);
        if (aphones.isEmpty()) continue;
        RealTime offset = utt->tMin();
        foreach (SpeechPhone phone, aphones) {
            list_phones << new Interval(offset + phone.start, offset + phone.end, phone.phone);
        }
    }
    tier_phones->replaceAllIntervals(list_phones);
}

} // namespace ASR
} // namespace Praaline
