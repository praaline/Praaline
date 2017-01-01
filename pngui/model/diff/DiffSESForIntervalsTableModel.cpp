#include <QDebug>
#include <QObject>
#include <QList>
#include <QAbstractTableModel>
#include "pncore/annotation/Interval.h"
using namespace Praaline::Core;
using namespace std;
#include "pnlib/diff/dtl.h"
using namespace dtl;
#include "pnlib/diff/diffintervals.h"

#include "DiffSESForIntervalsTableModel.h"

DiffSESforIntervalsTableModel::DiffSESforIntervalsTableModel(dtl::Ses<Interval *>::sesElemVec &sesElemVec,
                                                             const QList<Interval *> &intervals_A, const QList<Interval *> &intervals_B,
                                                             const QString &attributeID_A, const QString &attributeID_B,
                                                             const QStringList &extraAttributeIDs_A, const QStringList &extraAttributeIDs_B,
                                                             QObject *parent) :
    QAbstractTableModel(parent), m_sesElemVec(sesElemVec), m_intervals_A(intervals_A), m_intervals_B(intervals_B),
    m_attributeID_A(attributeID_A), m_attributeID_B(attributeID_B),
    m_extraAttributeIDs_A(extraAttributeIDs_A), m_extraAttributeIDs_B(extraAttributeIDs_B)
{
    m_colorAddition = QColor(204, 255, 204);
    m_colorDeletion = QColor(255, 204, 255);
    m_colorSame = QColor(255, 255, 255);
}

DiffSESforIntervalsTableModel::~DiffSESforIntervalsTableModel()
{
}

void DiffSESforIntervalsTableModel::setColors(const QColor &colorSame, const QColor &colorAddition, const QColor &colorDeletion)
{
    m_colorAddition = colorAddition;
    m_colorDeletion = colorDeletion;
    m_colorSame = colorSame;
}

QModelIndex DiffSESforIntervalsTableModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QModelIndex();
}

int DiffSESforIntervalsTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_sesElemVec.size();
}

int DiffSESforIntervalsTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 7 + m_extraAttributeIDs_A.count() + m_extraAttributeIDs_B.count();
}

QVariant DiffSESforIntervalsTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    int indexInA = section - 7;
    int indexInB = section - 7 - m_extraAttributeIDs_A.count();

    if (orientation == Qt::Horizontal) {
        if      (section == 0) return tr("OP");
        else if (section == 1) return tr("tMin A");
        else if (section == 2) return tr("tMax A");
        else if (section == 3) return tr("tMin B");
        else if (section == 4) return tr("tMax B");
        else if (section == 5) return tr("A");
        else if (section == 6) return tr("B");
        else if ((indexInA >= 0) && (indexInA < m_extraAttributeIDs_A.count())) {
            return QString("%1 A").arg(m_extraAttributeIDs_A.at(indexInA));
        }
        else if ((indexInB >= 0) && (indexInB < m_extraAttributeIDs_B.count())) {
            return QString("%1 B").arg(m_extraAttributeIDs_B.at(indexInB));
        }
        else {
            return QVariant();
        }
    }
    return QVariant();
}

QVariant DiffSESforIntervalsTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (index.row() >= rowCount() || index.row() < 0)
        return QVariant();
    if (index.row() >= m_sesElemVec.size())
        return QVariant();

    int indexInA = index.column() - 7;
    int indexInB = index.column() - 7 - m_extraAttributeIDs_A.count();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        dtl::Ses<Interval *>::sesElem elem = m_sesElemVec[index.row()];
        Interval *intv_A = (elem.second.beforeIdx > 0) ? m_intervals_A.at(elem.second.beforeIdx - 1) : 0;
        Interval *intv_B = (elem.second.afterIdx > 0) ? m_intervals_B.at(elem.second.afterIdx - 1) : 0;
        dtl::edit_t editType = elem.second.type;

        if (index.column() == 0) {
            if      (editType == dtl::SES_COMMON) return "=";
            else if (editType == dtl::SES_ADD)    return "+";
            else if (editType == dtl::SES_DELETE) return "-";
        }
        else if (index.column() == 1) {
            if ((editType == dtl::SES_COMMON || editType == dtl::SES_DELETE) && intv_A)
                return intv_A->tMin().toDouble();
            else return QVariant();
        }
        else if (index.column() == 2) {
            if ((editType == dtl::SES_COMMON || editType == dtl::SES_DELETE) && intv_A)
                return intv_A->tMax().toDouble();
            else return QVariant();
        }
        else if (index.column() == 3) {
            if ((editType == dtl::SES_COMMON || editType == dtl::SES_ADD) && intv_B)
                return intv_B->tMin().toDouble();
            else return QVariant();
        }
        else if (index.column() == 4) {
            if ((editType == dtl::SES_COMMON || editType == dtl::SES_ADD) && intv_B)
                return intv_B->tMax().toDouble();
            else return QVariant();
        }
        else if (index.column() == 5) {
            if ((editType == dtl::SES_COMMON || editType == dtl::SES_DELETE) && intv_A) {
                return (m_attributeID_A.isEmpty()) ? intv_A->text() : intv_A->attribute(m_attributeID_A);
            }
            else return QVariant();
        }
        else if (index.column() == 6) {
            if ((editType == dtl::SES_COMMON || editType == dtl::SES_ADD) && intv_B) {
                return (m_attributeID_B.isEmpty()) ? intv_B->text() : intv_B->attribute(m_attributeID_B);
            }
            else return QVariant();
        }
        else if ((indexInA >= 0) && (indexInA < m_extraAttributeIDs_A.count())) {
            if ((editType == dtl::SES_COMMON || editType == dtl::SES_DELETE) && intv_A) {
                return intv_A->attribute(m_extraAttributeIDs_A.at(indexInA));
            }
            else return QVariant();
        }
        else if ((indexInB >= 0) && (indexInB < m_extraAttributeIDs_B.count())) {
            if ((editType == dtl::SES_COMMON || editType == dtl::SES_ADD) && intv_B) {
                return intv_B->attribute(m_extraAttributeIDs_B.at(indexInB));
            }
            else return QVariant();
        }
        else {
            return QVariant();
        }
    }
    else if (role == Qt::BackgroundColorRole) {
        dtl::edit_t editType = m_sesElemVec[index.row()].second.type;
        switch (editType) {
        case dtl::SES_COMMON:   return m_colorSame;
        case dtl::SES_ADD:      return m_colorAddition;
        case dtl::SES_DELETE:   return m_colorDeletion;
        }
        return QColor("White");
    }
    return QVariant();
}

Qt::ItemFlags DiffSESforIntervalsTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;
//    if (index.column() >= 5)
//        return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
    return QAbstractTableModel::flags(index);
}
