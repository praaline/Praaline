#include <QString>
#include <QList>
#include <QMessageBox>

#include "pngui/model/annotation/AnnotationMultiTierTableModel.h"

#include "AnnotationWidgetDisfluencies.h"
#include "ui_AnnotationWidgetDisfluencies.h"

#include "DisfluencyAnalyser.h"

struct AnnotationWidgetDisfluenciesData {
    AnnotationMultiTierTableModel *model;
    int columnIndexToken, columnIndexDisfluency, columnIndexDiscourse;
    QList<int> rowsSelected;

    AnnotationWidgetDisfluenciesData() :
        model(nullptr), columnIndexToken(-1), columnIndexDisfluency(-1), columnIndexDiscourse(-1)
    {
    }
};


AnnotationWidgetDisfluencies::AnnotationWidgetDisfluencies(QWidget *parent) :
    QWidget(parent), ui(new Ui::AnnotationWidgetDisfluencies), d(new AnnotationWidgetDisfluenciesData())
{
    ui->setupUi(this);
    setEnabledSimpleDisfluencies(false);
    setEnabledStructuredDisfluencies(false);
}

AnnotationWidgetDisfluencies::~AnnotationWidgetDisfluencies()
{
    delete ui;
    delete d;
}

void AnnotationWidgetDisfluencies::setModel(AnnotationMultiTierTableModel *model)
{
    d->model = model;
    // Find the column indices relevant for disfluency editing (tok_min, tok_min/disfluency and tok_mwu/discourse).
    auto displayedAttributes = d->model->displayedAttributes();
    for (int i = 0; i < displayedAttributes.count(); ++i) {
        if (displayedAttributes.at(i).first == "tok_min") {
            if (displayedAttributes.at(i).second.isEmpty())
                d->columnIndexToken = i + 3; // found tok_min text attribute
            else if (displayedAttributes.at(i).second == "disfluency")
                d->columnIndexDisfluency = i + 3; // found disfluency attribute of the tok_min level
        }
        else if (displayedAttributes.at(i).first == "tok_mwu") {
            if (displayedAttributes.at(i).second == "discourse") {
                d->columnIndexDiscourse = i + 3;
            }
        }
    }
    // Explanation for +3: in the AnnotationMultiTierTableModel, the first three columns are: speakerID, tMin, tMax.
}

void AnnotationWidgetDisfluencies::setColumnIndexes(int columnIndexToken, int columnIndexDisfluency, int columnIndexDiscourse)
{
    // If known from some other source, the column indexes can be set manually.
    d->columnIndexToken = columnIndexToken;
    d->columnIndexDisfluency = columnIndexDisfluency;
    d->columnIndexDiscourse = columnIndexDiscourse;
}

void AnnotationWidgetDisfluencies::setEnabledSimpleDisfluencies(bool enable)
{
    ui->cmdFilledPause->setEnabled(enable);
    ui->cmdFalseStart->setEnabled(enable);
    ui->cmdLengthening->setEnabled(enable);
    ui->cmdWithinWordPause->setEnabled(enable);
}

void AnnotationWidgetDisfluencies::setEnabledStructuredDisfluencies(bool enable)
{
    ui->cmdRepetition->setEnabled(enable);
    ui->cmdSubstitution->setEnabled(enable);
    ui->cmdInsertion->setEnabled(enable);
    ui->cmdComplex->setEnabled(enable);
}

void AnnotationWidgetDisfluencies::setSelection(QList<int> rowsSelected)
{
    d->rowsSelected = rowsSelected;
    if (d->rowsSelected.count() == 1) {
        setEnabledSimpleDisfluencies(true);
        setEnabledStructuredDisfluencies(false);
        ui->cmdDeletion->setEnabled(true);
    } else if (d->rowsSelected.count() > 1) {
        setEnabledSimpleDisfluencies(false);
        setEnabledStructuredDisfluencies(true);
        ui->cmdDeletion->setEnabled(true);
    }
    else {
        setEnabledSimpleDisfluencies(false);
        setEnabledStructuredDisfluencies(false);
        ui->cmdDeletion->setEnabled(false);
    }
}

