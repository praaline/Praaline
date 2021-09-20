#include <QString>
#include <QAction>

#include <memory>
using namespace std;

#include "AnnotationModeWidget.h"
#include "ui_AnnotationModeWidget.h"

#include "AutomaticAnnotationWidget.h"
#include "ManualAnnotationWidget.h"
#include "TranscriberWidget.h"
#include "SpeechRecognitionWidget.h"
#include "AnnotationBrowserWidget.h"
#include "BatchEditorWidget.h"
#include "CompareAnnotationsWidget.h"

#include "importannotations/ImportAnnotationsWizard.h"
#include "exportannotations/ExportAnnotationsWizard.h"

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
    QAction *actionShowSpeechRecognition;
    QAction *actionShowAnnotationBrowser;
    QAction *actionShowBatchEditor;
    QAction *actionShowCompareAnnotations;

    QAction *actionImportAnnotations;
    QAction *actionExportAnnotations;

    QAction *actionAddCalculatedAnnotation;
    QAction *actionCreateSequenceAnnotation;
    QAction *actionComposeTranscription;
    QAction *actionTidyUpAnnotations;

    AutomaticAnnotationWidget *widgetAutomaticAnnotation;
    ManualAnnotationWidget *widgetManualAnnotation;
    TranscriberWidget *widgetTranscriber;
    SpeechRecognitionWidget *widgetSpeechRecognition;
    AnnotationBrowserWidget *widgetAnnotationBrowser;
    BatchEditorWidget *widgetBatchEditor;
    CompareAnnotationsWidget *widgetCompareAnnotations;

    const int indexAutomaticAnnotation = 0;
    const int indexManualAnnotation = 1;
    const int indexTranscriber = 2;
    const int indexSpeechRecognition = 3;
    const int indexAnnotationBrowser = 4;
    const int indexBatchEditor = 5;
    const int indexCompareAnnotations = 6;
};


AnnotationModeWidget::AnnotationModeWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AnnotationModeWidget), d(new AnnotationModeWidgetData)
{
    ui->setupUi(this);

    d->widgetAutomaticAnnotation = new AutomaticAnnotationWidget(this);
    d->widgetManualAnnotation = new ManualAnnotationWidget(this);
    d->widgetTranscriber = new TranscriberWidget(this);
    d->widgetSpeechRecognition = new SpeechRecognitionWidget(this);
    d->widgetAnnotationBrowser = new AnnotationBrowserWidget(this);
    d->widgetBatchEditor = new BatchEditorWidget(this);
    d->widgetCompareAnnotations = new CompareAnnotationsWidget(this);

    ui->gridLayoutAutomaticAnnotation->addWidget(d->widgetAutomaticAnnotation);
    ui->gridLayoutManualAnnotation->addWidget(d->widgetManualAnnotation);
    ui->gridLayoutTranscriber->addWidget(d->widgetTranscriber);
    ui->gridLayoutSpeechRecognition->addWidget(d->widgetSpeechRecognition);
    ui->gridLayoutAnnotationBrowser->addWidget(d->widgetAnnotationBrowser);
    ui->gridLayoutBatchEditor->addWidget(d->widgetBatchEditor);
    ui->gridLayoutCompareAnnotations->addWidget(d->widgetCompareAnnotations);

    setupActions();

    connect(ui->commandAutomaticAnnotation, &QAbstractButton::clicked, this, &AnnotationModeWidget::showAutomaticAnnotation);
    connect(ui->commandManualAnnotation, &QAbstractButton::clicked, this, &AnnotationModeWidget::showManualAnnotation);
    connect(ui->commandTranscriber, &QAbstractButton::clicked, this, &AnnotationModeWidget::showTranscriber);
    connect(ui->commandSpeechRecognition, &QAbstractButton::clicked, this, &AnnotationModeWidget::showSpeechRecognition);
    connect(ui->commandAnnotationBrowser, &QAbstractButton::clicked, this, &AnnotationModeWidget::showAnnotationBrowser);
    connect(ui->commandBatchEditor, &QAbstractButton::clicked, this, &AnnotationModeWidget::showBatchEditor);
    connect(ui->commandCompareAnnotations, &QAbstractButton::clicked, this, &AnnotationModeWidget::showCompareAnnotations);

    ui->stackedWidget->setCurrentIndex(0);
}

