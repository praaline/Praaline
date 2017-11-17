#include <QObject>
#include <QPointer>
#include <QApplication>
#include <QElapsedTimer>
#include <QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>
#include <QMutex>
#include <QTabWidget>

#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/structure/AnnotationStructure.h"
using namespace Praaline::Core;

#include "pnlib/asr/SpeechRecognitionRecipes.h"
#include "pnlib/asr/sphinx/SphinxOfflineRecogniser.h"
using namespace Praaline::ASR;

#include "pngui/widgets/CorpusItemSelectorWidget.h"
#include "pngui/widgets/GridViewWidget.h"

#include "AutomaticTranscriptionWidget.h"
#include "ui_AutomaticTranscriptionWidget.h"

struct AutomaticTranscriptionWidgetData {
    AutomaticTranscriptionWidgetData()
    {}

    QPointer<CorpusRepository> repository;
    QPointer<Corpus> corpus;
    QPointer<CorpusCommunication> communication;
    QPointer<CorpusRecording> recording;
    QPointer<CorpusAnnotation> annotation;

    GridViewWidget *gridviewTranscription;

    QString filepathDownsampled;
    QString filepathFeaturesMFC;

    QFuture<QString> future;
    QFutureWatcher<QString> watcher;

    SpeechRecognitionRecipes::Configuration config;
    SphinxOfflineRecogniser *recogniser;
};

AutomaticTranscriptionWidget::AutomaticTranscriptionWidget(QWidget *parent) :
    ASRModuleVisualiserWidgetBase(parent), ui(new Ui::AutomaticTranscriptionWidget), d(new AutomaticTranscriptionWidgetData)
{
    ui->setupUi(this);

    // UI Visualiser and transcription grid
    ui->gridLayoutVisualiser->addWidget(m_visualiserScroll);
    d->gridviewTranscription = new GridViewWidget(this);
    d->gridviewTranscription->tableView()->verticalHeader()->setDefaultSectionSize(20);
    ui->gridLayoutTranscription->addWidget(d->gridviewTranscription);

    // Annotation Level and Attributes
    connect(ui->comboBoxAnnotationLevel, SIGNAL(currentTextChanged(QString)), this, SLOT(annotationLevelChanged(QString)));

    // Processing steps
    connect(ui->commandPrepare, SIGNAL(clicked(bool)), this, SLOT(stepPrepare()));
    connect(ui->commandVAD, SIGNAL(clicked(bool)), this, SLOT(stepVAD()));
    connect(ui->commandAutoTranscribe, SIGNAL(clicked(bool)), this, SLOT(stepAutoTranscribe()));
    connect(ui->commandSpeakerAdaptation, SIGNAL(clicked(bool)), this, SLOT(stepSpeakerAdaptation()));

    // Sphinx configuration
    QString sphinxPath = QCoreApplication::applicationDirPath() + "/plugins/aligner/sphinx/";
    d->config.sphinxHMModelPath = sphinxPath + "model/hmm/french_f0";
    d->config.sphinxPronunciationDictionary = sphinxPath + "model/lm/french_f0/frenchWords62K.dic";
    d->config.sphinxLanguageModelPath = sphinxPath + "model/lm/french_f0/french3g62K.lm.bin";

    connect(&(d->watcher), SIGNAL(resultReadyAt(int)), this, SLOT(futureResultReadyAt(int)));
    connect(&(d->watcher), SIGNAL(progressValueChanged(int)), this, SLOT(futureProgressValueChanged(int)));
    connect(&(d->watcher), SIGNAL(finished()), this, SLOT(futureFinished()));

    d->recogniser = new SphinxOfflineRecogniser(this);
}

AutomaticTranscriptionWidget::~AutomaticTranscriptionWidget()
{
    delete ui;
    delete d;
}

void AutomaticTranscriptionWidget::futureResultReadyAt(int index)
{
    QString result = d->watcher.resultAt(index);
    ui->textMessages->appendPlainText(result);
    ui->progressBar->setValue(d->watcher.progressValue() * 100 / d->watcher.progressMaximum());
}

void AutomaticTranscriptionWidget::futureProgressValueChanged(int progressValue)
{
    if (d->watcher.progressMaximum() > 0)
        ui->progressBar->setValue(progressValue * 100 / d->watcher.progressMaximum());
    else
        ui->progressBar->setValue(100);
}

void AutomaticTranscriptionWidget::futureFinished()
{
    ui->progressBar->setValue(100);
    ui->textMessages->appendPlainText("Finished");
}

struct AutomaticTranscriptionWidget::SphinxAutomaticTranscriptionStep
{
    SphinxAutomaticTranscriptionStep(
            const QString &sphinxAcousticModel, const QString &sphinxLanguageModel,
            const QString &sphinxPronunciationDictionary, const QString &sphinxMLLRMatrix)
        : m_sphinxAcousticModel(sphinxAcousticModel), m_sphinxLanguageModel(sphinxLanguageModel),
          m_sphinxPronunciationDictionary(sphinxPronunciationDictionary), m_sphinxMLLRMatrix(sphinxMLLRMatrix) { }
    typedef QString result_type;

