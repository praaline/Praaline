#ifndef CORPUSBOOKMARKMODEL_H
#define CORPUSBOOKMARKMODEL_H

#include <QObject>
#include <QPointer>
#include <QAbstractTableModel>
#include <QList>

namespace Praaline {
namespace Core {
    class CorpusBookmark;
}
}

struct CorpusBookmarkModelData;

class CorpusBookmarkModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    CorpusBookmarkModel(QList<QPointer<Praaline::Core::CorpusBookmark> > bookmarks, QObject *parent = 0);
    ~CorpusBookmarkModel();

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
    CorpusBookmarkModelData *d;
};

#endif // CORPUSBOOKMARKMODEL_H