void AnnotationWidgetDisfluencies::annotateSimple(DisfluencyTypesEnum type)
{
    if (d->rowsSelected.count() != 1) return;
    QModelIndex tok = d->model->index(d->rowsSelected[0], d->columnIndexToken);
    QModelIndex dis = d->model->index(d->rowsSelected[0], d->columnIndexDisfluency);
    if (type == AnnotationWidgetDisfluencies::FIL) {
        d->model->setData(dis, "FIL");
    } else if (type == AnnotationWidgetDisfluencies::LEN) {
        d->model->setData(dis, "LEN");
    } else if (type == AnnotationWidgetDisfluencies::FST) {
        d->model->setData(dis, "FST");
        QString token = d->model->data(tok, Qt::EditRole).toString();
        if (token.endsWith("/") || token.endsWith("-")) token.chop(1);
        token = token.append("/");
        d->model->setData(tok, token);
    } else if (type == AnnotationWidgetDisfluencies::WDP) {
        d->model->setData(dis, "WDP");
    }
}

void AnnotationWidgetDisfluencies::on_cmdFilledPause_clicked()
{
    annotateSimple(AnnotationWidgetDisfluencies::FIL);
}

void AnnotationWidgetDisfluencies::on_cmdLengthening_clicked()
{
    annotateSimple(AnnotationWidgetDisfluencies::LEN);
}

void AnnotationWidgetDisfluencies::on_cmdFalseStart_clicked()
{
    annotateSimple(AnnotationWidgetDisfluencies::FST);
}

void AnnotationWidgetDisfluencies::on_cmdWithinWordPause_clicked()
{
    annotateSimple(AnnotationWidgetDisfluencies::WDP);
}

void AnnotationWidgetDisfluencies::annotateStructured(DisfluencyTypesEnum type)
{
    QList<DisfluencyAnalyser::Token> tokens;
    if (d->rowsSelected.count() <= 1 && type != DEL) return;
    if (d->rowsSelected.count() < 1 && type == DEL) return;
    for (int i = 0; i < d->rowsSelected.count(); i++) {
        QString tok = d->model->data(d->model->index(d->rowsSelected[i], d->columnIndexToken), Qt::EditRole).toString();
        QString dis = d->model->data(d->model->index(d->rowsSelected[i], d->columnIndexDisfluency), Qt::EditRole).toString();
        tokens << DisfluencyAnalyser::Token(tok, dis);
    }
    QList<QString> tags;
    if (type == REP) {
        tags = DisfluencyAnalyser::annotateRepetition(tokens);
    }
    else if (type == SUB) {
        tags = DisfluencyAnalyser::annotateSubstitution(tokens);
    }
    else if (type == INS) {
        tags = DisfluencyAnalyser::annotateInsertion(tokens);
    }
    else if (type == DEL) {
        tags = DisfluencyAnalyser::annotateDeletion(tokens);
    }
    else if (type == COM) {
        tags = DisfluencyAnalyser::annotateComplexBase(tokens);
    }
    if (tags.count() != tokens.count()) return; // something went wrong
    // Save tags
    for (int i = 0; i < d->rowsSelected.count(); i++) {
        d->model->setData(d->model->index(d->rowsSelected[i], d->columnIndexDisfluency), tags.at(i));
    }
}

void AnnotationWidgetDisfluencies::on_cmdRepetition_clicked()
{
    annotateStructured(AnnotationWidgetDisfluencies::REP);
}

void AnnotationWidgetDisfluencies::on_cmdSubstitution_clicked()
{
    annotateStructured(AnnotationWidgetDisfluencies::SUB);
}

void AnnotationWidgetDisfluencies::on_cmdInsertion_clicked()
{
    annotateStructured(AnnotationWidgetDisfluencies::INS);
}

void AnnotationWidgetDisfluencies::on_cmdDeletion_clicked()
{
    annotateStructured(AnnotationWidgetDisfluencies::DEL);
}

void AnnotationWidgetDisfluencies::on_cmdComplex_clicked()
{
    annotateStructured(AnnotationWidgetDisfluencies::COM);
}

void AnnotationWidgetDisfluencies::on_cmdBoundaryBreak_clicked()
{
    if (d->rowsSelected.count() != 1) return;
    QModelIndex index = d->model->index(d->rowsSelected[0], d->columnIndexDiscourse);
    d->model->setData(index, "*");
}

void AnnotationWidgetDisfluencies::on_cmdBoundaryContinue_clicked()
{
    if (d->rowsSelected.count() != 1) return;
    QModelIndex index = d->model->index(d->rowsSelected[0], d->columnIndexDiscourse);
    d->model->setData(index, "");
}

void AnnotationWidgetDisfluencies::on_cmdDiscourseMarker_clicked()
{
    if (d->rowsSelected.count() != 1) return;
    QModelIndex index = d->model->index(d->rowsSelected[0], d->columnIndexDiscourse);
    d->model->setData(index, "DM");
}

