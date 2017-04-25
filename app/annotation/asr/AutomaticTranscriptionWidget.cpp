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

    QFuture<QString> future;
    QFutureWatcher<QString> watcher;

    SpeechRecognitionRecipes::Configuration config;
    SphinxOfflineRecogniser *recogniser;
};

AutomaticTranscriptionWidget::AutomaticTranscriptionWidget(QWidget *parent) :
    ASRModuleWidgetBase(parent), ui(new Ui::AutomaticTranscriptionWidget), d(new AutomaticTranscriptionWidgetData)
{
    ui->setupUi(this);

    // Annotation Level and Attributes
    connect(ui->comboBoxAnnotationLevel, SIGNAL(currentTextChanged(QString)), this, SLOT(annotationLevelChanged(QString)));

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
    if (d->repository != corpus->repository()) {
        if (!corpus->repository()) return;
        d->repository = corpus->repository();
        ui->comboBoxAnnotationLevel->clear();
        ui->comboBoxAnnotationAttributeASR->clear();
        ui->comboBoxAnnotationAttributeCompare->clear();
        foreach (AnnotationStructureLevel *level, d->repository->annotationStructure()->levels()) {
            ui->comboBoxAnnotationLevel->addItem(level->name(), level->ID());
        }
        annotationLevelChanged("");
    }
    if ((d->corpus != corpus) || (d->communication != com) || (d->recording != rec) || (d->annotation != annot)) {
        d->corpus = corpus;
        d->communication = com;
        d->recording = rec;
        d->annotation = annot;
        prepareFile();
    }
}

void AutomaticTranscriptionWidget::annotationLevelChanged(QString text)
{
    Q_UNUSED(text)
    if (!d->repository) return;
    QString levelID = ui->comboBoxAnnotationLevel->currentData().toString();
    AnnotationStructureLevel *level = d->repository->annotationStructure()->level(levelID);
    if (!level) return;
    ui->comboBoxAnnotationAttributeASR->clear();
    ui->comboBoxAnnotationAttributeCompare->clear();
    ui->comboBoxAnnotationAttributeASR->addItem("(text)", "");
    ui->comboBoxAnnotationAttributeCompare->addItem("(text)", "");
    int i = 0;
    foreach (AnnotationStructureAttribute *attribute, level->attributes()) {
        ui->comboBoxAnnotationAttributeASR->addItem(attribute->name(), attribute->ID());
        ui->comboBoxAnnotationAttributeCompare->addItem(attribute->name(), attribute->ID());
        i++;
    }
}

void AutomaticTranscriptionWidget::prepareFile()
{
    if (!d->recording) return;
    ui->textMessages->clear();
    if (!QFile::exists(d->recording->filePath())) {
        ui->textMessages->appendHtml(QString("File <i>%1</i> does not exist.").append(d->recording->filePath()));
        return;
    }
    QString filepathDownsampled = d->recording->basePath() + "/asr/" + QString(d->recording->filename()).replace(".wav", ".16k");
    QString filepathFeaturesMFC = d->recording->basePath() + "/asr/" + QString(d->recording->filename()).replace(".wav", ".mfc");
    if (!QFile::exists(filepathFeaturesMFC)) {
        ui->textMessages->appendHtml(QString("Preparing recording for automatic speech recognition."));
        if (!QDir(d->recording->basePath() + "/asr").exists()) {
            QDir(d->recording->basePath()).mkdir("asr");
        }
        SpeechRecognitionRecipes::downsampleWaveFile(d->recording, d->recording->basePath() + "/asr");
    }


    // SpeechRecognitionRecipes::createSphinxFeatureFile(d->recording, d->config);
}
