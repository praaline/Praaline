#include <stdio.h>
#include "pocketsphinx.h"
#include "sphinxbase/byteorder.h"

#include "pncore/corpus/CorpusRecording.h"
#include "pncore/annotation/IntervalTier.h"

#include "SphinxConfiguration.h"
#include "SphinxOfflineRecogniser.h"

namespace Praaline {
namespace ASR {

struct SphinxOfflineRecogniserData {
    SphinxOfflineRecogniserData() : pocketSphinx(0), sphinxConfig(0), mfccFileHandle(0),
        mfccShouldSwap(false), mfccVectorLength(0), mfccNumberFeatures(0)
    {}

    SphinxConfiguration config;
    QString filenameFeatures;

    ps_decoder_t *pocketSphinx;
    cmd_ln_t *sphinxConfig;

    FILE *mfccFileHandle;
    bool mfccShouldSwap;
    int mfccVectorLength;
    int32 mfccNumberFeatures;

    QString filenameMLLR;
};

SphinxOfflineRecogniser::SphinxOfflineRecogniser(QObject *parent) :
    QObject(parent), d(new SphinxOfflineRecogniserData())
{
}

SphinxOfflineRecogniser::~SphinxOfflineRecogniser()
{
    if (d->pocketSphinx) {
        ps_free(d->pocketSphinx);
    }
    delete d;
}

SphinxConfiguration SphinxOfflineRecogniser::config() const
{
    return d->config;
}

bool SphinxOfflineRecogniser::initialize(const SphinxConfiguration &config)
{
    cmd_ln_t *ps_config;
    ps_decoder_t *ps;

    ps_config = cmd_ln_init(NULL, ps_args(), TRUE,
                            "-hmm", config.directoryAcousticModel().toLatin1().constData(),
                            "-lm", config.filenameLanguageModel().toLatin1().constData(),
                            "-dict", config.filenamePronunciationDictionary().toLatin1().constData(),
                            NULL);
    if (ps_config == NULL) return false;
    ps = ps_init(ps_config);
    if (ps == NULL) return false;
    // All OK, replace in class
    if (d->pocketSphinx) {
        ps_free(d->pocketSphinx);
    }
    d->pocketSphinx = ps;
    d->mfccVectorLength = cmd_ln_int32_r(ps_config, "-ceplen");
    d->config = config;
    return true;
}

// protected
bool SphinxOfflineRecogniser::openFeatureFile(const QString &filename)
{
    FILE *infh;
    int32 nmfc;
    long flen;
    bool mfccShouldSwap = false;
    // Check that decoder is initialised
    if (!d->pocketSphinx) return false;
    // Open file
    if ((infh = fopen(filename.toLocal8Bit().constData(), "rb")) == NULL) {
        error(QString("Failed to open %1").arg(filename));
        return false;
    }
    // Read and check it is an MFCC file
    fseek(infh, 0, SEEK_END);
    flen = ftell(infh);
    fseek(infh, 0, SEEK_SET);
    if (fread(&nmfc, 4, 1, infh) != 1) {
        error("Failed to read 4 bytes from MFCC file");
        fclose(infh);
        return false;
    }
    if (nmfc != flen / 4 - 1) {
        SWAP_INT32(&nmfc);
        mfccShouldSwap = true;
        if (nmfc != flen / 4 - 1) {
            error(QString("File length mismatch: %1 != %2, maybe it's not MFCC file").arg(nmfc).arg(flen / 4 - 1));
            return false;
        }
    }
    if (nmfc == 0) {
        error("Empty mfcc file");
        return false;
    }
    // All is OK, update internal state
    // If a file was previously open, close it
    if (d->mfccFileHandle) {
        fclose(d->mfccFileHandle);
    }
    // Use new feature file from now on
    d->mfccFileHandle = infh;
    d->mfccShouldSwap = mfccShouldSwap;
    d->mfccNumberFeatures = nmfc;
    ps_start_stream(d->pocketSphinx);
    return true;
}

bool SphinxOfflineRecogniser::closeFeatureFile()
{
    // If a file was previously open, close it
    if (d->mfccFileHandle) {
        fclose(d->mfccFileHandle);
    }
    return true;
}

bool SphinxOfflineRecogniser::decode(int startFrame, int endFrame)
{
    mfcc_t **mfcs;
    float32 *floats;
    int32 numberOfFrames;
    size_t length;
    int32 nmfc;

    // Checks
    if (!d->pocketSphinx) return false;
    if (!d->mfccFileHandle) return false;
    if (endFrame != -1 && endFrame < startFrame) {
        error(QString("End frame %1 is < start frame %2").arg(endFrame).arg(startFrame));
        return false;
    }

    // Return to beginning of MFC file and read first 4 bytes
    fseek(d->mfccFileHandle, 0, SEEK_SET);
    if (fread(&nmfc, 4, 1, d->mfccFileHandle) != 1) {
        error("Failed to read 4 bytes from MFCC file");
        fclose(d->mfccFileHandle);
        return false;
    }
    // All is OK

    // Read MFCC data corresponding to stard-end frame
    fseek(d->mfccFileHandle, startFrame * 4 * d->mfccVectorLength, SEEK_CUR);
    if (endFrame == -1)
        endFrame = d->mfccNumberFeatures / d->mfccVectorLength;
    numberOfFrames = endFrame - startFrame;
    mfcs = (mfcc_t **)(ckd_calloc_2d(numberOfFrames, d->mfccVectorLength, sizeof(**mfcs)));
    floats = (float32 *)mfcs[0];
    length = numberOfFrames * d->mfccVectorLength;
    size_t ret = fread(floats, sizeof(mfcc_t), length, d->mfccFileHandle);
    if (ret != length) {
        perror("Error");
        error("Failed to read items from mfcfile");
        ckd_free_2d(mfcs);
        return false;
    }
    if (d->mfccShouldSwap) {
        for (size_t i = 0; i < length; ++i)
            SWAP_FLOAT32(&floats[i]);
    }
#ifdef FIXED_POINT
    for (size_t i = 0; i < length; ++i)
        mfcs[0][i] = FLOAT2MFCC(floats[i]);
#endif

    // Process MFCC data, do recognition
    ps_start_utt(d->pocketSphinx);
    ps_process_cep(d->pocketSphinx, mfcs, numberOfFrames, FALSE, TRUE);
    ps_end_utt(d->pocketSphinx);
    ckd_free_2d(mfcs);

    return true;
}

bool SphinxOfflineRecogniser::setMLLR(const QString &filenameMLLR)
{
    ps_mllr_t *mllr;

    if (!d->pocketSphinx) return false;
    if (filenameMLLR.isEmpty()) return false;
    if (filenameMLLR == d->filenameMLLR) return true;

    if ((mllr = ps_mllr_read(filenameMLLR.toLocal8Bit().constData())) == NULL) {
        return false;
    }
    if (ps_update_mllr(d->pocketSphinx, mllr) == NULL) {
        ps_mllr_free(mllr);
        return false;
    }

    return true;
}

bool SphinxOfflineRecogniser::setLanguageModel(const QString &filenameLM)
{
    if (!d->pocketSphinx) return false;
    if (filenameLM.isEmpty()) return false;
    if (ps_set_search(d->pocketSphinx, filenameLM.toLocal8Bit().constData())) {
        error(QString("No such language model: %1").arg(filenameLM));
        return false;
    }
    return true;
}

QString SphinxOfflineRecogniser::getUtteranceText() const
{
    QString utterance;
    if (!d->pocketSphinx) return QString();
    int32 ascr, lscr, sf, ef, bestscore;
#ifdef Q_OS_WIN
    ps_seg_t *itor = ps_seg_iter(d->pocketSphinx, &bestscore);
#else
    ps_seg_t *itor = ps_seg_iter(d->pocketSphinx);
#endif

    lscr = 0; ascr = 0;
    while (itor) {
        // Accumulate language model scores
        int32 wlascr, wlscr;
        ps_seg_prob(itor, &wlascr, &wlscr, NULL);
        lscr += wlscr;
        ascr += wlascr;
        // Get word, start and end frame of word
        char const *w = ps_seg_word(itor);
        ps_seg_frames(itor, &sf, &ef);
        utterance.append(" ").append(w);
        // Move to next word
        itor = ps_seg_next(itor);
    }
    return utterance.trimmed();
}

QList<Interval *> SphinxOfflineRecogniser::getSegmentation() const
{
    QList<Interval *> words;
    if (!d->pocketSphinx) return words;
    int32 ascr, lscr, sf, ef, bestscore;
#ifdef Q_OS_WIN
    ps_seg_t *itor = ps_seg_iter(d->pocketSphinx, &bestscore);
#else
    ps_seg_t *itor = ps_seg_iter(d->pocketSphinx);
#endif

    lscr = 0; ascr = 0;
    while (itor) {
        // Accumulate language model scores
        int32 wlascr, wlscr;
        ps_seg_prob(itor, &wlascr, &wlscr, NULL);
        lscr += wlscr;
        ascr += wlascr;
        // Get word, start and end frame of word
        char const *w = ps_seg_word(itor);
        ps_seg_frames(itor, &sf, &ef);
        // Create interval for this word
        Interval *word = new Interval(RealTime::fromMilliseconds(sf * 10),
                                      RealTime::fromMilliseconds(ef * 10),
                                      QString(w));
        word->setAttribute("wlscr", wlscr);
        word->setAttribute("wlascr", wlascr);
        words << word;
        // Move to next word
        itor = ps_seg_next(itor);
    }
    return words;
}

} // namespace ASR
} // namespace Praaline
