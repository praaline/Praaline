#include "PraalineCore/Query/QueryDefinition.h"
using namespace Praaline::Core;

#include "QueryDefinitionTreeModel.h"

QueryDefinitionTreeModel::QueryDefinitionTreeModel(QueryDefinition &definition, QObject *parent) :
    QAbstractItemModel(parent), m_definition(definition)
{
}

QueryDefinitionTreeModel::~QueryDefinitionTreeModel()
{
}

int QueryDefinitionTreeModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 0;
}

int QueryDefinitionTreeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 0;
}

QModelIndex QueryDefinitionTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(row)
    Q_UNUSED(column)
    Q_UNUSED(parent)
    return QModelIndex();
}

QModelIndex QueryDefinitionTreeModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QModelIndex();
}

QVariant QueryDefinitionTreeModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(index)
    Q_UNUSED(role)
    return QVariant();
}

QVariant QueryDefinitionTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section)
    Q_UNUSED(orientation)
    Q_UNUSED(role)
    return QVariant();
}

Qt::ItemFlags QueryDefinitionTreeModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return Qt::ItemFlags();
}

bool QueryDefinitionTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(index)
    Q_UNUSED(value)
    Q_UNUSED(role)
    return false;
}

bool QueryDefinitionTreeModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    Q_UNUSED(position)
    Q_UNUSED(rows)
    Q_UNUSED(parent)
    return false;
}

bool QueryDefinitionTreeModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    Q_UNUSED(position)
    Q_UNUSED(rows)
    Q_UNUSED(parent)
    return false;
}

