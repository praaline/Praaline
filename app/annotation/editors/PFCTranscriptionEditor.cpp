#include <QFont>
#include <QFontDatabase>
#include <QStandardItemModel>

#include "PraalineCore/Corpus/Corpus.h"
#include "PraalineCore/Corpus/CorpusCommunication.h"
#include "PraalineCore/Corpus/CorpusAnnotation.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"
#include "PraalineCore/Annotation/AnnotationTierGroup.h"
#include "PraalineCore/Annotation/IntervalTier.h"
using namespace Praaline::Core;

#include "pngui/model/annotation/AnnotationElementTableModel.h"
#include "CorpusRepositoriesManager.h"

#include "PFCTranscriptionEditor.h"
#include "ui_PFCTranscriptionEditor.h"

struct PFCTranscriptionEditorData
{
    QPointer<CorpusRepository> repository;
    QList<AnnotationElement *> elements;
    int index;
    bool editing;
    QSharedPointer<QStandardItemModel> model;
};

PFCTranscriptionEditor::PFCTranscriptionEditor(QWidget *parent) :
    QMainWindow(parent), d(new PFCTranscriptionEditorData), ui(new Ui::PFCTranscriptionEditor)
{
    ui->setupUi(this);
    connect(ui->commandPrevious, &QAbstractButton::clicked, this, &PFCTranscriptionEditor::previous);
    connect(ui->commandNext, &QAbstractButton::clicked, this, &PFCTranscriptionEditor::next);
    connect(ui->commandSave, &QAbstractButton::clicked, this, &PFCTranscriptionEditor::save);
    connect(ui->commandGoto, &QAbstractButton::clicked, this, &PFCTranscriptionEditor::gotoIndex);

    connect(ui->editTranscript, &QLineEdit::textChanged, this, &PFCTranscriptionEditor::updateTableView);
    connect(ui->editSchwa, &QLineEdit::textChanged, this, &PFCTranscriptionEditor::updateTableView);
    connect(ui->editLiaison, &QLineEdit::textChanged, this, &PFCTranscriptionEditor::updateTableView);

    QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    ui->editTranscript->setFont(fixedFont);
    ui->editLiaison->setFont(fixedFont);
    ui->editSchwa->setFont(fixedFont);
    ui->labelIndex->setFont(fixedFont);
    d->editing = false;
}

PFCTranscriptionEditor::~PFCTranscriptionEditor()
{
    delete ui;
    delete d;
}

void PFCTranscriptionEditor::open(Corpus *corpus, CorpusCommunication *com, CorpusRecording *rec, CorpusAnnotation *annot)
{
    if (!corpus) return;
    if (!corpus->repository()) return;
    d->repository = corpus->repository();
    d->elements.clear();
    QList<AnnotationElement *> elements = d->repository->annotations()->getAnnotationElements(
                AnnotationDatastore::Selection("", "", "transcription"));
    foreach (AnnotationElement *element, elements) {
        if (!element->attribute("tocheck").toString().isEmpty())
            d->elements << element;
    }
    d->index = 0;
    ui->labelTotalToCheck->setText(QString("Total to check: %1").arg(d->elements.count()));
    edit();
}

void PFCTranscriptionEditor::jumpToTime(Corpus *corpus, CorpusCommunication *com, CorpusAnnotation *annot, const RealTime &time)
{
}

void PFCTranscriptionEditor::previous()
{
    update();
    d->index = d->index - 1;
    if (d->index < 0) {
        d->index = d->elements.count() - 1;
    }
    edit();
}

void PFCTranscriptionEditor::next()
{
    update();
    d->index = d->index + 1;
    if (d->index >= d->elements.count()) {
        d->index = 0;
    }
    edit();
}

