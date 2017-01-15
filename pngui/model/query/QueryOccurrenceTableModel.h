#ifndef QUERYOCCURRENCETABLEMODEL_H
#define QUERYOCCURRENCETABLEMODEL_H

#include <QObject>
#include <QPointer>
#include <QAbstractTableModel>
#include <QPair>
#include <QList>
#include <QHash>
#include "pncore/base/RealTime.h"

namespace Praaline {
namespace Core {
class CorpusRepository;
class QueryDefinition;
class QueryOccurrencePointer;
class QueryOccurrence;
}
}

struct QueryOccurrenceTableModelData;

class QueryOccurrenceTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit QueryOccurrenceTableModel(Praaline::Core::CorpusRepository *repository,
                                       Praaline::Core::QueryDefinition *queryDefinition,
                                       QList<Praaline::Core::QueryOccurrencePointer *> &pointers,
                                       bool multiline = false, QObject *parent = 0);
    ~QueryOccurrenceTableModel();

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
    QueryOccurrenceTableModelData *d;
};

#endif // QUERYOCCURRENCETABLEMODEL_H
