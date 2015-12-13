#include <QDebug>
#include <QString>
#include <QProcess>
#include <QTemporaryFile>
#include <QTextStream>
#include <QCoreApplication>

#include "pncore/corpus/corpus.h"

#include "sphinxsegmentation.h"
#include "sphinxrecogniser.h"

struct SphinxRecogniserData {
    SphinxRecogniserData() {}

    bool useMLLR;
    QString defaultAcousticModel;
    QString defaultLanguageModel;
    QString defaultPronunciationDictionary;
};

SphinxRecogniser::SphinxRecogniser(QObject *parent) :
    QObject(parent), d(new SphinxRecogniserData)
{
    d->defaultAcousticModel = "D:/SPHINX/pocketsphinx-0.8/model/hmm/french_f0";
    d->defaultLanguageModel = "D:/SPHINX/pocketsphinx-0.8/model/lm/french_f0/french3g62K.lm.dmp";
    d->defaultPronunciationDictionary = "F:/CORPUS_THESIS_EXPERIMENT/PROSO2015/adapt/french_dict.dic";
}

SphinxRecogniser::~SphinxRecogniser()
{
    delete d;
}

void SphinxRecogniser::setUseMLLR(bool use)
{
    d->useMLLR = use;
}

bool SphinxRecogniser::recogniseUtterances_MFC(QPointer<CorpusCommunication> com, QString recordingID, QList<Interval *> &utterances, QList<Interval *> &segmentation)
{
    // Batch recognition of utterances given a pre-segmented tier and a corresponding recording to which
    // feature extaction has already been applied.

    if (!com) return false;
    if (utterances.isEmpty()) return true;
    QPointer<CorpusRecording> rec = com->recording(recordingID);
    if (!rec) return false;

    // Create fileids file
    // Get a temporary file and write out the utterances
    QString filenameCtl;
    QTemporaryFile fileCtl;
    if (!fileCtl.open()) return false;
    filenameCtl = fileCtl.fileName();
    QTextStream ctl(&fileCtl);
    ctl.setCodec("ISO 8859-1");
    int i = 0;
    QString fileID = QString(rec->filename()).replace(".wav", "");
    if (utterances.count() > 1) {
        foreach (Interval *utterance, utterances) {
            QString utteranceID = fileID + QString("_%1").arg(i);
            qint64 frameStart = utterance->tMin().toNanoseconds() / 10000000L - 5;
            if (frameStart < 0) frameStart = 0;
            qint64 frameEnd = utterance->tMax().toNanoseconds() / 10000000L;
            if (i < utterances.count() - 1)
                frameEnd = frameEnd + 5;
            else
                frameEnd = frameEnd - 5;
            ctl << fileID << " " << frameStart << " " << frameEnd << " " << utteranceID << "\n";
            i++;
        }
    } else {
        ctl << fileID << "\n";
    }
    fileCtl.close();

    QString filenameAcousticModel, filenameLanguageModel, filenameMLLRMatrix;
    if (d->useMLLR) {
        filenameAcousticModel = d->defaultAcousticModel;
        filenameMLLRMatrix = com->basePath() + "/" + com->property("model_acoustic").toString() + "/mllr_matrix";
    } else {
        filenameAcousticModel = com->property("model_acoustic").toString();
        if (filenameAcousticModel.isEmpty()) filenameAcousticModel = d->defaultAcousticModel;
        else filenameAcousticModel = com->basePath() + "/" + filenameAcousticModel;
    }
    filenameLanguageModel = com->property("model_language").toString();
    if (filenameLanguageModel.isEmpty()) filenameLanguageModel = d->defaultLanguageModel;
    else filenameLanguageModel = com->basePath() + "/" + filenameLanguageModel;

    QString filenameHypotheses = rec->basePath() + "/" + fileID + ".hyp";
    QString filenameSegmentation = rec->basePath() + "/" + fileID + ".seg";

    QString appPath = QCoreApplication::applicationDirPath();
    QString sphinxPath = appPath + "/plugins/aligner/sphinx/";
    QProcess sphinx;
    sphinx.setWorkingDirectory(sphinxPath);
    QStringList sphinxParams;
    // pocketsphinx_batch -argfile argFile.txt -cepdir . -ctl participant01econ.ctl -cepext .wav -adcin true -hyp participant01econ.hyp -hypseg participant01econ.seg -mllr ../PROSO2015/adapt/participant01_adapt/mllr_matrix

    sphinxParams << "-hmm" << filenameAcousticModel <<
                    "-lm" << filenameLanguageModel <<
                    "-dict" << d->defaultPronunciationDictionary <<
                    "-cepdir" << rec->basePath() <<
                    "-ctl" << filenameCtl <<
                    "-cepext" << ".mfc" <<
                    "-hyp" << filenameHypotheses <<
                    "-hypseg" << filenameSegmentation;
    if (d->useMLLR) {
        sphinxParams << "-mllr" << filenameMLLRMatrix;
    }
    sphinx.start(sphinxPath + "pocketsphinx_batch", sphinxParams);
    if (!sphinx.waitForStarted(-1)) return false;
    if (!sphinx.waitForFinished(-1)) return false;
    QString out = QString(sphinx.readAllStandardOutput());
    qDebug() << out;
    QString err = QString(sphinx.readAllStandardError());
    qDebug() << err;

    // Read utterance hypotheses into intervals
    QFile fileHyp(filenameHypotheses);
    if ( !fileHyp.open( QIODevice::ReadOnly | QIODevice::Text ) ) return false;
    QTextStream hyp(&fileHyp);
    hyp.setCodec("UTF-8");
    while (!hyp.atEnd()) {
        QString line = hyp.readLine().trimmed();
        if (line.isEmpty()) continue;
        if (utterances.count() > 1) {
            QString utteranceID = line.section(" ", -2, -2);
            if (!utteranceID.startsWith("(")) continue;
            utteranceID = utteranceID.section("_", -1, -1);
            qDebug() << utteranceID;
            bool ok;
            int i = utteranceID.toInt(&ok);
            if (!ok) continue;
            if ((i < 0) || (i >= utterances.count())) continue;
            QString hypothesis = line.section("(", 0, 0);
            utterances[i]->setText(hypothesis);
        } else {
            utterances[0]->setText(line);
        }
    }

    // Optional - read segmentation
    QHash<QString, QList<Interval *> > segmentsHash;
    SphinxSegmentation::readSegmentationHypothesisFile(filenameSegmentation, segmentsHash);
    segmentation.clear();
    for (int i = 0; i < utterances.count(); ++i) {
        QString utteranceID;
        if (utterances.count() > 1) {
            utteranceID = fileID + QString("_%1").arg(i);
        } else {
            utteranceID = fileID;
        }
        QList<Interval *> uttSegments = segmentsHash.value(utteranceID);
        foreach (Interval *uttSegment, uttSegments) {
            segmentation << new Interval(uttSegment->tMin() + utterances.at(i)->tMin(),
                                         uttSegment->tMax() + utterances.at(i)->tMin(), uttSegment);
        }
        qDeleteAll(uttSegments);
    }
    return true;
}

