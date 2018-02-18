#include <QObject>
#include <QPointer>
#include <QApplication>
#include <QTabWidget>
#include <QMessageBox>

#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/structure/AnnotationStructure.h"
#include "pncore/annotation/IntervalTier.h"
using namespace Praaline::Core;

#include "pnlib/asr/sphinx/SphinxConfiguration.h"
#include "pnlib/asr/sphinx/SphinxAutoTranscriber.h"
using namespace Praaline::ASR;

#include "pngui/widgets/CorpusItemSelectorWidget.h"
#include "pngui/widgets/GridViewWidget.h"

#include "svcore/data/model/WaveFileModel.h"
#include "svcore/data/model/SparseTimeValueModel.h"
#include "svgui/view/Pane.h"
#include "svgui/layer/WaveformLayer.h"
#include "svgui/layer/TimeRulerLayer.h"
#include "svgui/layer/TimeInstantLayer.h"
#include "svgui/layer/TimeValueLayer.h"

#include "AutomaticTranscriptionWidget.h"
#include "ui_AutomaticTranscriptionWidget.h"

struct AutomaticTranscriptionWidgetData {
    AutomaticTranscriptionWidgetData() :
        paneVisualisation(0), layerVAD(0),
        gridviewTranscription(0), transcriber(0)
    {}

    QPointer<CorpusRepository> repository;
    QPointer<Corpus> corpus;
    QPointer<CorpusCommunication> communication;
    QPointer<CorpusRecording> recording;
    QPointer<CorpusAnnotation> annotation;

    Pane *paneVisualisation;
    Layer *layerVAD;
    Layer *layerUtterances;

    GridViewWidget *gridviewTranscription;

    SphinxConfiguration config;
    SphinxAutoTranscriber *transcriber;
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
    d->config.setDirectoryAcousticModel(sphinxPath + "model/hmm/french_f0");
    d->config.setFilenamePronunciationDictionary(sphinxPath + "model/lm/french_f0/frenchWords62K.dic");
    d->config.setFilenameLanguageModel(sphinxPath + "model/lm/french_f0/french3g62K.lm.bin");

    // Automatic Transcriber module
    d->transcriber = new SphinxAutoTranscriber(this);
    d->transcriber->setConfiguration(d->config);
    connect(d->transcriber, SIGNAL(printMessage(QString)), this, SLOT(receivedMessage(QString)));
}

AutomaticTranscriptionWidget::~AutomaticTranscriptionWidget()
{
    delete ui;
    delete d;
}

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

    // Update transcriber object
    if (d->recording) {
        // Create asr directory if it does not exist
        if (!QDir(d->recording->basePath() + "/asr").exists())
            QDir().mkdir(d->recording->basePath() + "/asr");
        d->transcriber->setWorkingDirectory(d->recording->basePath() + "/asr");
        d->transcriber->setFilePathRecording(d->recording->filePath());
        ui->textMessages->appendHtml(QString("<b>Opened Communication %1, Recording %2</b>").arg(com->ID()).arg(rec->ID()));
    }

    // Visualisation
    if (!com) return;
    // Remove previous panes
    close();
    // Open each recording in the communication and add a corresponding audio panel
    bool first = false;
    foreach (QPointer<CorpusRecording> rec, com->recordings()) {
        if (!first) {
            // Main audio
            VisualiserWindowBase::FileOpenStatus status = openPath(rec->filePath());
            if (status == VisualiserWindowBase::FileOpenFailed) {
                QMessageBox::critical(this, tr("Failed to open file"),
                                      tr("<b>File open failed</b><p>File \"%1\" could not be opened").arg(rec->filePath()));
                return;
            } else if (status == VisualiserWindowBase::FileOpenWrongMode) {
                QMessageBox::critical(this, tr("Failed to open file"),
                                      tr("<b>Audio required</b><p>Unable to load layer data from \"%1\" without an audio file.<br>"
                                         "Please load at least one audio file before importing annotations.").arg(rec->filePath()));
                return;
            }
            first = true;
        }
        else {
            // import more audio
            // addRecordingToSession(rec);
        }
    }
    // Visualiser pane
    Pane *pane = m_paneStack->addPane();
    d->paneVisualisation = pane;
    if (!m_timeRulerLayer) {
        m_timeRulerLayer = m_document->createMainModelLayer(LayerFactory::Type("TimeRuler"));
    }
    // Waveform and time ruler
    m_document->addLayerToView(pane, m_timeRulerLayer);
    Layer *waveform = m_document->createMainModelLayer(LayerFactory::Type("Waveform"));
    m_document->addLayerToView(pane, waveform);
    // Superposed VAD results
    d->layerVAD = m_document->createEmptyLayer(LayerFactory::Type("TimeValues"));
    TimeValueLayer *layerVAD = qobject_cast<TimeValueLayer *>(d->layerVAD);
    if (layerVAD) {
        layerVAD->setPlotStyle(TimeValueLayer::PlotCurve);
        layerVAD->setDrawSegmentDivisions(false);
        layerVAD->setDisplayExtents(-1.0, 1.0);
    }
    m_document->addLayerToView(pane, d->layerVAD);
    // Superposed utterance boundaries
    d->layerUtterances = m_document->createEmptyLayer(LayerFactory::Type("TimeInstants"));
    m_document->addLayerToView(pane, d->layerUtterances);

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

void AutomaticTranscriptionWidget::receivedMessage(QString text)
{
    ui->textMessages->appendHtml(text);
}

// ====================================================================================================================

void AutomaticTranscriptionWidget::stepPrepare()
{
    d->transcriber->stepExtractFeaturesFile();
}

void AutomaticTranscriptionWidget::stepVAD()
{
    if (!d->transcriber->stepVoiceActivityDetection()) return;
    // Visualise results
    sv_samplerate_t sampleRate = getMainModel()->getSampleRate();
    // VAD results (probability of frame being speech)
    QPair<double, double> v;
    SparseTimeValueModel *modelVAD = qobject_cast<SparseTimeValueModel *>(d->layerVAD->getModel());
    if (modelVAD) modelVAD->clear();
    foreach (v, d->transcriber->vadResults()) {
        RealTime t = RealTime::fromSeconds(v.first);
        SparseTimeValueModel::Point vadPoint(RealTime::realTime2Frame(t, sampleRate), v.second, QString());
        modelVAD->addPoint(vadPoint);
    }
    // Utterance boundaries
    SparseOneDimensionalModel *modelUtterances = qobject_cast<SparseOneDimensionalModel *>(d->layerUtterances->getModel());
    foreach(Interval *intv, d->transcriber->utterances()) {
        SparseOneDimensionalModel::Point point(RealTime::realTime2Frame(intv->tMin(), sampleRate), "");
        modelUtterances->addPoint(point);
    }
    // Update grid with new utterances

}

void AutomaticTranscriptionWidget::stepAutoTranscribe()
{
    d->transcriber->stepAutoTranscribe();
}

void AutomaticTranscriptionWidget::stepSpeakerAdaptation()
{

}

