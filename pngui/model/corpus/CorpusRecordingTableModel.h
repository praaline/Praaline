#ifndef CORPUSRECORDINGTABLEMODEL_H
#define CORPUSRECORDINGTABLEMODEL_H

#include <QObject>
#include <QPointer>
#include <QList>
#include <QAbstractTableModel>

namespace Praaline {
namespace Core {
class CorpusRecording;
class MetadataStructureAttribute;
}
}

struct CorpusRecordingTableModelData;

class CorpusRecordingTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit CorpusRecordingTableModel(QList<QPointer<Praaline::Core::CorpusRecording> > items,
                                       QList<QPointer<Praaline::Core::MetadataStructureAttribute> > attributes,
                                       bool multiCommunication = true, QObject *parent = 0);
    ~CorpusRecordingTableModel();

    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex());

signals:

public slots:

private:
    CorpusRecordingTableModelData *d;
};

#endif // CORPUSRECORDINGTABLEMODEL_H
