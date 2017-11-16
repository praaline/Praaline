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

#include "svcore/data/model/WaveFileModel.h"
#include "svgui/view/Pane.h"
#include "svgui/view/PaneStack.h"
#include "svgui/view/Overview.h"
#include "svgui/layer/WaveformLayer.h"
#include "svgui/layer/TimeRulerLayer.h"
#include "svgui/widgets/AudioDial.h"
#include "svgui/widgets/Fader.h"
#include "svgui/widgets/CommandHistory.h"

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
        gridviewResults(0), modelResults(0)
    {}

    QPointer<Corpus> corpus;
    QPointer<CorpusCommunication> communication;
    QPointer<CorpusRecording> recording;
    QPointer<CorpusAnnotation> annotation;

    // Diff UI elements
    GridViewWidget *gridviewResults;
    dtl::Ses<Interval *>::sesElemVec sesSequence;
    DiffSESforIntervalsTableModel *modelResults;
};

LongSoundAlignerWidget::LongSoundAlignerWidget(QWidget *parent) :
    ASRModuleVisualiserWidgetBase(parent),
    ui(new Ui::LongSoundAlignerWidget), d(new LongSoundAlignerWidgetData)
{
    ui->setupUi(this);

    // Visualiser
    ui->gridLayoutVisualiser->addWidget(m_visualiserScroll);

    // Diff grid
    d->gridviewResults = new GridViewWidget(this);
    d->gridviewResults->tableView()->verticalHeader()->setDefaultSectionSize(20);
    ui->gridLayoutDiff->addWidget(d->gridviewResults);

    connect(ui->commandCompare, SIGNAL(clicked(bool)), this, SLOT(diffTranscriptionWithRecogniser()));
}

LongSoundAlignerWidget::~LongSoundAlignerWidget()
{
    delete ui;
    delete d;
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

    Pane *pane = m_paneStack->addPane();
    if (!m_timeRulerLayer) {
        m_timeRulerLayer = m_document->createMainModelLayer(LayerFactory::Type("TimeRuler"));
    }
    m_document->addLayerToView(pane, m_timeRulerLayer);
    Layer *waveform = m_document->createMainModelLayer(LayerFactory::Type("Waveform"));
    m_document->addLayerToView(pane, waveform);

    ProsogramModel *model = new ProsogramModel(m_document->getMainModel()->getSampleRate(), rec);
    pane = m_paneStack->addPane();
    Layer *newLayer = m_document->createImportedLayer(model);
    newLayer->setProperty("Show Vertical Lines", 0);
    m_document->addLayerToView(pane, newLayer);
    m_paneStack->setCurrentPane(pane);
    m_paneStack->setCurrentLayer(pane, newLayer);
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
