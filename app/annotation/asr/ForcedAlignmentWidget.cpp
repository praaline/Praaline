#include <QString>
#include <QList>
#include <QPointer>
#include <QMap>
#include <QMessageBox>

#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/structure/AnnotationStructure.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/annotation/IntervalTier.h"
using namespace Praaline::Core;

// Praaline GUI widgets
#include "pngui/widgets/CorpusItemSelectorWidget.h"
#include "pngui/widgets/GridViewWidget.h"
#include "pngui/widgets/StatusMessagesWidget.h"

// Visualiser
#include "svcore/data/model/WaveFileModel.h"
#include "svcore/data/model/SparseTimeValueModel.h"
#include "svgui/view/Pane.h"
#include "svgui/layer/ColourDatabase.h"
#include "svgui/layer/WaveformLayer.h"
#include "svgui/layer/TimeRulerLayer.h"
#include "svgui/layer/TimeInstantLayer.h"
#include "svgui/layer/TimeValueLayer.h"
#include "pngui/layer/AnnotationGridLayer.h"

// Data models
#include "pngui/model/visualiser/AnnotationGridModel.h"
#include "pngui/model/annotation/AnnotationMultiTierTableModel.h"

// Property grid
#include "qtpropertymanager.h"
#include "qtvariantproperty.h"
#include "qttreepropertybrowser.h"

#include "pnlib/asr/htk/HTKForcedAligner.h"
#include "pnlib/asr/syllabifier/SyllabifierEasy.h"
using namespace Praaline::ASR;

#include "ForcedAlignmentWidget.h"
#include "ui_ForcedAlignmentWidget.h"

struct ForcedAlignmentWidgetData {
    ForcedAlignmentWidgetData() :
        repository(0), corpus(0), communication(0), recording(0), annotation(0),
        paneWave(0), paneAnnotation(0), layerAnnotation(0),
        gridviewTranscription(0),
        propertyVariantManagerConfigParameters(0), propertyVariantFactoryConfigParameters(0), propertyBrowserConfigParameters(0),
        statusMessages(0),
        tierModel(0)
    {}

    QPointer<CorpusRepository> repository;
    QPointer<Corpus> corpus;
    QPointer<CorpusCommunication> communication;
    QPointer<CorpusRecording> recording;
    QPointer<CorpusAnnotation> annotation;
    // Visualiser
    Pane *paneWave;
    Pane *paneAnnotation;
    Layer *layerAnnotation;
    // Utterance grid
    GridViewWidget *gridviewTranscription;
    // Tokenisation-Phonetisation grid
    // Configuration parameters
    QtVariantPropertyManager *propertyVariantManagerConfigParameters;
    QtVariantEditorFactory *propertyVariantFactoryConfigParameters;
    QtTreePropertyBrowser *propertyBrowserConfigParameters;
    QHash<QString, QtProperty *> propertiesConfig;
    // QHash<QString, QHash<QString, QtProperty *> > propertiesParameters;
    // Status messages
    StatusMessagesWidget *statusMessages;
    // Annotation tiers
    QMap<QString, QPointer<AnnotationTierGroup> > currentTierGroups;
    AnnotationMultiTierTableModel *tierModel;
};

ForcedAlignmentWidget::ForcedAlignmentWidget(QWidget *parent) :
    ASRModuleVisualiserWidgetBase(parent), ui(new Ui::ForcedAlignmentWidget), d(new ForcedAlignmentWidgetData)
{
    ui->setupUi(this);

    // UI Visualiser and transcription grid
    ui->dockWidgetVisualiser->setMinimumHeight(250);
    ui->gridLayoutVisualiser->addWidget(m_visualiserScroll);
    d->gridviewTranscription = new GridViewWidget(this);
    d->gridviewTranscription->tableView()->verticalHeader()->setDefaultSectionSize(20);
    d->gridviewTranscription->setResultsLabelVisible(false);
    ui->gridLayoutTranscription->addWidget(d->gridviewTranscription);

    // Status messages output widget
    d->statusMessages = new StatusMessagesWidget(this);
    ui->gridLayoutMessages->addWidget(d->statusMessages);

    // Processing steps
    connect(ui->commandPhonetiseAll, SIGNAL(clicked(bool)), this, SLOT(stepPhonetiseAll()));
    connect(ui->commandAlignAll, SIGNAL(clicked(bool)), this, SLOT(stepAlignAll()));
    connect(ui->commandSpeakerAdaptation, SIGNAL(clicked(bool)), this, SLOT(stepSpeakerAdaptation()));

    // Plugin parameters property browser
    d->propertyVariantManagerConfigParameters = new QtVariantPropertyManager(this);
    d->propertyVariantFactoryConfigParameters = new QtVariantEditorFactory(this);
    d->propertyBrowserConfigParameters = new QtTreePropertyBrowser(this);
    d->propertyBrowserConfigParameters->setFactoryForManager(d->propertyVariantManagerConfigParameters, d->propertyVariantFactoryConfigParameters);
    d->propertyBrowserConfigParameters->setPropertiesWithoutValueMarked(true);
    d->propertyBrowserConfigParameters->setRootIsDecorated(false);
    d->propertyBrowserConfigParameters->show();
    ui->gridLayoutConfig->addWidget(d->propertyBrowserConfigParameters);
}

