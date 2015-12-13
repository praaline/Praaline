#ifndef QUERYOCCURRENCETABLEMODEL_H
#define QUERYOCCURRENCETABLEMODEL_H

#include <QObject>
#include <QPointer>
#include <QAbstractTableModel>
#include <QPair>
#include <QList>
#include <QHash>
#include "pncore/base/RealTime.h"
class QueryOccurrencePointer;
class QueryOccurrence;
class Corpus;
class QueryDefinition;

class QueryOccurrenceTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit QueryOccurrenceTableModel(QPointer<Corpus> corpus, QueryDefinition *queryDefinition,
                                       QList<QueryOccurrencePointer *> &pointers,
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
    QPointer<Corpus> m_corpus;
    QueryDefinition *m_queryDefinition;
    QList<QueryOccurrencePointer *> m_pointers;
    mutable QHash<int, QueryOccurrence *> m_occurrences;
    mutable QHash<int, QString> m_leftContext;
    mutable QHash<int, QString> m_rightContext;
    bool m_multiLine;
};

#endif // QUERYOCCURRENCETABLEMODEL_H
