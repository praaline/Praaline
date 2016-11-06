#ifndef CORPUSSPEAKERTABLEMODEL_H
#define CORPUSSPEAKERTABLEMODEL_H

#include <QObject>
#include <QPointer>
#include <QList>
#include <QAbstractTableModel>

namespace Praaline {
namespace Core {
class Corpus;
class CorpusSpeaker;
class MetadataStructureAttribute;
}
}

struct CorpusSpeakerTableModelData;

class CorpusSpeakerTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit CorpusSpeakerTableModel(QList<QPointer<Praaline::Core::CorpusSpeaker> > items,
                                     QList<QPointer<Praaline::Core::MetadataStructureAttribute> > attributes,
                                     QPointer<Praaline::Core::Corpus> corpus, bool multiCorpus = false, QObject *parent = 0);
    ~CorpusSpeakerTableModel();

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
    CorpusSpeakerTableModelData *d;
};

#endif // CORPUSSPEAKERTABLEMODEL_H
