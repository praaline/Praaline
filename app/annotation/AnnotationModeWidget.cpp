#include "AnnotationModeWidget.h"
#include "ui_AnnotationModeWidget.h"

#include "AutomaticAnnotationWidget.h"
#include "ManualAnnotationWidget.h"
#include "BatchEditorWidget.h"
#include "CompareAnnotationsWidget.h"
#include "TranscriberWidget.h"

#include "calculate/AddCalculatedAnnotationDialog.h"
#include "calculate/CreateSequenceAnnotationDialog.h"
#include "calculate/ComposeTranscriptionDialog.h"
#include "calculate/TidyUpAnnotationsDialog.h"

struct AnnotationModeWidgetData {
    AnnotationModeWidgetData()
    { }

    QAction *actionShowAutomaticAnnotation;
    QAction *actionShowManualAnnotation;
    QAction *actionShowTranscriber;
    QAction *actionShowBatchEditor;
    QAction *actionShowCompareAnnotations;

    QAction *actionAddCalculatedAnnotation;
    QAction *actionCreateSequenceAnnotation;
    QAction *actionComposeTranscription;
    QAction *actionTidyUpAnnotations;

    AutomaticAnnotationWidget *widgetAutomaticAnnotation;
    ManualAnnotationWidget *widgetManualAnnotation;
    TranscriberWidget *widgetTranscriber;
    BatchEditorWidget *widgetBatchEditor;
    CompareAnnotationsWidget *widgetCompareAnnotations;
};


AnnotationModeWidget::AnnotationModeWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AnnotationModeWidget), d(new AnnotationModeWidgetData)
{
    ui->setupUi(this);

    d->widgetAutomaticAnnotation = new AutomaticAnnotationWidget(this);
    d->widgetManualAnnotation = new ManualAnnotationWidget(this);
    d->widgetTranscriber = new TranscriberWidget(this);
    d->widgetBatchEditor = new BatchEditorWidget(this);
    d->widgetCompareAnnotations = new CompareAnnotationsWidget(this);

    ui->gridLayoutAutomaticAnnotation->addWidget(d->widgetAutomaticAnnotation);
    ui->gridLayoutManualAnnotation->addWidget(d->widgetManualAnnotation);
    ui->gridLayoutTranscriber->addWidget(d->widgetTranscriber);
    ui->gridLayoutBatchEditor->addWidget(d->widgetBatchEditor);
    ui->gridLayoutCompareAnnotations->addWidget(d->widgetCompareAnnotations);

    setupActions();

    connect(ui->commandAutomatic, SIGNAL(clicked()), this, SLOT(showAutomaticAnnotation()));
    connect(ui->commandTranscriber, SIGNAL(clicked()), this, SLOT(showTranscriber()));
    connect(ui->commandTimelineEditor, SIGNAL(clicked()), this, SLOT(showManualAnnotation()));
    connect(ui->commandBatchEditor, SIGNAL(clicked()), this, SLOT(showBatchEditor()));
    connect(ui->commandCompareAnnotations, SIGNAL(clicked()), this, SLOT(showCompareAnnotations()));

    ui->stackedWidget->setCurrentIndex(1);
}

AnnotationModeWidget::~AnnotationModeWidget()
{
    delete ui;
    if (d->widgetAutomaticAnnotation) delete d->widgetAutomaticAnnotation;
    if (d->widgetTranscriber) delete d->widgetTranscriber;
    if (d->widgetManualAnnotation) delete d->widgetManualAnnotation;
    if (d->widgetBatchEditor) delete d->widgetBatchEditor;
    if (d->widgetCompareAnnotations) delete d->widgetCompareAnnotations;
    delete d;
}

