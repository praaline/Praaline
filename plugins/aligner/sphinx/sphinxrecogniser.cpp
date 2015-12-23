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
    bool useSpecialisedLM;
    QString defaultAcousticModel;
    QString defaultLanguageModel;
    QString defaultPronunciationDictionary;

    QString attributename_acoustic_model;
    QString attributename_language_model;
};

SphinxRecogniser::SphinxRecogniser(QObject *parent) :
    QObject(parent), d(new SphinxRecogniserData)
{
    d->defaultAcousticModel = "D:/SPHINX/pocketsphinx-0.8/model/hmm/french_f0";
    d->defaultLanguageModel = "D:/SPHINX/pocketsphinx-0.8/model/lm/french_f0/french3g62K.lm.dmp";
    d->defaultPronunciationDictionary = "D:/SPHINX/pocketsphinx-0.8/model/lm/french_f0/frenchWords62K.dic";
    d->attributename_acoustic_model = "acoustic_model";
    d->attributename_language_model = "language_model";
    d->useMLLR = false;
    d->useSpecialisedLM = false;
}

SphinxRecogniser::~SphinxRecogniser()
{
    delete d;
}

void SphinxRecogniser::setUseMLLR(bool use)
{
    d->useMLLR = use;
}

void SphinxRecogniser::setUseSpecialisedLM(bool use)
{
    d->useSpecialisedLM = use;
}

void SphinxRecogniser::setAttributeNames(const QString &attributenameAcousticModel, const QString &attributenameLanguageModel)
{
    d->attributename_acoustic_model = attributenameAcousticModel;
    d->attributename_language_model = attributenameLanguageModel;
}

bool SphinxRecogniser::recogniseUtterances_MFC(QPointer<CorpusCommunication> com, QString recordingID,
                                               QList<Interval *> &utterances, QList<Interval *> &segmentation)
{
    // Batch recognition of utterances given a pre-segmented tier and a corresponding recording to which
    // feature extaction has already been applied.

    if (!com) return false;
    if (utterances.isEmpty()) return true;
    QPointer<CorpusRecording> rec = com->recording(recordingID);
    if (!rec) return false;

    // File names
    QString fileID = QString(rec->filename()).replace(".wav", "");
    QString filenameCtl = rec->basePath() + "/" + fileID + ".ctl";
    QString filenameHypotheses = rec->basePath() + "/" + fileID + ".hyp";
    QString filenameSegmentation = rec->basePath() + "/" + fileID + ".seg";

    // Create fileids file
    QFile fileCtl(filenameCtl);
    if ( !fileCtl.open( QIODevice::WriteOnly | QIODevice::Text ) ) return false;
    QTextStream ctl(&fileCtl);
    ctl.setCodec("ISO 8859-1");
    // Write utterances to transcribe
    int i = 0;
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
        filenameMLLRMatrix = com->basePath() + "/" + com->property(d->attributename_acoustic_model).toString() + "/mllr_matrix";
    } else {
        filenameAcousticModel = com->property(d->attributename_acoustic_model).toString();
        if (filenameAcousticModel.isEmpty()) filenameAcousticModel = d->defaultAcousticModel;
        else filenameAcousticModel = com->basePath() + "/" + filenameAcousticModel;
    }
    if (d->useSpecialisedLM) {
        filenameLanguageModel = com->property(d->attributename_language_model).toString();
        if (filenameLanguageModel.isEmpty()) filenameLanguageModel = d->defaultLanguageModel;
        else filenameLanguageModel = com->basePath() + "/" + filenameLanguageModel;
    } else {
        filenameLanguageModel = d->defaultLanguageModel;
    }

    QString appPath = QCoreApplication::applicationDirPath();
    QString sphinxPath = appPath + "/plugins/aligner/sphinx/";
    QProcess sphinx;
    sphinx.setWorkingDirectory(sphinxPath);
    QStringList sphinxParams;
    // pocketsphinx_batch -argfile argFile.txt -cepdir . -ctl MYFILE.ctl -cepext .wav -adcin true -hyp MYFILE.hyp -hypseg MYFILE.seg -mllr ../PROSO2015/adapt/participant01_adapt/mllr_matrix

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

    return readRecognitionResults(fileID, filenameHypotheses, filenameSegmentation, utterances, segmentation);
}

bool SphinxRecogniser::readRecognitionResults(const QString &fileID, const QString &filenameHypotheses, const QString filenameSegmentation,
                                              QList<Interval *> &utterances, QList<Interval *> &segmentation)
{
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