AnnotationModeWidget::~AnnotationModeWidget()
{
    delete ui;
    if (d->widgetAutomaticAnnotation) delete d->widgetAutomaticAnnotation;
    if (d->widgetManualAnnotation) delete d->widgetManualAnnotation;
    if (d->widgetTranscriber) delete d->widgetTranscriber;
    if (d->widgetSpeechRecognition) delete d->widgetSpeechRecognition;
    if (d->widgetAnnotationBrowser) delete d->widgetAnnotationBrowser;
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

    d->actionImportAnnotations = new QAction(tr("Import annotations..."), this);
    connect(d->actionImportAnnotations, &QAction::triggered, this, &AnnotationModeWidget::wizardImportAnnotations);
    command = ACTION_MANAGER->registerAction("Annotation.ImportAnnotations", d->actionImportAnnotations, context);
    command->setCategory(QtilitiesCategory(tr("Annotation")));
    menu_annotation->addAction(command);

    d->actionExportAnnotations = new QAction(tr("Export annotations..."), this);
    connect(d->actionExportAnnotations, &QAction::triggered, this, &AnnotationModeWidget::wizardExportAnnotations);
    command = ACTION_MANAGER->registerAction("Annotation.ExportAnnotations", d->actionExportAnnotations, context);
    command->setCategory(QtilitiesCategory(tr("Annotation")));
    menu_annotation->addAction(command);

    d->actionAddCalculatedAnnotation = new QAction(tr("Add Annotation from Calculations..."), this);
    connect(d->actionAddCalculatedAnnotation, &QAction::triggered, this, &AnnotationModeWidget::dialogAddCalculatedAnnotation);
    command = ACTION_MANAGER->registerAction("Annotation.AddCalculatedAnnotation", d->actionAddCalculatedAnnotation, context);
    command->setCategory(QtilitiesCategory(tr("Annotation")));
    menu_annotation->addAction(command);

    d->actionCreateSequenceAnnotation = new QAction(tr("Create Sequences from Existing Annotations..."), this);
    connect(d->actionCreateSequenceAnnotation, &QAction::triggered, this, &AnnotationModeWidget::dialogCreateSequenceAnnotation);
    command = ACTION_MANAGER->registerAction("Annotation.CreateSequenceAnnotation", d->actionCreateSequenceAnnotation, context);
    command->setCategory(QtilitiesCategory(tr("Annotation")));
    menu_annotation->addAction(command);

    d->actionComposeTranscription = new QAction(tr("Compose Transcription..."), this);
    connect(d->actionComposeTranscription, &QAction::triggered, this, &AnnotationModeWidget::dialogComposeTranscription);
    command = ACTION_MANAGER->registerAction("Annotation.ComposeTranscription", d->actionComposeTranscription, context);
    command->setCategory(QtilitiesCategory(tr("Annotation")));
    menu_annotation->addAction(command);

    d->actionTidyUpAnnotations = new QAction(tr("Tidy-up annotations..."), this);
    connect(d->actionTidyUpAnnotations, &QAction::triggered, this, &AnnotationModeWidget::dialogTidyUpAnnotations);
    command = ACTION_MANAGER->registerAction("Annotation.TidyUpAnnotations", d->actionTidyUpAnnotations, context);
    command->setCategory(QtilitiesCategory(tr("Annotation")));
    menu_annotation->addAction(command);

    // ------------------------------------------------------------------------------------------------------
    // WINDOW MENU
    // ------------------------------------------------------------------------------------------------------
    d->actionShowAutomaticAnnotation = new QAction(tr("Automatic Annotation"), this);
    connect(d->actionShowAutomaticAnnotation, &QAction::triggered, this, &AnnotationModeWidget::showAutomaticAnnotation);
    command = ACTION_MANAGER->registerAction("Annotation.ShowAutomatic", d->actionShowAutomaticAnnotation, context);
    command->setCategory(QtilitiesCategory(tr("Active Window Selection")));
    menu_window->addAction(command);

    d->actionShowManualAnnotation = new QAction(tr("Manual Annotation"), this);
    connect(d->actionShowManualAnnotation, &QAction::triggered, this, &AnnotationModeWidget::showManualAnnotation);
    command = ACTION_MANAGER->registerAction("Annotation.ShowManual", d->actionShowManualAnnotation, context);
    command->setCategory(QtilitiesCategory(tr("Active Window Selection")));
    menu_window->addAction(command);

    d->actionShowTranscriber = new QAction(tr("Transcriber"), this);
    connect(d->actionShowTranscriber, &QAction::triggered, this, &AnnotationModeWidget::showTranscriber);
    command = ACTION_MANAGER->registerAction("Annotation.ShowTranscriber", d->actionShowTranscriber, context);
    command->setCategory(QtilitiesCategory(tr("Active Window Selection")));
    menu_window->addAction(command);

    d->actionShowSpeechRecognition = new QAction(tr("Speech Recognition && Alignment"), this);
    connect(d->actionShowSpeechRecognition, &QAction::triggered, this, &AnnotationModeWidget::showSpeechRecognition);
    command = ACTION_MANAGER->registerAction("Annotation.ShowSpeechRecognition", d->actionShowSpeechRecognition, context);
    command->setCategory(QtilitiesCategory(tr("Active Window Selection")));
    menu_window->addAction(command);

    d->actionShowAnnotationBrowser = new QAction(tr("Annotation Table Browser"), this);
    connect(d->actionShowAnnotationBrowser, &QAction::triggered, this, &AnnotationModeWidget::showAnnotationBrowser);
    command = ACTION_MANAGER->registerAction("Corpus.ShowAnnotationBrowser", d->actionShowAnnotationBrowser, context);
    command->setCategory(QtilitiesCategory(tr("Active Window Selection")));
    menu_window->addAction(command);

    d->actionShowBatchEditor = new QAction(tr("Batch Annotation Editor"), this);
    connect(d->actionShowBatchEditor, &QAction::triggered, this, &AnnotationModeWidget::showBatchEditor);
    command = ACTION_MANAGER->registerAction("Annotation.ShowBatch", d->actionShowBatchEditor, context);
    command->setCategory(QtilitiesCategory(tr("Active Window Selection")));
    menu_window->addAction(command);

    d->actionShowCompareAnnotations = new QAction(tr("Compare Annotations"), this);
    connect(d->actionShowCompareAnnotations, &QAction::triggered, this, &AnnotationModeWidget::showCompareAnnotations);
    command = ACTION_MANAGER->registerAction("Annotation.ShowCompareAnnotations", d->actionShowCompareAnnotations, context);
    command->setCategory(QtilitiesCategory(tr("Active Window Selection")));
    menu_window->addAction(command);

    menu_window->addSeparator();
}