void AnnotationModeWidget::setupActions()
{
    // Context and menu bar
    QList<int> context;
    context.push_front(CONTEXT_MANAGER->contextID(qti_def_CONTEXT_STANDARD));

    bool existed;
    Command* command;

    ActionContainer* menubar = ACTION_MANAGER->menuBar(qti_action_MENUBAR_STANDARD);
    ActionContainer* menu_annotation = ACTION_MANAGER->createMenu(tr("&Annotation"), existed);
    if (!existed) menubar->addMenu(menu_annotation, qti_action_HELP);
    ActionContainer* menu_window = ACTION_MANAGER->createMenu(tr("&Window"), existed);
    if (!existed) menubar->addMenu(menu_window, qti_action_HELP);

    // ------------------------------------------------------------------------------------------------------
    // ANNOTATION MENU
    // ------------------------------------------------------------------------------------------------------
    d->actionAddCalculatedAnnotation = new QAction(tr("Add Annotation from Calculations..."), this);
    connect(d->actionAddCalculatedAnnotation, SIGNAL(triggered()), SLOT(dialogAddCalculatedAnnotation()));
    command = ACTION_MANAGER->registerAction("Annotation.AddCalculatedAnnotation", d->actionAddCalculatedAnnotation, context);
    command->setCategory(QtilitiesCategory(tr("Annotation")));
    menu_annotation->addAction(command);

    d->actionCreateSequenceAnnotation = new QAction(tr("Create Sequences from Existing Annotations..."), this);
    connect(d->actionCreateSequenceAnnotation, SIGNAL(triggered()), SLOT(dialogCreateSequenceAnnotation()));
    command = ACTION_MANAGER->registerAction("Annotation.CreateSequenceAnnotation", d->actionCreateSequenceAnnotation, context);
    command->setCategory(QtilitiesCategory(tr("Annotation")));
    menu_annotation->addAction(command);

    d->actionComposeTranscription = new QAction(tr("Compose Transcription..."), this);
    connect(d->actionComposeTranscription, SIGNAL(triggered()), SLOT(dialogComposeTranscription()));
    command = ACTION_MANAGER->registerAction("Annotation.ComposeTranscription", d->actionComposeTranscription, context);
    command->setCategory(QtilitiesCategory(tr("Annotation")));
    menu_annotation->addAction(command);

    d->actionTidyUpAnnotations = new QAction(tr("Tidy-up annotations..."), this);
    connect(d->actionTidyUpAnnotations, SIGNAL(triggered()), SLOT(dialogTidyUpAnnotations()));
    command = ACTION_MANAGER->registerAction("Annotation.TidyUpAnnotations", d->actionTidyUpAnnotations, context);
    command->setCategory(QtilitiesCategory(tr("Annotation")));
    menu_annotation->addAction(command);

    // ------------------------------------------------------------------------------------------------------
    // WINDOW MENU
    // ------------------------------------------------------------------------------------------------------
    d->actionShowAutomaticAnnotation = new QAction(tr("Automatic Annotation"), this);
    connect(d->actionShowAutomaticAnnotation, SIGNAL(triggered()), SLOT(showAutomaticAnnotation()));
    command = ACTION_MANAGER->registerAction("Annotation.ShowAutomatic", d->actionShowAutomaticAnnotation, context);
    command->setCategory(QtilitiesCategory(tr("Active Window Selection")));
    menu_window->addAction(command);

    d->actionShowTranscriber = new QAction(tr("Transcriber"), this);
    connect(d->actionShowTranscriber, SIGNAL(triggered()), SLOT(showTranscriber()));
    command = ACTION_MANAGER->registerAction("Annotation.ShowTranscriber", d->actionShowTranscriber, context);
    command->setCategory(QtilitiesCategory(tr("Active Window Selection")));
    menu_window->addAction(command);

    d->actionShowManualAnnotation = new QAction(tr("Timeline Editor"), this);
    connect(d->actionShowManualAnnotation, SIGNAL(triggered()), SLOT(showManualAnnotation()));
    command = ACTION_MANAGER->registerAction("Annotation.ShowManual", d->actionShowManualAnnotation, context);
    command->setCategory(QtilitiesCategory(tr("Active Window Selection")));
    menu_window->addAction(command);

    d->actionShowBatchEditor = new QAction(tr("Batch Annotation Editor"), this);
    connect(d->actionShowBatchEditor, SIGNAL(triggered()), SLOT(showBatchEditor()));
    command = ACTION_MANAGER->registerAction("Annotation.ShowBatch", d->actionShowBatchEditor, context);
    command->setCategory(QtilitiesCategory(tr("Active Window Selection")));
    menu_window->addAction(command);

    d->actionShowCompareAnnotations = new QAction(tr("Compare Annotations"), this);
    connect(d->actionShowCompareAnnotations, SIGNAL(triggered()), SLOT(showCompareAnnotations()));
    command = ACTION_MANAGER->registerAction("Annotation.ShowCompareAnnotations", d->actionShowCompareAnnotations, context);
    command->setCategory(QtilitiesCategory(tr("Active Window Selection")));
    menu_window->addAction(command);

    menu_window->addSeparator();
}

void AnnotationModeWidget::showAutomaticAnnotation()
{
    ui->stackedWidget->setCurrentIndex(0);
    emit activateMode();
}

void AnnotationModeWidget::showTranscriber()
{
    ui->stackedWidget->setCurrentIndex(1);
    emit activateMode();
}

void AnnotationModeWidget::showManualAnnotation()
{
    ui->stackedWidget->setCurrentIndex(2);
    emit activateMode();
}

void AnnotationModeWidget::showBatchEditor()
{
    ui->stackedWidget->setCurrentIndex(3);
    emit activateMode();
}

void AnnotationModeWidget::showCompareAnnotations()
{
    ui->stackedWidget->setCurrentIndex(4);
    emit activateMode();
}

void AnnotationModeWidget::dialogAddCalculatedAnnotation()
{
    AddCalculatedAnnotationDialog *dialog = new AddCalculatedAnnotationDialog(this);
    dialog->exec();
    delete dialog;
}

void AnnotationModeWidget::dialogCreateSequenceAnnotation()
{
    CreateSequenceAnnotationDialog *dialog = new CreateSequenceAnnotationDialog(this);
    dialog->exec();
    delete dialog;
}

void AnnotationModeWidget::dialogComposeTranscription()
{
    ComposeTranscriptionDialog *dialog = new ComposeTranscriptionDialog(this);
    dialog->exec();
    delete dialog;
}

void AnnotationModeWidget::dialogTidyUpAnnotations()
{
    TidyUpAnnotationsDialog *dialog = new TidyUpAnnotationsDialog(this);
    dialog->exec();
    delete dialog;
}
