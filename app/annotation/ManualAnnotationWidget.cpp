#include <QPointer>
#include <QString>
#include <QList>
#include <QMessageBox>
#include <QMap>
#include <QDebug>
#include <QFileDialog>
#include <QStandardItemModel>
#include <QModelIndex>
#include "ManualAnnotationWidget.h"
#include "ui_ManualAnnotationWidget.h"

#include "pncore/corpus/Corpus.h"
#include "pncore/datastore/CorpusRepository.h"

#include "pngui/observers/CorpusObserver.h"
#include "pngui/model/CheckableProxyModel.h"
#include "pngui/model/corpus/CorpusBookmarkModel.h"
#include "pngui/widgets/CorpusItemSelectorWidget.h"

#include "editors/AnnotationEditorBase.h"
#include "editors/AnnotationMultiTierEditor.h"
#include "editors/GroupingAnnotationEditor.h"
#include "editors/TranscriptAnnotationEditor.h"

#include "PraalineUserInterfaceOptions.h"

struct ManualAnnotationWidgetData {
    ManualAnnotationWidgetData() :
        corpusItemSelector(0)
    { }

    // Corpus
    CorpusItemSelectorWidget *corpusItemSelector;
    // Current selection
    QPointer<Corpus> corpus;
    QPointer<CorpusCommunication> com;
    QPointer<CorpusRecording> rec;
    QPointer<CorpusAnnotation> annot;
    // Open editors
    QList<AnnotationEditorBase *> openEditors;
};

ManualAnnotationWidget::ManualAnnotationWidget(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ManualAnnotationWidget), d(new ManualAnnotationWidgetData)
{
    setParent(parent);
    ui->setupUi(this);

    // Annotation Editor Selector
    ui->comboBoxEditorSelection->addItem("Timeline Editor", "AnnotationMultiTierEditor");
    ui->comboBoxEditorSelection->addItem("Grouping Annotation Editor", "GroupingAnnotationEditor");
    ui->comboBoxEditorSelection->addItem("Transcrtipt Editor", "TranscriptAnnotationEditor");
    ui->comboBoxEditorSelection->setCurrentIndex(0);
    connect(ui->commandOpenEditor, SIGNAL(clicked(bool)), this, SLOT(editorTabNew()));

    // Corpus item selector
    d->corpusItemSelector = new CorpusItemSelectorWidget(this);
    ui->gridLayoutCorpusExplorer->addWidget(d->corpusItemSelector);
    connect(d->corpusItemSelector, SIGNAL(selectedCorpusCommunication(QPointer<Corpus>,QPointer<CorpusCommunication>)),
            this, SLOT(selectedCorpusCommunication(QPointer<Corpus>,QPointer<CorpusCommunication>)));
    connect(d->corpusItemSelector, SIGNAL(selectedCorpusRecording(QPointer<Corpus>,QPointer<CorpusCommunication>,QPointer<CorpusRecording>)),
            this, SLOT(selectedCorpusRecording(QPointer<Corpus>,QPointer<CorpusCommunication>,QPointer<CorpusRecording>)));
    connect(d->corpusItemSelector, SIGNAL(selectedCorpusAnnotation(QPointer<Corpus>,QPointer<CorpusCommunication>,QPointer<CorpusAnnotation>)),
            this, SLOT(selectedCorpusAnnotation(QPointer<Corpus>,QPointer<CorpusCommunication>,QPointer<CorpusAnnotation>)));
    connect(d->corpusItemSelector, SIGNAL(moveToAnnotationTime(QPointer<Corpus>,QPointer<CorpusCommunication>,QPointer<CorpusAnnotation>,RealTime)),
            this, SLOT(moveToAnnotationTime(QPointer<Corpus>,QPointer<CorpusCommunication>,QPointer<CorpusAnnotation>,RealTime)));

    // Left-right splitter sizes
    ui->splitterLR->setSizes(QList<int>() << 50 << 350);

    // Handle close document
    connect(ui->tabWidgetEditors, SIGNAL(tabCloseRequested(int)), this, SLOT(editorTabCloseRequested(int)));
}

ManualAnnotationWidget::~ManualAnnotationWidget()
{
    delete ui;
    delete d;
}

// ====================================================================================================================
// Corpus item selection (selector -> editor)
// ====================================================================================================================

void ManualAnnotationWidget::selectedCorpusCommunication(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com)
{
    QPointer<CorpusRecording> rec(0);
    QPointer<CorpusAnnotation> annot(0);
    if (!corpus) return;
    if (!com) return;
    if (com->recordingsCount() >= 1) rec = com->recordings().first();
    if (com->annotationsCount() >= 1) annot = com->annotations().first();
    selectionChanged(corpus, com, rec, annot);
}

void ManualAnnotationWidget::selectedCorpusRecording(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com,
                                                     QPointer<CorpusRecording> rec)
{
    QPointer<CorpusAnnotation> annot(0);
    if (!corpus) return;
    if (!com) return;
    if (!rec) return;
    if (com->annotationsCount() == 1) {
        annot = com->annotations().first();
    } else if (com->annotationsCount() > 0) {
        annot = com->annotation(rec->ID());
    }
    selectionChanged(corpus, com, rec, annot);
}

void ManualAnnotationWidget::selectedCorpusAnnotation(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com,
                                                      QPointer<CorpusAnnotation> annot)
{
    QPointer<CorpusRecording> rec(0);
    if (!corpus) return;
    if (!com) return;
    if (!annot) return;
    if (com->recordingsCount() == 1) {
        rec = com->recordings().first();
    } else if (com->annotationsCount() > 0) {
        rec = com->recording(annot->ID());
    }
    selectionChanged(corpus, com, rec, annot);
}

void ManualAnnotationWidget::selectionChanged(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com,
                                              QPointer<CorpusRecording> rec, QPointer<CorpusAnnotation> annot)
{
    int index = ui->tabWidgetEditors->currentIndex();
    if (ui->checkBoxFollowSelection->isChecked()) {
        d->corpus = corpus; d->com = com; d->rec = rec; d->annot = annot;
        if (index < 0 || index >= d->openEditors.count()) return;
        d->openEditors.at(index)->open(corpus, com, rec, annot);
    }
}

void ManualAnnotationWidget::moveToAnnotationTime(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com,
                                                  QPointer<CorpusAnnotation> annot, RealTime time)
{
    int index = ui->tabWidgetEditors->currentIndex();
    if (index < 0 || index >= d->openEditors.count()) return;
    d->openEditors.at(index)->jumpToTime(corpus, com, annot, time);
}

void ManualAnnotationWidget::editorTabNew()
{
    AnnotationEditorBase *editor(0);
    QString editorType = ui->comboBoxEditorSelection->currentData().toString();
    if      (editorType == "AnnotationMultiTierEditor") {
        editor = new AnnotationMultiTierEditor(this);
    }
    else if (editorType == "GroupingAnnotationEditor") {
        editor = new GroupingAnnotationEditor(this);
    }
    else if (editorType == "TranscriptAnnotationEditor") {
        editor = new TranscriptAnnotationEditor(this);
    }
    if (!editor) return;
    d->openEditors.append(editor);
    ui->tabWidgetEditors->addTab(editor, ui->comboBoxEditorSelection->currentText());
    ui->tabWidgetEditors->setCurrentWidget(editor);
    editor->open(d->corpus, d->com, d->rec, d->annot);
}

void ManualAnnotationWidget::editorTabCloseRequested(int index)
{
    AnnotationEditorBase *editor = d->openEditors.takeAt(index);
    if (editor) {
        editor->deleteLater();
        ui->tabWidgetEditors->removeTab(index);
    }
}
