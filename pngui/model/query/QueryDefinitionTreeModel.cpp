#include "pncore/query/QueryDefinition.h"
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
    return 0;
}

int QueryDefinitionTreeModel::columnCount(const QModelIndex &parent) const
{
    return 0;
}

QModelIndex QueryDefinitionTreeModel::index(int row, int column, const QModelIndex &parent) const
{
}

QModelIndex QueryDefinitionTreeModel::parent(const QModelIndex &index) const
{
}

QVariant QueryDefinitionTreeModel::data(const QModelIndex &index, int role) const
{
}

QVariant QueryDefinitionTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
}

Qt::ItemFlags QueryDefinitionTreeModel::flags(const QModelIndex &index) const
{
}

bool QueryDefinitionTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return false;
}

bool QueryDefinitionTreeModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    return false;
}

bool QueryDefinitionTreeModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    return false;
}

