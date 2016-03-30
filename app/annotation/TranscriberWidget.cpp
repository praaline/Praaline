#include <QObject>
#include <QString>
#include <QLabel>
#include <QStatusBar>
#include <QGridLayout>
#include <QScrollArea>
#include <QFrame>

#include "svgui/view/Overview.h"
#include "svgui/view/PaneStack.h"
#include "svgui/widgets/AudioDial.h"
#include "svgui/widgets/Fader.h"

#include "../visualisation/SimpleVisualiserWidget.h"
#include "pngui/widgets/annotationtimelineeditor.h"
#include "TranscriberWidget.h"
#include "ui_transcriberwidget.h"

struct TranscriberWidgetData {
    TranscriberWidgetData() :
    annotationEditor(0)
    {}

    AnnotationTimelineEditor *annotationEditor;     // Annotation timeline editor
};

TranscriberWidget::TranscriberWidget(QWidget *parent) :
    SimpleVisualiserWidget("Transcriber", true, true),
    ui(new Ui::TranscriberWidget), d(new TranscriberWidgetData)
{
    Q_UNUSED(parent)
    ui->setupUi(this);

    setCentralWidget(m_visualiserFrame);

    QGridLayout *layout = new QGridLayout;

    QFrame *playControlsSpacer = new QFrame;

    layout->setSpacing(4);
    layout->addWidget(m_mainScroll, 0, 0, 1, 5);
    layout->addWidget(m_overview, 1, 1);
    layout->addWidget(playControlsSpacer, 1, 2);
    layout->addWidget(m_playSpeed, 1, 3);
    layout->addWidget(m_fader, 1, 4);

    int playControlsWidth = m_fader->width() + m_playSpeed->width() + layout->spacing() * 2;

    layout->setColumnMinimumWidth(0, 14);
    layout->setColumnStretch(0, 0);

    m_paneStack->setPropertyStackMinWidth(playControlsWidth + 2 + layout->spacing());
    playControlsSpacer->setFixedSize(QSize(2, 2));

    layout->setColumnStretch(1, 10);

    //connect(m_paneStack, SIGNAL(propertyStacksResized(int)), this, SLOT(propertyStacksResized(int)));

    m_visualiserFrame->setLayout(layout);

    setupMenus();
    //setupToolbars();

    statusBar();
    m_currentLabel = new QLabel;
    statusBar()->addPermanentWidget(m_currentLabel);

    finaliseMenus();

    d->annotationEditor = new AnnotationTimelineEditor(this);
    layout->addWidget(d->annotationEditor);
}

TranscriberWidget::~TranscriberWidget()
{
    delete ui;
    delete d;
}

