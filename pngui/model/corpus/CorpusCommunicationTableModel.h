#ifndef CORPUSCOMMUNICATIONTABLEMODEL_H
#define CORPUSCOMMUNICATIONTABLEMODEL_H

#include <QObject>
#include <QPointer>
#include <QList>
#include <QAbstractTableModel>

namespace Praaline {
namespace Core {
class Corpus;
class CorpusCommunication;
class MetadataStructureAttribute;
}
}

struct CorpusCommunicationTableModelData;

class CorpusCommunicationTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    CorpusCommunicationTableModel(QList<QPointer<Praaline::Core::CorpusCommunication> > items,
                                  QList<QPointer<Praaline::Core::MetadataStructureAttribute> > attributes,
                                  QPointer<Praaline::Core::Corpus> corpus,
                                  bool multiCorpus = false, QObject *parent = 0);
    ~CorpusCommunicationTableModel();

    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    bool insertRows(int position, int rows, const QModelIndex &index=QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &index=QModelIndex());

signals:

public slots:

private:
    CorpusCommunicationTableModelData *d;
};

#endif // CORPUSCOMMUNICATIONTABLEMODEL_H
