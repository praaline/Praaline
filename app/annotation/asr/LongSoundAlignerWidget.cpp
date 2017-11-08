#include <QObject>
#include <QPointer>
#include <QGridLayout>
#include <QScrollArea>
#include <QFrame>
#include <QMenu>

#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/structure/AnnotationStructure.h"
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
#include "svapp/framework/Document.h"
#include "svapp/audioio/AudioCallbackPlaySource.h"
#include "svapp/audioio/AudioCallbackPlayTarget.h"

#include "pngui/widgets/CorpusItemSelectorWidget.h"

#include "LongSoundAlignerWidget.h"
#include "ui_LongSoundAlignerWidget.h"

struct LongSoundAlignerWidgetData {
    LongSoundAlignerWidgetData() :
        paneStack(0), viewManager(0), timeRulerLayer(0)
    {}

    QPointer<Corpus> corpus;
    QPointer<CorpusCommunication> communication;
    QPointer<CorpusRecording> recording;
    QPointer<CorpusAnnotation> annotation;

    PaneStack *paneStack;
    ViewManager *viewManager;
    Layer *timeRulerLayer;

    bool m_audioOutput;
    AudioCallbackPlaySource *playSource;
    AudioCallbackPlayTarget *playTarget;
};

LongSoundAlignerWidget::LongSoundAlignerWidget(QWidget *parent) :
    ASRModuleWidgetBase(parent),
    ui(new Ui::LongSoundAlignerWidget), d(new LongSoundAlignerWidgetData)
{
    ui->setupUi(this);

    d->viewManager = new ViewManager();
    d->paneStack = new PaneStack(0, d->viewManager);

}

LongSoundAlignerWidget::~LongSoundAlignerWidget()
{
    delete ui;
    if (d->playTarget) d->playTarget->shutdown();
    delete d->playSource;
    delete d->viewManager;
    delete d;
}

void LongSoundAlignerWidget::open(Corpus *corpus, CorpusCommunication *com, CorpusRecording *rec, CorpusAnnotation *annot)
{
    if (!corpus) return;
    if ((d->corpus == corpus) && (d->communication == com) && (d->recording == rec) && (d->annotation == annot)) return;

    d->corpus = corpus;
    d->communication = com;
    d->recording = rec;
    d->annotation = annot;

    Pane *pane = d->paneStack->addPane();
    if (!d->timeRulerLayer) {
        d->timeRulerLayer = m_document->createMainModelLayer(LayerFactory::Type("TimeRuler"));
    }
    m_document->addLayerToView(pane, m_timeRulerLayer);
    Layer *waveform = m_document->createMainModelLayer(LayerFactory::Type("Waveform"));
    m_document->addLayerToView(pane, waveform);
    m_overview->registerView(pane);
}

