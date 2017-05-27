#include <QString>
#include <QList>
#include <QMessageBox>

#include "pngui/model/annotation/AnnotationMultiTierTableModel.h"

#include "AnnotationControlsDisfluencies.h"
#include "ui_AnnotationControlsDisfluencies.h"

#include "DisfluencyAnalyser.h"

AnnotationControlsDisfluencies::AnnotationControlsDisfluencies(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AnnotationControlsDisfluencies)
{
    ui->setupUi(this);
    setEnabledSimpleDisfluencies(false);
    setEnabledStructuredDisfluencies(false);
}

AnnotationControlsDisfluencies::~AnnotationControlsDisfluencies()
{
    delete ui;
}

void AnnotationControlsDisfluencies::setModel(AnnotationMultiTierTableModel *model,
                                              int columnToken, int columnDisfluency, int columnDiscourse)
{
    m_model = model;
    m_columnToken = columnToken;
    m_columnDisfluency = columnDisfluency;
    m_columnDiscourse = columnDiscourse;
}

void AnnotationControlsDisfluencies::setEnabledSimpleDisfluencies(bool enable)
{
    ui->cmdFilledPause->setEnabled(enable);
    ui->cmdFalseStart->setEnabled(enable);
    ui->cmdLengthening->setEnabled(enable);
    ui->cmdWithinWordPause->setEnabled(enable);
}

void AnnotationControlsDisfluencies::setEnabledStructuredDisfluencies(bool enable)
{
    ui->cmdRepetition->setEnabled(enable);
    ui->cmdSubstitution->setEnabled(enable);
    ui->cmdInsertion->setEnabled(enable);
    ui->cmdComplex->setEnabled(enable);
}

void AnnotationControlsDisfluencies::setSelection(QList<int> rowsSelected)
{
    m_rowsSelected = rowsSelected;
    if (m_rowsSelected.count() == 1) {
        setEnabledSimpleDisfluencies(true);
        setEnabledStructuredDisfluencies(false);
        ui->cmdDeletion->setEnabled(true);
    } else if (m_rowsSelected.count() > 1) {
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

void AnnotationControlsDisfluencies::annotateSimple(DisfluencyTypesEnum type)
{
    if (m_rowsSelected.count() != 1) return;
    QModelIndex t = m_model->index(m_rowsSelected[0], m_columnToken);
    QModelIndex d = m_model->index(m_rowsSelected[0], m_columnDisfluency);
    if (type == AnnotationControlsDisfluencies::FIL) {
        m_model->setData(d, "FIL");
    } else if (type == AnnotationControlsDisfluencies::LEN) {
        m_model->setData(d, "LEN");
    } else if (type == AnnotationControlsDisfluencies::FST) {
        m_model->setData(d, "FST");
        QString token = m_model->data(t, Qt::EditRole).toString();
        if (token.endsWith("/") || token.endsWith("-")) token.chop(1);
        token = token.append("/");
        m_model->setData(t, token);
    } else if (type == AnnotationControlsDisfluencies::WDP) {
        m_model->setData(d, "WDP");
    }
}

void AnnotationControlsDisfluencies::on_cmdFilledPause_clicked()
{
    annotateSimple(AnnotationControlsDisfluencies::FIL);
}

void AnnotationControlsDisfluencies::on_cmdLengthening_clicked()
{
    annotateSimple(AnnotationControlsDisfluencies::LEN);
}

void AnnotationControlsDisfluencies::on_cmdFalseStart_clicked()
{
    annotateSimple(AnnotationControlsDisfluencies::FST);
}

void AnnotationControlsDisfluencies::on_cmdWithinWordPause_clicked()
{
    annotateSimple(AnnotationControlsDisfluencies::WDP);
}

void AnnotationControlsDisfluencies::annotateStructured(DisfluencyTypesEnum type)
{
    QList<DisfluencyAnalyser::Token> tokens;
    if (m_rowsSelected.count() <= 1 && type != DEL) return;
    if (m_rowsSelected.count() < 1 && type == DEL) return;
    for (int i = 0; i < m_rowsSelected.count(); i++) {
        QString t = m_model->data(m_model->index(m_rowsSelected[i], m_columnToken), Qt::EditRole).toString();
        QString d = m_model->data(m_model->index(m_rowsSelected[i], m_columnDisfluency), Qt::EditRole).toString();
        tokens << DisfluencyAnalyser::Token(t, d);
    }
    QList<QString> tags;
    switch (type) {
    case REP:
        tags = DisfluencyAnalyser::annotateRepetition(tokens);
        break;
    case SUB:
        tags = DisfluencyAnalyser::annotateSubstitution(tokens);
        break;
    case INS:
        tags = DisfluencyAnalyser::annotateInsertion(tokens);
        break;
    case DEL:
        tags = DisfluencyAnalyser::annotateDeletion(tokens);
        break;
    case COM:
        tags = DisfluencyAnalyser::annotateComplexBase(tokens);
        break;
    }
    if (tags.count() != tokens.count()) return; // something went wrong
    // Save tags
    for (int i = 0; i < m_rowsSelected.count(); i++) {
        m_model->setData(m_model->index(m_rowsSelected[i], m_columnDisfluency), tags.at(i));
    }
}

void AnnotationControlsDisfluencies::on_cmdRepetition_clicked()
{
    annotateStructured(AnnotationControlsDisfluencies::REP);
}

void AnnotationControlsDisfluencies::on_cmdSubstitution_clicked()
{
    annotateStructured(AnnotationControlsDisfluencies::SUB);
}

void AnnotationControlsDisfluencies::on_cmdInsertion_clicked()
{
    annotateStructured(AnnotationControlsDisfluencies::INS);
}

void AnnotationControlsDisfluencies::on_cmdDeletion_clicked()
{
    annotateStructured(AnnotationControlsDisfluencies::DEL);
}

void AnnotationControlsDisfluencies::on_cmdComplex_clicked()
{
    annotateStructured(AnnotationControlsDisfluencies::COM);
}

void AnnotationControlsDisfluencies::on_cmdBoundaryBreak_clicked()
{
    if (m_rowsSelected.count() != 1) return;
    QModelIndex d = m_model->index(m_rowsSelected[0], m_columnDiscourse);
    m_model->setData(d, "*");
}

void AnnotationControlsDisfluencies::on_cmdBoundaryContinue_clicked()
{
    if (m_rowsSelected.count() != 1) return;
    QModelIndex d = m_model->index(m_rowsSelected[0], m_columnDiscourse);
    m_model->setData(d, "");
}

void AnnotationControlsDisfluencies::on_cmdDiscourseMarker_clicked()
{
    if (m_rowsSelected.count() != 1) return;
    QModelIndex d = m_model->index(m_rowsSelected[0], m_columnDiscourse);
    m_model->setData(d, "MD");
}

