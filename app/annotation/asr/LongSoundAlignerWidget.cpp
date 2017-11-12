#include <QObject>
#include <QPointer>
#include <QGridLayout>
#include <QScrollArea>
#include <QFrame>
#include <QMenu>
#include <QMessageBox>

#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/structure/AnnotationStructure.h"
#include "pncore/annotation/IntervalTier.h"
using namespace Praaline::Core;

#include "svcore/base/Preferences.h"
#include "svcore/base/PlayParameterRepository.h"
#include "svcore/data/model/WaveFileModel.h"
#include "svgui/view/Pane.h"
#include "svgui/view/PaneStack.h"
#include "svgui/view/Overview.h"
#include "svgui/layer/WaveformLayer.h"
#include "svgui/layer/TimeRulerLayer.h"
#include "svgui/widgets/AudioDial.h"
#include "svgui/widgets/Fader.h"
#include "svgui/widgets/CommandHistory.h"
#include "svgui/widgets/ProgressDialog.h"
#include "svapp/framework/Document.h"
#include "svapp/audioio/AudioCallbackPlaySource.h"
#include "svapp/audioio/AudioCallbackPlayTarget.h"
#include "svapp/framework/Document.h"
#include "svapp/framework/VisualiserWindowBase.h"

#include "pngui/widgets/CorpusItemSelectorWidget.h"
#include "pngui/model/visualiser/ProsogramModel.h"

#include "pnlib/diff/diffintervals.h"
#include "pngui/model/diff/DiffSESForIntervalsTableModel.h"
#include "pngui/widgets/GridViewWidget.h"

#include "pnlib/asr/sphinx/SphinxSegmentation.h"
using namespace Praaline::ASR;

#include "LongSoundAlignerWidget.h"
#include "ui_LongSoundAlignerWidget.h"

struct LongSoundAlignerWidgetData {
    LongSoundAlignerWidgetData() :
        document(0), visualiserScroll(0), paneStack(0), viewManager(0), timeRulerLayer(0),
        playSource(0), playTarget(0),
        gridviewResults(0), modelResults(0)
    {}

    QPointer<Corpus> corpus;
    QPointer<CorpusCommunication> communication;
    QPointer<CorpusRecording> recording;
    QPointer<CorpusAnnotation> annotation;

    // Visualiser UI elements
    Document    *document;
    QScrollArea *visualiserScroll;
    PaneStack   *paneStack;
    ViewManager *viewManager;
    Layer       *timeRulerLayer;
    // Visualiser sound interface
    bool audioOutput;
    AudioCallbackPlaySource *playSource;
    AudioCallbackPlayTarget *playTarget;
    // Diff UI elements
    GridViewWidget *gridviewResults;
    dtl::Ses<Interval *>::sesElemVec sesSequence;
    DiffSESforIntervalsTableModel *modelResults;
};

LongSoundAlignerWidget::LongSoundAlignerWidget(QWidget *parent) :
    ASRModuleWidgetBase(parent),
    ui(new Ui::LongSoundAlignerWidget), d(new LongSoundAlignerWidgetData)
{
    ui->setupUi(this);

    // Visualiser
    d->document = new Document();
    d->viewManager = new ViewManager();
    d->paneStack = new PaneStack(0, d->viewManager);
    d->paneStack->setLayoutStyle(PaneStack::NoPropertyStacks);

    d->playSource = new AudioCallbackPlaySource(d->viewManager, QApplication::applicationName());

    d->visualiserScroll = new QScrollArea(this);
    d->visualiserScroll->setWidgetResizable(true);
    d->visualiserScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    d->visualiserScroll->setFrameShape(QFrame::NoFrame);
    d->visualiserScroll->setWidget(d->paneStack);

    ui->gridLayoutVisualiser->addWidget(d->visualiserScroll);

    // Diff grid
    d->gridviewResults = new GridViewWidget(this);
    d->gridviewResults->tableView()->verticalHeader()->setDefaultSectionSize(20);
    ui->gridLayoutDiff->addWidget(d->gridviewResults);

    connect(ui->commandCompare, SIGNAL(clicked(bool)), this, SLOT(diffTranscriptionWithRecogniser()));
}

LongSoundAlignerWidget::~LongSoundAlignerWidget()
{
    delete ui;
    if (d->playTarget) d->playTarget->shutdown();
    delete d->playSource;
    delete d->viewManager;
    delete d;
}

VisualiserWindowBase::FileOpenStatus
LongSoundAlignerWidget::openPath(const QString &fileOrUrl)
{
    ProgressDialog dialog(tr("Opening file or URL..."), true, 2000, this);

    FileSource source = FileSource(fileOrUrl, &dialog);

    if (!source.isAvailable()) {
        if (source.wasCancelled()) {
            return VisualiserWindowBase::FileOpenCancelled;
        } else {
            return VisualiserWindowBase::FileOpenFailed;
        }
    }
    source.waitForData();

    sv_samplerate_t rate = 0;

    if (Preferences::getInstance()->getFixedSampleRate() != 0) {
        rate = Preferences::getInstance()->getFixedSampleRate();
    } else if (Preferences::getInstance()->getResampleOnLoad()) {
        rate = d->playSource->getSourceSampleRate();
    }

    WaveFileModel *newModel = new WaveFileModel(source, rate);

    if (!newModel->isOK()) {
        delete newModel;
        if (source.wasCancelled()) {
            return VisualiserWindowBase::FileOpenCancelled;
        } else {
            return VisualiserWindowBase::FileOpenFailed;
        }
    }

    Model *prevMain = (d->document) ? d->document->getMainModel() : 0;
    if (prevMain) {
        d->playSource->removeModel(prevMain);
        PlayParameterRepository::getInstance()->removePlayable(prevMain);
    }
    PlayParameterRepository::getInstance()->addPlayable(newModel);

    d->document->setMainModel(newModel);

    return VisualiserWindowBase::FileOpenSucceeded;
}