void AnnotationModeWidget::showAutomaticAnnotation()
{
    ui->stackedWidget->setCurrentIndex(d->indexAutomaticAnnotation);
    emit activateMode();
}

void AnnotationModeWidget::showManualAnnotation()
{
    ui->stackedWidget->setCurrentIndex(d->indexManualAnnotation);
    emit activateMode();
}

void AnnotationModeWidget::showTranscriber()
{
    ui->stackedWidget->setCurrentIndex(d->indexTranscriber);
    emit activateMode();
}

void AnnotationModeWidget::showSpeechRecognition()
{
    ui->stackedWidget->setCurrentIndex(d->indexSpeechRecognition);
    emit activateMode();
}

void AnnotationModeWidget::showAnnotationBrowser()
{
    ui->stackedWidget->setCurrentIndex(d->indexAnnotationBrowser);
    emit activateMode();
}

void AnnotationModeWidget::showBatchEditor()
{
    ui->stackedWidget->setCurrentIndex(d->indexBatchEditor);
    emit activateMode();
}

void AnnotationModeWidget::showCompareAnnotations()
{
    ui->stackedWidget->setCurrentIndex(d->indexCompareAnnotations);
    emit activateMode();
}

void AnnotationModeWidget::wizardImportAnnotations()
{
    unique_ptr<ImportAnnotationsWizard> wizard { new ImportAnnotationsWizard() };
    wizard->exec();
}

void AnnotationModeWidget::wizardExportAnnotations()
{
    unique_ptr<ExportAnnotationsWizard> wizard { new ExportAnnotationsWizard() };
    wizard->exec();
}

void AnnotationModeWidget::dialogAddCalculatedAnnotation()
{
    unique_ptr<AddCalculatedAnnotationDialog> dialog { new AddCalculatedAnnotationDialog() };
    dialog->exec();
}

void AnnotationModeWidget::dialogCreateSequenceAnnotation()
{
    unique_ptr<CreateSequenceAnnotationDialog> dialog { new CreateSequenceAnnotationDialog() };
    dialog->exec();
}

void AnnotationModeWidget::dialogComposeTranscription()
{
    unique_ptr<ComposeTranscriptionDialog> dialog { new ComposeTranscriptionDialog() };
    dialog->exec();
}

void AnnotationModeWidget::dialogTidyUpAnnotations()
{
    unique_ptr<TidyUpAnnotationsDialog> dialog { new TidyUpAnnotationsDialog() };
    dialog->exec();
}
