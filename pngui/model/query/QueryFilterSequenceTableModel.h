#ifndef QUERYFILTERSEQUENCETABLEMODEL_H
#define QUERYFILTERSEQUENCETABLEMODEL_H

#include <QObject>
#include <QPointer>
#include <QAbstractTableModel>
#include "PraalineCore/Query/QueryDefinition.h"
#include "PraalineCore/Query/QueryFilterSequence.h"

class QueryFilterSequenceTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit QueryFilterSequenceTableModel(Praaline::Core::QueryFilterSequence &filterSequence, QObject *parent = nullptr);
    ~QueryFilterSequenceTableModel();

    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole);

signals:

public slots:
private:
    Praaline::Core::QueryFilterSequence &m_filterSequence;
};

#endif // QUERYFILTERSEQUENCETABLEMODEL_H
