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
    QString pathTemp;
    uint sampleRateAM;
    PhonemeTranslation phonemeTranslation;
};

MFAExternal::MFAExternal(QObject *parent) :
    ForcedAligner(parent), d(new MFAExternalData)
{

    d->sampleRateAM = 16000;
    d->pathTemp = QDir::homePath() + "/Praaline/aligner_temp/";
}

MFAExternal::~MFAExternal()
{
    delete d;
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

    // Create LAB file
    QFile fileLAB(waveResampledBase + ".lab");
    if ( !fileLAB.open( QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text ) ) return false;
    QTextStream lab(&fileLAB);
    lab.setCodec("UTF-8");
    // Insert all tokens into LAB file and populate a hash table with (unique) pronunciation variants
    QList<QPair<QString, QString> > pronunciations;
    foreach (SpeechToken atoken, atokens) {
        if (atoken.orthographic == "_") continue;
        if (atoken.orthographic.trimmed() == "") continue;
        QString orthoEncoded = atoken.orthographic.trimmed();
        lab << orthoEncoded << " ";
        foreach (QString phonetisation, atoken.phonetisations) {
            QPair<QString, QString> pronunciation(orthoEncoded, phonetisation);
            if (!pronunciations.contains(pronunciation))
                pronunciations << pronunciation;
        }
    }
    fileLAB.close();

    // Create DIC file
    QFile fileDCT(waveResampledBase + ".dct");
    if ( !fileDCT.open( QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text ) ) return false;
    QTextStream dct(&fileDCT);
    dct.setCodec("UTF-8");
    for (int i = 0; i < pronunciations.count(); ++i) {
        QString pron = d->phonemeTranslation.translate( (pronunciations[i].second) );
        dct << pronunciations[i].first << "\t" << pron << " sp\n";
        // if (i != pronunciations.count() - 1)
    }
    fileDCT.close();


    return false;
}


} // namespace ASR
} // namespace Praaline

