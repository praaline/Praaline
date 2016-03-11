#include <QString>
#include <QFile>
#include <QTextStream>

#include "base/RealTime.h"
#include "pncore/corpus/corpusrecording.h"
#include "speechrecogniser.h"
#include "pocketsphinx.h"

SpeechRecogniser::SpeechRecogniser()
{
}

SpeechRecogniser::~SpeechRecogniser()
{
}

void SpeechRecogniser::setFilenameAcousticModel(const QString &filename)
{
    m_filenameAcousticModel = filename;
}

void SpeechRecogniser::setFilenameLanguageModel(const QString &filename)
{
    m_filenameLanguageModel = filename;
}

void SpeechRecogniser::setFilenamePronuncationDictionary(const QString &filename)
{
    m_filenamePronunciationDictionary = filename;
}



int SpeechRecogniser::recogniseContinuous(CorpusRecording *recording)
{
    ps_decoder_t *ps;
    cmd_ln_t *config;
    FILE *fh;
    char const *hyp;
    int16 buf[512];
    int rv;
    int32 score;

    config = cmd_ln_init(NULL, ps_args(), TRUE,
                         "-hmm", m_filenameAcousticModel.toLatin1().constData(),
                         "-lm", m_filenameLanguageModel.toLatin1().constData(),
                         "-dict", m_filenamePronunciationDictionary.toLatin1().constData(),
                         NULL);

    if (config == NULL)
        return 1;
    ps = ps_init(config);
    if (ps == NULL)
        return 1;

    // fh = fopen("C:/Qt/mingw-4.8.2-x32/msys/home/george/pocketsphinx-0.8/test/data/goforward.raw", "rb");

    QString path = "D:/SPHINX/inte/";
    QFile fileTranscription(path + "transcription.txt");
    fileTranscription.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&fileTranscription);
    out.setCodec("UTF-8");

    for (int i = 1; i <= 100; ++i) {
        QString fileWav = path + QString("inte_fr_%1.wav").arg(i);
        fh = fopen(fileWav.toLatin1().constData(), "rb");
        if (fh == NULL) {
            perror("Failed to open goforward.raw");
            return 1;
        }

        rv = ps_decode_raw(ps, fh, -1);
        if (rv < 0)
            continue; // return 1;
        hyp = ps_get_hyp(ps, &score);
        if (hyp == NULL)
            continue; // return 1;
        ps_seg_t *segmentation = ps_seg_iter(ps, NULL);
        while (segmentation) {
            const char *word = ps_seg_word(segmentation);
            int startFrame, endFrame;
            ps_seg_frames(segmentation, &startFrame, &endFrame);
            out << QString("%1\t%2\t%3\t%4\n").arg(i).arg(startFrame).arg(endFrame).arg(word);
            segmentation = ps_seg_next(segmentation);
        }

        // save hypothesis
        // QMessageBox::information(0, "", QString(hyp));
//        out << QString("%1\t%2\n").arg(i).arg(hyp);
        out.flush();

        fclose(fh);
    }
    fileTranscription.close();

//    fseek(fh, 0, SEEK_SET);
//    rv = ps_start_utt(ps, "goforward");
//    if (rv < 0)
//        return 1;
//    while (!feof(fh)) {
//        size_t nsamp;
//        nsamp = fread(buf, 2, 512, fh);
//        rv = ps_process_raw(ps, buf, nsamp, FALSE, FALSE);
//    }
//    rv = ps_end_utt(ps);
//    if (rv < 0)
//        return 1;
//    hyp = ps_get_hyp(ps, &score, &uttid);
//    if (hyp == NULL)
//        return 1;
//    QMessageBox::information(0, "", QString(hyp));
//    // printf("Recognized: %s\n", hyp);


    ps_free(ps);
    return 0;
}
