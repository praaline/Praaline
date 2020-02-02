#include "PraalineCore/Query/QueryDefinition.h"
#include "PraalineCore/Query/QueryFilterSequence.h"
using namespace Praaline::Core;

#include "QueryFilterSequenceTableModel.h"

QueryFilterSequenceTableModel::QueryFilterSequenceTableModel(QueryFilterSequence &filterSequence, QObject *parent) :
    QAbstractTableModel(parent), m_filterSequence(filterSequence)
{
}

QueryFilterSequenceTableModel::~QueryFilterSequenceTableModel()
{
}

QModelIndex QueryFilterSequenceTableModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QModelIndex();
}

int QueryFilterSequenceTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_filterSequence.attributesCount();
}

int QueryFilterSequenceTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_filterSequence.sequenceLength() + 1;
}

QVariant QueryFilterSequenceTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Horizontal) {
        if (section > columnCount()) return QVariant();
        if (section == 0)
            return tr("Attribute");
        else
            return QString("T%1").arg(section);
    }
    return QVariant();
}

QVariant QueryFilterSequenceTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();
    if (index.row() < 0 || index.row() >= rowCount()) return QVariant();
    if (index.column() < 0 || index.column() >= columnCount()) return QVariant();
    if (m_filterSequence.attributesCount() == 0) return QVariant();

    QStringList attributeIDs = m_filterSequence.attributeIDs();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        if (index.column() == 0) {
            return attributeIDs.at(index.row());
        } else {
            QString attributeID = attributeIDs.at(index.row());
            return m_filterSequence.conditionString(attributeID, index.column() - 1);
        }
    }
    return QVariant();
}

Qt::ItemFlags QueryFilterSequenceTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) return Qt::ItemIsEnabled;
    if (index.column() > 0) return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
    return QAbstractTableModel::flags(index);
}

bool QueryFilterSequenceTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) return false;
    if (role != Qt::EditRole) return false;
    if (index.column() == 0) return false;
    if (m_filterSequence.attributesCount() == 0) return false;

    QStringList attributeIDs = m_filterSequence.attributeIDs();
    QString attributeID = attributeIDs.at(index.row());
    m_filterSequence.setConditionString(attributeID, index.column() - 1, value.toString());
    return true;
}
