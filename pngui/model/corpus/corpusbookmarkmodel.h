#ifndef CORPUSBOOKMARKMODEL_H
#define CORPUSBOOKMARKMODEL_H

#include <QObject>
#include <QPointer>
#include <QAbstractTableModel>
#include <QList>
#include "pncore/corpus/corpusbookmark.h"

class CorpusBookmarkModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    CorpusBookmarkModel(QList<QPointer<CorpusBookmark> > bookmarks, QObject *parent = 0);

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole);
    // bool insertRows(int position, int rows, const QModelIndex &index=QModelIndex());
    // bool removeRows(int position, int rows, const QModelIndex &index=QModelIndex());

signals:

public slots:

private:
    QList<QPointer<CorpusBookmark> > m_bookmarks;
};

#endif // CORPUSBOOKMARKMODEL_H
