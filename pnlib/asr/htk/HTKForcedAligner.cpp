#include <QDebug>
#include <QString>
#include <QProcess>
#include <QTemporaryDir>
#include <QTextStream>
#include <QCoreApplication>

#include "pnlib/AudioSegmenter.h"
#include "SpeechToken.h"
#include "HTKForcedAligner.h"
#include "phonetiser/ExternalPhonetiser.h"

struct HTKForcedAlignerData {
    QString filenameCFG;
    QString filenameHMM;
    QString filenamePhoneList;
    uint sampleRateAM;
    uint beamThreshold;
    QHash<QString, QString> phonemeTranslations;
};


HTKForcedAligner::HTKForcedAligner(QObject *parent)
    : QObject(parent), d(new HTKForcedAlignerData)
{
    // DIRECTORY:
    // QString appPath = QCoreApplication::applicationDirPath();
    QString path = "D:/Aligner_tests/";
    d->filenameCFG = path + "fra.cfg";
    d->filenameHMM = path + "fra.hmm";
    d->filenamePhoneList = path + "fra_phone.list";
    d->sampleRateAM = 16000;
    d->beamThreshold = 500;

    d->phonemeTranslations.insert("9~", "oe~");
    d->phonemeTranslations.insert("2", "eu");
    d->phonemeTranslations.insert("9", "oe");
    d->phonemeTranslations.insert("A", "a");
    d->phonemeTranslations.insert("*", "");
}

HTKForcedAligner::~HTKForcedAligner()
{
    delete d;
}

QString encodeEntities(const QString &src, const QString &force=QString())
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

QString decodeEntities(const QString &src)
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

QString HTKForcedAligner::translatePhonemes(QString input)
{
    QString translated(input);
    translated = translated.append(" ");
    foreach (QString original, d->phonemeTranslations.keys()) {
        translated = translated.replace(original + " ", d->phonemeTranslations.value(original) + " ");
    }
    return translated.trimmed();
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
    if ( !fileLAB.open( QIODevice::ReadWrite | QIODevice::Text ) ) return false;
    QTextStream lab(&fileLAB);
    lab.setCodec("ISO 8859-1");
    // Insert all tokens into LAB file and populate a hash table with (unique) pronunciation variants
    QList<QPair<QString, QString> > pronunciations;
    foreach (SpeechToken atoken, atokens) {
        if (atoken.orthographic == "_") continue;
        QString orthoEncoded = encodeEntities(atoken.orthographic);
        lab << orthoEncoded << "\n";
        QPair<QString, QString> pronunciation(orthoEncoded, atoken.phonetisation);
        if (!pronunciations.contains(pronunciation))
            pronunciations << pronunciation;
    }
    fileLAB.close();
    // Create DCT file with pronunciation variants
    QFile fileDCT(filenameBase + ".dct");
    if ( !fileDCT.open( QIODevice::ReadWrite | QIODevice::Text ) ) return false;
    QTextStream dct(&fileDCT);
    dct.setCodec("ISO 8859-1");
    dct << "sil\tsil\n";
    for (int i = 0; i < pronunciations.count(); ++i) {
        dct << pronunciations[i].first << "\t" << translatePhonemes(pronunciations[i].second);
        if (i != pronunciations.count() - 1)
            dct << " sp\n";
        else
            dct << "\n";
    }
    fileDCT.close();
    return true;
}

bool HTKForcedAligner::runAligner(const QString &filenameBase, QList<SpeechToken> &atokens, QList<SpeechPhone> &aphones)
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
    QString htkPath = appPath + "/plugins/aligner/";
    QProcess hvite;
    hvite.setWorkingDirectory(htkPath);
    hvite.start(htkPath + "hvite.exe" , hviteParameters);
    if (!hvite.waitForStarted(-1))  return false;
    if (!hvite.waitForFinished(-1)) return false;

    QFile fileREC(filenameBase + ".rec");
    if ( !fileREC.open( QIODevice::ReadOnly | QIODevice::Text ) ) return false;
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
        if (start == end)
            continue;
        phone = fields[2];
        if (phone == "sil") phone = "_";
        scoreAM = fields[3].toDouble();
        if (fields.count() > 4) token = fields[4];
        aphones << SpeechPhone(phone, RealTime::fromNanoseconds(start), RealTime::fromNanoseconds(end), scoreAM, token);
    }
    return true;
}

void HTKForcedAligner::alignUtterances(QString waveFile, IntervalTier *tier_utterances, IntervalTier *tier_tokens, IntervalTier *tier_phones)
{
    QString path = "D:/Aligner_tests/";

    QList<SpeechToken> atokens;
    QList<SpeechPhone> aphones;

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
        runAligner(QString("%1/%2").arg(path).arg(utt->attribute("utteranceID").toString()), atokens, aphones);
        if (aphones.isEmpty()) continue;
        RealTime offset = utt->tMin();
        foreach (SpeechPhone phone, aphones) {
            list_phones << new Interval(offset + phone.start, offset + phone.end, phone.phone);
        }
    }
    tier_phones->replaceAllIntervals(list_phones);
}

