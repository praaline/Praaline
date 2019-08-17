#ifndef CORPUSSPEAKERTABLEMODEL_H
#define CORPUSSPEAKERTABLEMODEL_H

#include <QObject>
#include <QPointer>
#include <QList>
#include <QAbstractTableModel>

namespace Praaline {
namespace Core {
class CorpusSpeaker;
class MetadataStructureAttribute;
}
}

struct CorpusSpeakerTableModelData;

class CorpusSpeakerTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit CorpusSpeakerTableModel(QList<Praaline::Core::CorpusSpeaker *> items,
                                     QList<Praaline::Core::MetadataStructureAttribute *> attributes,
                                     bool multiCorpus = true, QObject *parent = nullptr);
    ~CorpusSpeakerTableModel() override;

    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex()) override;
    bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex()) override;

signals:

public slots:

private:
    CorpusSpeakerTableModelData *d;
};

#endif // CORPUSSPEAKERTABLEMODEL_H