void PFCTranscriptionEditor::edit()
{
    if (d->elements.isEmpty()) return;
    if (d->index < 0) d->index = 0;
    if (d->index >= d->elements.count()) d->index = d->elements.count() - 1;
    ui->editTranscript->setText(d->elements.at(d->index)->text());
    ui->editLiaison->setText(d->elements.at(d->index)->attribute("liaison").toString());
    ui->editSchwa->setText(d->elements.at(d->index)->attribute("schwa").toString());
    ui->editComment->setText(d->elements.at(d->index)->attribute("comment").toString());
    ui->labelCheck->setText(d->elements.at(d->index)->attribute("tocheck").toString());
    ui->labelAnnotationID->setText(QString("AnnotationID: %1 Interval Index: %2")
                                   .arg(d->elements.at(d->index)->attribute("annotationID").toString(),
                                        d->elements.at(d->index)->attribute("indexNo").toString()));
    ui->labelIndex->setText(QString("Index: %1\n%2\n%3")
                            .arg(d->index + 1)
                            .arg(d->elements.at(d->index)->attribute("wordalign").toString(),
                                 d->elements.at(d->index)->attribute("wordalign_wer").toString())
                            );
    updateTableView();
    ui->editTranscript->setFocus();
    d->editing = true;
}

void PFCTranscriptionEditor::update()
{
    if (!d->editing) return;
    if (d->index < 0) return;
    if (d->index >= d->elements.count()) return;
    d->elements.at(d->index)->setText(ui->editTranscript->text());
    d->elements.at(d->index)->setAttribute("liaison", ui->editLiaison->text());
    d->elements.at(d->index)->setAttribute("schwa", ui->editSchwa->text());
    d->elements.at(d->index)->setAttribute("comment", ui->editComment->text());
}

void PFCTranscriptionEditor::gotoIndex()
{
    int newIndex = ui->editGoto->text().toInt() - 1;
    if (newIndex < 0) return;
    if (newIndex >= d->elements.count()) return;
    d->index = newIndex;
    update();
}

void PFCTranscriptionEditor::save()
{
    update();
    if (!d->repository) return;
    if (d->elements.isEmpty()) return;
    bool result = d->repository->annotations()->saveAnnotationElements(d->elements, "transcription", QStringList() << "liaison" << "schwa");
    if (result)
        ui->statusbar->showMessage("Saved updates in database.", 1000);
}

void PFCTranscriptionEditor::updateTableView()
{
    d->model = QSharedPointer<QStandardItemModel>(new QStandardItemModel());
    QString ortho = ui->editTranscript->text();
    QString schwa = ui->editSchwa->text();
    QString liaison = ui->editLiaison->text();
    ortho = ortho.replace("< ", "<").replace("<", " <").replace(" >", ">").replace(">", "> ").replace(" :", ":")
            .replace(" )", ")").replace("( ", "(")
            .replace(" .", ".").replace(" ,", ",")
            .replace("  ", " ").replace("  ", " ").replace("  ", " ").trimmed();
    schwa = schwa.replace("< ", "<").replace("<", " <").replace(" >", ">").replace(">", "> ").replace(" :", ":")
            .replace(" )", ")").replace("( ", "(")
            .replace(" .", ".").replace(" ,", ",")
            .replace("  ", " ").replace("  ", " ").replace("  ", " ").trimmed();
    liaison = liaison.replace("< ", "<").replace("<", " <").replace(" >", ">").replace(">", "> ").replace(" :", ":")
            .replace(" )", ")").replace("( ", "(")
            .replace(" .", ".").replace(" ,", ",")
            .replace("  ", " ").replace("  ", " ").replace("  ", " ").trimmed();
    if (liaison.isEmpty()) liaison = ortho;
    if (schwa.isEmpty()) schwa = ortho;
    QList<QStandardItem *> itemsOrtho, itemsSchwa, itemsLiaison;
    foreach (QString o, ortho.split(" ")) itemsOrtho << new QStandardItem(o);
    foreach (QString s, schwa.split(" ")) itemsSchwa << new QStandardItem(s);
    foreach (QString l, liaison.split(" ")) itemsLiaison << new QStandardItem(l);
    d->model->insertColumn(0, itemsOrtho);
    d->model->insertColumn(1, itemsSchwa);
    d->model->insertColumn(2, itemsLiaison);
    ui->tableView->setModel(d->model.data());
}

