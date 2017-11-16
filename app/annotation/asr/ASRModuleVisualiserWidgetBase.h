#ifndef ASRMODULEVISUALISERWIDGETBASE_H
#define ASRMODULEVISUALISERWIDGETBASE_H

#include "ASRModuleWidgetBase.h"

#include "pncore/corpus/Corpus.h"
using namespace Praaline::Core;

#include <QScrollArea>
#include "svgui/view/PaneStack.h"
#include "svgui/view/ViewManager.h"
#include "svgui/layer/Layer.h"
#include "svapp/framework/Document.h"
#include "svapp/audioio/AudioCallbackPlaySource.h"
#include "svapp/audioio/AudioCallbackPlayTarget.h"
#include "svapp/framework/Document.h"
#include "svapp/framework/VisualiserWindowBase.h"

class ASRModuleVisualiserWidgetBase : public ASRModuleWidgetBase
{
    Q_OBJECT
public:
    explicit ASRModuleVisualiserWidgetBase(QWidget *parent = nullptr);
    virtual ~ASRModuleVisualiserWidgetBase();

protected:
    VisualiserWindowBase::FileOpenStatus openPath(const QString &fileOrUrl);
    void close();

    // Visualiser UI elements
    QScrollArea *m_visualiserScroll;
    Document    *m_document;
    PaneStack   *m_paneStack;
    ViewManager *m_viewManager;
    Layer       *m_timeRulerLayer;
    // Visualiser sound interface
    bool m_audioOutput;
    AudioCallbackPlaySource *m_playSource;
    AudioCallbackPlayTarget *m_playTarget;

signals:

public slots:
};

#endif // ASRMODULEVISUALISERWIDGETBASE_H
