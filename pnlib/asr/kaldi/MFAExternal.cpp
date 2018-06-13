#include <QString>
#include <QList>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QPair>

#include "pncore/annotation/IntervalTier.h"
using namespace Praaline::Core;

#include "pnlib/media/AudioSegmenter.h"
using namespace Praaline::Media;

#include "SpeechToken.h"
#include "phonetiser/PhonemeTranslation.h"
#include "MFAExternal.h"

namespace Praaline {
namespace ASR {

struct MFAExternalData {
    QString pathOutput;
    uint sampleRateAM;
    QHash<QString, QString> phonemeTranslations;
    QHash<QString, QString> phonemeReverseTranslations;
    QStringList dictionary;
    QString speakerID;
    bool outputWaveFiles;
};

MFAExternal::MFAExternal(QObject *parent) :
    ForcedAligner(parent), d(new MFAExternalData)
{

    d->sampleRateAM = 16000;
    d->pathOutput = QDir::homePath() + "/Praaline/mfa_temp/";
    d->outputWaveFiles = true;

    // The order is important. Start with the longest phonemes.
    QStringList phonemeset;
    phonemeset << "9~" << "a~" << "e~" << "o~"
               << "2" << "9" << "A" << "@" << "E" << "H" << "O" << "R" << "S" << "Z"
               << "a" << "b" << "d" << "e" << "f" << "g" << "i" << "j" << "k" << "l"
               << "m" << "n" << "o" << "p" << "s" << "t" << "u" << "v" << "w" << "y" << "z";
    setPhonemeset(phonemeset);

    d->phonemeTranslations.insert("9~", "oe~");
    d->phonemeTranslations.insert("2", "eu");
    d->phonemeTranslations.insert("9", "oe");

    d->phonemeReverseTranslations.insert("oe~", "9~");
    d->phonemeReverseTranslations.insert("eu", "2");
    d->phonemeReverseTranslations.insert("oe", "9");
}

MFAExternal::~MFAExternal()
{
    delete d;
}

QString MFAExternal::translatePhonemes(const QString &input)
{
    QString translated(input);
    translated = translated.append(" ");
    foreach (QString original, d->phonemeTranslations.keys()) {
        translated = translated.replace(original + " ", d->phonemeTranslations.value(original) + " ");
    }
    return translated.trimmed();
}

void MFAExternal::setSpeakerID(const QString &speakerID)
{
    d->speakerID = speakerID;
}

void MFAExternal::setOutputPath(const QString &path)
{
    d->pathOutput = path;
}

QString makesafe(const QString &in) {
    QString r= QString(in).replace("/", "").replace("_", "").replace(":", "").replace("?", "").replace("^", "")
                          .replace("~", "").replace("[", "").replace("]", "").replace("@", "").replace("(", "").replace(")", "")
                          .replace(".", "").replace(",", "");
    while (r.contains(" ")) r = r.replace(" ", "_");
    return r;
}


// Encode UTF entities (used for accented characters in dictionary and transcription files)
// private
QString MFAExternal::encodeEntities(const QString &src, const QString &force)
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
QString MFAExternal::decodeEntities(const QString &src)
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

bool MFAExternal::alignTokens(const QString &waveFilepath, RealTime timeFrom, RealTime timeTo,
                              IntervalTier *tierTokens, int &indexFrom, int &indexTo,
                              bool insertLeadingAndTrailingPauses,
                              QList<Interval *> &outPhonesList, QString &outAlignerOutput)
{
    QList<SpeechToken> atokens;
    QList<SpeechPhone> aphones;

    if (!tierTokens) return false;
    if (indexFrom < 0) return false;
    if (indexFrom >= tierTokens->count()) return false;
    if (indexTo < 0) return false;
    if (indexTo >= tierTokens->count()) return false;
    if (indexTo < indexFrom) return false;

    // Create temp path, if it does not exist
    QDir tempDir(d->pathOutput + "/" + d->speakerID);
    if (!tempDir.exists()) {
        tempDir.mkpath(".");
    }
    // Create resampled wave file in temporary directory
    QString waveResampledBase = d->pathOutput + "/" + d->speakerID + "/" + QFileInfo(waveFilepath).baseName() +
            "_" + d->speakerID + "_" + QString::number(indexFrom);
    if (d->outputWaveFiles) {
        if (QFile::exists(waveResampledBase + ".wav")) QFile::remove(waveResampledBase + ".wav");
        if ((timeFrom > RealTime::zeroTime) && (timeTo > timeFrom)) {
            AudioSegmenter::segment(waveFilepath, waveResampledBase + ".wav", timeFrom, timeTo, d->sampleRateAM, false, 1);
        } else {
            AudioSegmenter::resample(waveFilepath, waveResampledBase + ".wav", d->sampleRateAM, false, 1);
        }
    }
    // Simulate run on alignment tokens
    // Create alignment tokens for the entire utterance
    atokens = alignerTokensFromIntervalTier(insertLeadingAndTrailingPauses, tierTokens, indexFrom, indexTo);

    // Create LAB file
    QFile fileLAB(waveResampledBase + ".lab");
    if ( !fileLAB.open( QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text ) ) return false;
    QTextStream lab(&fileLAB);
    lab.setCodec("UTF-8");
    // Insert all tokens into LAB file and populate a hash table with (unique) pronunciation variants
    foreach (SpeechToken atoken, atokens) {
        if (atoken.orthographic == "_") continue;
        if (atoken.orthographic.trimmed() == "") continue;
        if (atoken.phonetisations.isEmpty()) continue;
        QString orthoEncoded = makesafe(atoken.orthographic.trimmed().toLower());
        lab << orthoEncoded << " ";
        foreach (QString phonetisation, atoken.phonetisations) {
            QString phon = phonetisation;
            phon = translatePhonemes(phon);
            QString dicEntry = QString(orthoEncoded).append("\t").append(phon);
            if (!d->dictionary.contains(dicEntry))
                d->dictionary << dicEntry;
        }
    }
    fileLAB.close();
    return false;
}

bool MFAExternal::startDictionary()
{
    d->dictionary.clear();
    return true;
}

bool MFAExternal::finishDictionary(const QString &filename)
{
    QString dicFilename = filename;
    if (dicFilename.isEmpty()) dicFilename = "dictionary";
    // Create DIC file
    QFile fileDIC(d->pathOutput + "/" + dicFilename + ".dic");
    if ( !fileDIC.open( QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text ) ) return false;
    QTextStream dic(&fileDIC);
    dic.setCodec("UTF-8");
    std::sort(d->dictionary.begin(), d->dictionary.end());
    foreach (QString entry, d->dictionary) {
        dic << entry << "\n";
    }
    dic << "\n";
    fileDIC.close();
    return true;
}

void MFAExternal::setOutputWaveFiles(bool out)
{
    d->outputWaveFiles = out;
}

} // namespace ASR
} // namespace Praaline

