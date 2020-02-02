#include <QDebug>
#include <QString>
#include <QProcess>
#include <QTemporaryFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QFile>
#include <QMutex>
#include <QMutexLocker>
#include <QFileInfo>

#include "PraalineCore/Corpus/CorpusCommunication.h"
#include "PraalineCore/Annotation/Interval.h"
using namespace Praaline::Core;

#include "PraalineASR/Sphinx/SphinxConfiguration.h"
#include "PraalineASR/Sphinx/SphinxSegmentation.h"
#include "PraalineASR/Sphinx/SphinxRecogniser.h"

namespace Praaline {
namespace ASR {

struct SphinxRecogniserData {
    SphinxRecogniserData() {}

    QString defaultAcousticModel;
    QString defaultLanguageModel;
    QString defaultPronunciationDictionary;

    QString userAcousticModel;
    QString userLanguageModel;
    QString userPronunciationDictionary;
    QString userMLLRMatrix;

    bool useAcousticModelFromAttribute;
    bool useLanguageModelFromAttribute;
    bool useMLLRMatrixFromAttribute;
    QString attributename_acoustic_model;
    QString attributename_language_model;
    QString attributename_mllr_matrix;
};

SphinxRecogniser::SphinxRecogniser(QObject *parent) :
    QObject(parent), d(new SphinxRecogniserData)
{
    d->defaultAcousticModel = SphinxConfiguration::defaultModelsPath() + "/model/hmm/french_f0";
    d->defaultLanguageModel = SphinxConfiguration::defaultModelsPath() + "/model/lm/french_f0/french3g62K.lm.bin";
    d->defaultPronunciationDictionary = SphinxConfiguration::defaultModelsPath() + "/model/lm/french_f0/frenchWords62K.dic";
    d->attributename_acoustic_model = "acoustic_model";
    d->attributename_language_model = "language_model";
    d->useMLLRMatrixFromAttribute = false;
    d->useLanguageModelFromAttribute = false;
}

SphinxRecogniser::~SphinxRecogniser()
{
    delete d;
}

// ==============================================================================================================================
// Configuration
// ==============================================================================================================================

void SphinxRecogniser::setAcousticModel(const QString &pathAcousticModel)
{
    d->userAcousticModel = pathAcousticModel;
}

void SphinxRecogniser::setLanguageModel(const QString &filenameLanguageModel)
{
    d->userLanguageModel = filenameLanguageModel;
}

void SphinxRecogniser::setPronunciationDictionary(const QString &filenamePronunciationDictionary)
{
    d->userPronunciationDictionary = filenamePronunciationDictionary;
}

void SphinxRecogniser::setMLLRMatrix(const QString &filenameMLLRMatrix)
{
    d->userMLLRMatrix = filenameMLLRMatrix;
}

void SphinxRecogniser::setUseAcousticModelFromAttribute(bool use)
{
    d->useAcousticModelFromAttribute = use;
}

void SphinxRecogniser::setUseLanguageModelFromAttribute(bool use)
{
    d->useLanguageModelFromAttribute = use;
}

void SphinxRecogniser::setUseMLLRMatrixFromAttribute(bool use)
{
    d->useMLLRMatrixFromAttribute = use;
}

void SphinxRecogniser::setAttributeNames(const QString &attributenameAcousticModel, const QString &attributenameLanguageModel,
                                         const QString &attributenameMLLRMatrix)
{
    d->attributename_acoustic_model = attributenameAcousticModel;
    d->attributename_language_model = attributenameLanguageModel;
    d->attributename_mllr_matrix = attributenameMLLRMatrix;
}

// ==============================================================================================================================

bool SphinxRecogniser::recogniseUtterances_MFC(CorpusCommunication *com, QString recordingID,
                                               QList<Interval *> &utterances, QList<Interval *> &segmentation)
{
    // Batch recognition of utterances given a pre-segmented tier and a corresponding recording to which
    // feature extaction has already been applied.

    if (!com) return false;
    if (utterances.isEmpty()) return true;
    CorpusRecording *rec = com->recording(recordingID);
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

    // Configure Sphinx
    QString pathAcousticModel, filenameLanguageModel, filenamePronunciationDictionary, filenameMLLRMatrix;
    // Acoustic Model
    pathAcousticModel = d->defaultAcousticModel;
    if ((!d->userAcousticModel.isEmpty()) && (QFile::exists(d->userAcousticModel)))
        pathAcousticModel = d->userAcousticModel;
    if (d->useAcousticModelFromAttribute && (!com->property(d->attributename_acoustic_model).toString().isEmpty())) {
        QString f = rec->basePath() + "/" + com->property(d->attributename_acoustic_model).toString();
        if (QFile::exists(f)) pathAcousticModel = f;
    }
    // Language Model
    filenameLanguageModel = d->defaultLanguageModel;
    if ((!d->userLanguageModel.isEmpty()) && (QFile::exists(d->userLanguageModel)))
        filenameLanguageModel = d->userLanguageModel;
    if (d->useLanguageModelFromAttribute && (!com->property(d->attributename_language_model).toString().isEmpty())) {
        QString f = rec->basePath() + "/" + com->property(d->attributename_language_model).toString();
        if (QFile::exists(f)) filenameLanguageModel = f;
    }
    // Pronunciation Dictionary
    filenamePronunciationDictionary = d->defaultPronunciationDictionary;
    if ((!d->userPronunciationDictionary.isEmpty()) && (QFile::exists(d->userPronunciationDictionary)))
        filenamePronunciationDictionary = d->userPronunciationDictionary;
    // MLLR adaptation matrix
    filenameMLLRMatrix = "";
    if ((!d->userMLLRMatrix.isEmpty()) && (QFile::exists(d->userMLLRMatrix)))
        filenameMLLRMatrix = d->userMLLRMatrix;
    if (d->useMLLRMatrixFromAttribute && (!com->property(d->attributename_mllr_matrix).toString().isEmpty())) {
        QString f = rec->basePath() + "/" + com->property(d->attributename_mllr_matrix).toString();
        if (QFile::exists(f)) filenameMLLRMatrix = f;
    }

    QString sphinxPath = SphinxConfiguration::sphinxPath();
    QProcess sphinx;
    sphinx.setWorkingDirectory(sphinxPath);
    QStringList sphinxParams;
    // pocketsphinx_batch -argfile argFile.txt -cepdir . -ctl MYFILE.ctl -cepext .wav -adcin true -hyp MYFILE.hyp -hypseg MYFILE.seg -mllr ../PROSO2015/adapt/participant01_adapt/mllr_matrix

    sphinxParams << "-hmm" << pathAcousticModel <<
                    "-lm" << filenameLanguageModel <<
                    "-dict" << filenamePronunciationDictionary <<
                    "-cepdir" << rec->basePath() <<
                    "-ctl" << filenameCtl <<
                    "-cepext" << ".mfc" <<
                    "-hyp" << filenameHypotheses <<
                    "-hypseg" << filenameSegmentation;
    if (!filenameMLLRMatrix.isEmpty()) {
        sphinxParams << "-mllr" << filenameMLLRMatrix;
    }
    qDebug() << "pocketsphinx_batch" << sphinxParams.join(" ");
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
            int score = line.section("(", 1, 1).section(" ", 1, 1).toInt();
            utterances[i]->setText(hypothesis);
            utterances[i]->setAttribute("score", score);
        } else {
            utterances[0]->setText(line);
            utterances[0]->setAttribute("score", 0);
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
            segmentation << uttSegment->cloneReposition(uttSegment->tMin() + utterances.at(i)->tMin(),
                                                        uttSegment->tMax() + utterances.at(i)->tMin());
        }
        qDeleteAll(uttSegments);
    }
    return true;
}

} // namespace ASR
} // namespace Praaline