void LongSoundAlignerWidget::open(Corpus *corpus, CorpusCommunication *com, CorpusRecording *rec, CorpusAnnotation *annot)
{
    if (!corpus) return;
    if (!com) return;
    if ((d->corpus == corpus) && (d->communication == com) && (d->recording == rec) && (d->annotation == annot)) return;

    d->corpus = corpus;
    d->communication = com;
    d->recording = rec;
    d->annotation = annot;

    close();

    // Open each recording in the communication and add a corresponding audio panel
    bool first = false;
    foreach (QPointer<CorpusRecording> rec, com->recordings()) {
        if (!first) {
            // Open
            ProgressDialog dialog(tr("Opening file or URL..."), true, 2000, this);
            connect(&dialog, SIGNAL(showing()), this, SIGNAL(hideSplash()));

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

    Pane *pane = d->paneStack->addPane();
    if (!d->timeRulerLayer) {
        d->timeRulerLayer = d->document->createMainModelLayer(LayerFactory::Type("TimeRuler"));
    }
    d->document->addLayerToView(pane, d->timeRulerLayer);
    Layer *waveform = d->document->createMainModelLayer(LayerFactory::Type("Waveform"));
    d->document->addLayerToView(pane, waveform);

    ProsogramModel *model = new ProsogramModel(d->document->getMainModel()->getSampleRate(), rec);
    pane = d->paneStack->addPane();
    Layer *newLayer = d->document->createImportedLayer(model);
    newLayer->setProperty("Show Vertical Lines", 0);
    d->document->addLayerToView(pane, newLayer);
    d->paneStack->setCurrentPane(pane);
    d->paneStack->setCurrentLayer(pane, newLayer);
}

void LongSoundAlignerWidget::close()
{
    while (d->paneStack->getPaneCount() > 0) {
        Pane *pane = d->paneStack->getPane(d->paneStack->getPaneCount() - 1);
        while (pane->getLayerCount() > 0) {
            d->document->removeLayerFromView(pane, pane->getLayer(pane->getLayerCount() - 1));
        }
        d->paneStack->deletePane(pane);
    }
    while (d->paneStack->getHiddenPaneCount() > 0) {
        Pane *pane = d->paneStack->getHiddenPane(d->paneStack->getHiddenPaneCount() - 1);
        while (pane->getLayerCount() > 0) {
            d->document->removeLayerFromView(pane, pane->getLayer(pane->getLayerCount() - 1));
        }
        d->paneStack->deletePane(pane);
    }
    delete d->document;
    d->viewManager->clearSelections();
    d->timeRulerLayer = 0; // document owned this
    d->document = new Document();
}

void LongSoundAlignerWidget::diffTranscriptionWithRecogniser()
{
    if (!d->annotation) return;

    QString path = "/home/george/test_lsa/";
    QList<Interval *> tokens_recognised = SphinxSegmentation::readContinuousFile(path + "output.txt");

    QString s;
    foreach (Interval *intv, tokens_recognised) {
        s = s.append(QString("%1\t%2\t%3\n")
                     .arg(intv->tMin().toDouble()).arg(intv->tMax().toDouble()).arg(intv->text()));
    }
    ui->textMessages->clear();
    ui->textMessages->appendPlainText(s);

    QList<Interval *> tokens_transcript = d->annotation->repository()->annotations()->getIntervals(
                AnnotationDatastore::Selection(d->annotation->ID(), "", "tok_min"));

    d->sesSequence = DiffIntervals::intervalDiff(
                tokens_transcript, tokens_recognised, false).getSes().getSequence();

    DiffSESforIntervalsTableModel *model = new DiffSESforIntervalsTableModel(
                d->sesSequence, tokens_transcript, tokens_recognised,
                "", "", QStringList() << "anchor", QStringList(), this);
    int anchorContinuousMatches(4), anchorSkipMismatches(2);
    bool inAnchor(false);
    int i = 0;
    while (i < model->rowCount()) {
        QString op = model->data(model->index(i, 0), Qt::DisplayRole).toString();
        if (inAnchor) {


        }
        else {
            bool check(true);
            for (int j = i; (j < i + anchorContinuousMatches) && (j < model->rowCount()); ++j) {
                check = check && (model->data(model->index(j, 0), Qt::DisplayRole).toString() == "=");
            }
            if (check) {
                inAnchor = true;
                for (int j = i; (j < i + anchorContinuousMatches) && (j < model->rowCount()); ++j) {

                }
            }


        }



        ++i;
    }

    d->gridviewResults->tableView()->setModel(model);
    if (d->modelResults) delete d->modelResults;
    d->modelResults = model;
}
