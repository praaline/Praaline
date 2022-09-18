#ifndef SPEAKERRELATIONTABLEMODEL_H
#define SPEAKERRELATIONTABLEMODEL_H

#include <QObject>
#include <QString>
#include <QAbstractTableModel>

namespace Praaline {
namespace Core {
class CorpusCommunication;
class CorpusCommunicationSpeakerRelation;
}
}

struct SpeakerRelationTableModelData;

class SpeakerRelationTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    SpeakerRelationTableModel(Praaline::Core::CorpusCommunication *com, QObject *parent = nullptr);
    ~SpeakerRelationTableModel() override;

    // Table model implementation
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    // Add and remove relations
    bool insertRows(int position, int rows, const QModelIndex &parent) override;
    bool removeRows(int position, int rows, const QModelIndex &parent) override;

private:
    SpeakerRelationTableModelData *d;
};

#endif // SPEAKERRELATIONTABLEMODEL_H
