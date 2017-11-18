#include <QFrame>
#include <QWidget>
#include <QScrollArea>
#include <QApplication>

#include "svcore/base/Preferences.h"
#include "svcore/base/PlayParameterRepository.h"
#include "svcore/data/model/WaveFileModel.h"
#include "svgui/view/Pane.h"
#include "svgui/view/PaneStack.h"
#include "svgui/view/ViewManager.h"
#include "svgui/layer/Layer.h"
#include "svgui/widgets/ProgressDialog.h"
#include "svapp/framework/Document.h"
#include "svapp/audioio/AudioCallbackPlaySource.h"
#include "svapp/audioio/AudioCallbackPlayTarget.h"
#include "svapp/framework/Document.h"
#include "svapp/framework/VisualiserWindowBase.h"

#include "ASRModuleVisualiserWidgetBase.h"

ASRModuleVisualiserWidgetBase::ASRModuleVisualiserWidgetBase(QWidget *parent) :
    ASRModuleWidgetBase(parent),
    m_visualiserScroll(0), m_document(0), m_paneStack(0), m_viewManager(0), m_timeRulerLayer(0),
    m_audioOutput(false), m_playSource(0), m_playTarget(0)
{
    m_document = new Document();
    m_viewManager = new ViewManager();
    m_paneStack = new PaneStack(0, m_viewManager);
    m_paneStack->setLayoutStyle(PaneStack::NoPropertyStacks);

    m_playSource = new AudioCallbackPlaySource(m_viewManager, QApplication::applicationName());

    m_visualiserScroll = new QScrollArea(this);
    m_visualiserScroll->setWidgetResizable(true);
    m_visualiserScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_visualiserScroll->setFrameShape(QFrame::NoFrame);
    m_visualiserScroll->setWidget(m_paneStack);

}

ASRModuleVisualiserWidgetBase::~ASRModuleVisualiserWidgetBase()
{
    if (m_playTarget) m_playTarget->shutdown();
    delete m_playSource;
    delete m_viewManager;
}

VisualiserWindowBase::FileOpenStatus ASRModuleVisualiserWidgetBase::openPath(const QString &fileOrUrl)
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
        rate = m_playSource->getSourceSampleRate();
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
    Model *prevMain = (m_document) ? m_document->getMainModel() : 0;
    if (prevMain) {
        m_playSource->removeModel(prevMain);
        PlayParameterRepository::getInstance()->removePlayable(prevMain);
    }
    PlayParameterRepository::getInstance()->addPlayable(newModel);
    m_document->setMainModel(newModel);
    return VisualiserWindowBase::FileOpenSucceeded;
}

void ASRModuleVisualiserWidgetBase::close()
{
    while (m_paneStack->getPaneCount() > 0) {
        Pane *pane = m_paneStack->getPane(m_paneStack->getPaneCount() - 1);
        while (pane->getLayerCount() > 0) {
            m_document->removeLayerFromView(pane, pane->getLayer(pane->getLayerCount() - 1));
        }
        m_paneStack->deletePane(pane);
    }
    while (m_paneStack->getHiddenPaneCount() > 0) {
        Pane *pane = m_paneStack->getHiddenPane(m_paneStack->getHiddenPaneCount() - 1);
        while (pane->getLayerCount() > 0) {
            m_document->removeLayerFromView(pane, pane->getLayer(pane->getLayerCount() - 1));
        }
        m_paneStack->deletePane(pane);
    }
    delete m_document;
    m_viewManager->clearSelections();
    m_timeRulerLayer = 0; // document owned this
    m_document = new Document();
}

WaveFileModel * ASRModuleVisualiserWidgetBase::getMainModel()
{
    if (!m_document) return 0;
    return m_document->getMainModel();
}

const WaveFileModel * ASRModuleVisualiserWidgetBase::getMainModel() const
{
    if (!m_document) return 0;
    return m_document->getMainModel();
}