ForcedAlignmentWidget::~ForcedAlignmentWidget()
{
    delete ui;
    delete d;
}

void ForcedAlignmentWidget::open(Corpus *corpus, CorpusCommunication *com, CorpusRecording *rec, CorpusAnnotation *annot)
{
    if (!corpus) return;
    if ((d->corpus == corpus) && (d->communication == com) && (d->recording == rec) && (d->annotation == annot)) return;

    d->corpus = corpus;
    d->communication = com;
    d->recording = rec;
    d->annotation = annot;
    d->repository = corpus->repository();

    // Close previously opened file (base class method)
    close();

    // Open the selected recording
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

    // Read annotation tiers
    qDeleteAll(d->currentTierGroups);
    d->currentTierGroups.clear();
    if (annot && d->repository) {
        d->currentTierGroups = d->repository->annotations()->getTiersAllSpeakers(annot->ID());
    }

    // Visualiser pane
    d->paneWave = m_paneStack->addPane();
    if (!m_timeRulerLayer) {
        m_timeRulerLayer = m_document->createMainModelLayer(LayerFactory::Type("TimeRuler"));
    }
    // Waveform and time ruler
    m_document->addLayerToView(d->paneWave, m_timeRulerLayer);
    Layer *waveform = m_document->createMainModelLayer(LayerFactory::Type("Waveform"));
    m_document->addLayerToView(d->paneWave, waveform);
    // Set zoom in number of frames per pixel. The sample rate is the number of frames per second.
    d->paneWave->setZoomLevel(getMainModel()->getSampleRate() / 250);

    // Annotation grid
    QList<QPair<QString, QString> > annotationAttributes;
    annotationAttributes << QPair<QString, QString>("phone", "");
    annotationAttributes << QPair<QString, QString>("syll", "");
    annotationAttributes << QPair<QString, QString>("tok_min", "");
    annotationAttributes << QPair<QString, QString>("transcription", "");
    AnnotationGridModel *model = new AnnotationGridModel(getMainModel()->getSampleRate(), d->currentTierGroups, annotationAttributes);
    // Create a pane + layer for the annotations
    d->paneAnnotation = m_paneStack->addPane();
    Layer *annotLayer = m_document->createImportedLayer(model);
    qobject_cast<AnnotationGridLayer *>(annotLayer)->setPlotStyle(AnnotationGridLayer::PlotBlendedSpeakers);
    qobject_cast<AnnotationGridLayer *>(annotLayer)->setBaseColour(ColourDatabase::getInstance()->getColourIndex(tr("Black")));
    m_document->addLayerToView(d->paneAnnotation, m_timeRulerLayer);
    m_timeRulerLayer->setLayerDormant(d->paneAnnotation, true);
    m_document->addLayerToView(d->paneAnnotation, annotLayer);
    m_paneStack->setCurrentPane(d->paneAnnotation);
    m_paneStack->setCurrentLayer(d->paneAnnotation, annotLayer);

    // Show transcription tiers
    d->gridviewTranscription->tableView()->setModel(nullptr);
    if (d->tierModel) delete d->tierModel;
    d->tierModel = new AnnotationMultiTierTableModel(d->currentTierGroups, "transcription", annotationAttributes, Qt::Vertical);
    d->gridviewTranscription->tableView()->setModel(d->tierModel);
}

void ForcedAlignmentWidget::stepPhonetiseAll()
{
    d->statusMessages->appendMessage("Phonetise");
}

void ForcedAlignmentWidget::stepAlignAll()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Realign entire file?"),
                                  tr("Are you sure you want to realign all utterances? You will lose all adjustements "
                                     "to the alignement boundaries made manually."), QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::No) return;
    d->statusMessages->appendMessage("Align All Utterances");
    if (!d->recording) {
        d->statusMessages->appendMessage("No recording selected.");
        return;
    }
    if (!d->annotation) {
        d->statusMessages->appendMessage("No annotation selected.");
        return;
    }
    QString annotationID = d->annotation->ID();
    foreach (QString speakerID, d->currentTierGroups.keys()) {
        QPointer<AnnotationTierGroup> tiers = d->currentTierGroups.value(speakerID);
        if (!tiers) continue;
        IntervalTier *tier_tokens = tiers->getIntervalTierByName("tok_min");
        if (!tier_tokens) continue;
        d->statusMessages->appendMessage(QString("Speaker: %1").arg(speakerID));
        QList<Interval *> list_phones;
        QString alignerOutput;
        HTKForcedAligner aligner;
        bool ok = aligner.alignAllTokens(d->recording->filePath(), tier_tokens, list_phones, alignerOutput);
        d->statusMessages->appendMessage(alignerOutput);

        if (ok) {
            IntervalTier *tier_phone = new IntervalTier("phone", list_phones);
            IntervalTier *tier_syll= SyllabifierEasy::syllabify(tier_phone);
            tier_syll->setName("syll");
            d->repository->annotations()->saveTier(annotationID, speakerID, tier_tokens);
            d->repository->annotations()->saveTier(annotationID, speakerID, tier_phone);
            d->repository->annotations()->saveTier(annotationID, speakerID, tier_syll);
        }
    }
}

void ForcedAlignmentWidget::stepSpeakerAdaptation()
{
    d->statusMessages->appendMessage("Speaker Adaptation");
}
