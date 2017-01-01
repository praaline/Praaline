#ifndef QUERYDEFINITIONTREEMODEL_H
#define QUERYDEFINITIONTREEMODEL_H

#include <QObject>
#include <QAbstractItemModel>
#include "pncore/query/QueryDefinition.h"

class QueryDefinitionTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit QueryDefinitionTreeModel(Praaline::Core::QueryDefinition &definition, QObject *parent = 0);
    ~QueryDefinitionTreeModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    bool insertRows(int position, int rows, const QModelIndex &parent = QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex());

signals:

public slots:

private:
    Praaline::Core::QueryDefinition &m_definition;
};

#endif // QUERYDEFINITIONTREEMODEL_H