    QString operator() (const QPointer<CorpusCommunication> &com)
    {
        if (!com) return QString("%1\tis empty.").arg(com->ID());
        foreach (QPointer<CorpusRecording> rec, com->recordings()) {
            int id = rec->ID().left(3).right(2).toInt();
            if (id > 10) return QString("%1 ok").arg(id);

            bool result = false; /*SpeechRecognitionRecipes::transcribeUtterancesWithSphinx(
                        com, rec, rec->ID(), "auto_segment", "auto_transcription",
                        m_sphinxAcousticModel, m_sphinxLanguageModel,
                        m_sphinxPronunciationDictionary, m_sphinxMLLRMatrix);*/
            if (result) {
//                SpeechRecognitionRecipes::updateSegmentationFromTranscription(
//                            com, "auto_segment", "auto_transcription");
            }
        }
        return QString("%1\tautomatically transcribed %2 recording(s) using Sphinx.").arg(com->ID()).arg(com->recordingsCount());
    }
    QString m_sphinxAcousticModel;
    QString m_sphinxLanguageModel;
    QString m_sphinxPronunciationDictionary;
    QString m_sphinxMLLRMatrix;
};

void AutomaticTranscriptionWidget::open(Corpus *corpus, CorpusCommunication *com, CorpusRecording *rec, CorpusAnnotation *annot)
{
    if (!corpus) return;
    if ((d->corpus == corpus) && (d->communication == com) && (d->recording == rec) && (d->annotation == annot)) return;

    if (d->repository != corpus->repository()) {
        if (!corpus->repository()) return;
        d->repository = corpus->repository();
        ui->comboBoxAnnotationLevel->clear();
        ui->comboBoxAnnotationAttributeASR->clear();
        // ui->comboBoxAnnotationAttributeCompare->clear();
        foreach (AnnotationStructureLevel *level, d->repository->annotationStructure()->levels()) {
            ui->comboBoxAnnotationLevel->addItem(level->name(), level->ID());
        }
        annotationLevelChanged("");
    }

    d->corpus = corpus;
    d->communication = com;
    d->recording = rec;
    d->annotation = annot;



}

void AutomaticTranscriptionWidget::annotationLevelChanged(QString text)
{
    Q_UNUSED(text)
    if (!d->repository) return;
    QString levelID = ui->comboBoxAnnotationLevel->currentData().toString();
    AnnotationStructureLevel *level = d->repository->annotationStructure()->level(levelID);
    if (!level) return;
    ui->comboBoxAnnotationAttributeASR->clear();
    // ui->comboBoxAnnotationAttributeCompare->clear();
    ui->comboBoxAnnotationAttributeASR->addItem("(text)", "");
    // ui->comboBoxAnnotationAttributeCompare->addItem("(text)", "");
    foreach (AnnotationStructureAttribute *attribute, level->attributes()) {
        ui->comboBoxAnnotationAttributeASR->addItem(attribute->name(), attribute->ID());
        // ui->comboBoxAnnotationAttributeCompare->addItem(attribute->name(), attribute->ID());
    }
}

// ====================================================================================================================

void AutomaticTranscriptionWidget::stepPrepare()
{
    if (!d->recording) return;
    ui->textMessages->clear();
    if (!QFile::exists(d->recording->filePath())) {
        ui->textMessages->appendHtml(QString("File <i>%1</i> does not exist.").append(d->recording->filePath()));
        return;
    }
    QString filepathDownsampled = d->recording->basePath() + "/asr/" + QString(d->recording->filename()).replace(".wav", ".16k.wav");
    QString filepathFeaturesMFC = d->recording->basePath() + "/asr/" + QString(d->recording->filename()).replace(".wav", ".16k.mfc");
    if (!QFile::exists(filepathFeaturesMFC)) {
        ui->textMessages->appendHtml(QString("Preparing recording for automatic speech recognition."));
        if (!QDir(d->recording->basePath() + "/asr").exists()) {
            QDir(d->recording->basePath()).mkdir("asr");
        }
        if (!QFile::exists(filepathDownsampled)) {
            SpeechRecognitionRecipes::downsampleWaveFile(d->recording, d->recording->basePath() + "/asr");
            ui->textMessages->appendHtml(QString("Created downsampled file."));
            QApplication::processEvents();
        }
        SpeechRecognitionRecipes::createSphinxFeatureFiles(QStringList() << filepathDownsampled, d->config);
        ui->textMessages->appendHtml(QString("Created MFCC feature file for Sphinx."));
    }
    d->filepathDownsampled = filepathDownsampled;
    d->filepathFeaturesMFC = filepathFeaturesMFC;
    ui->textMessages->appendHtml(QString("Loaded recording %1.").arg(d->recording->ID()));
}

void AutomaticTranscriptionWidget::stepVAD()
{

}

void AutomaticTranscriptionWidget::stepAutoTranscribe()
{

}

void AutomaticTranscriptionWidget::stepSpeakerAdaptation()
{

}

