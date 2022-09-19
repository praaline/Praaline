#ifndef SEQUENCESTABLEMODEL_H
#define SEQUENCESTABLEMODEL_H

#include <QObject>
#include <QString>
#include <QPointer>
#include <QAbstractTableModel>

#include "PraalineCore/Annotation/AnnotationTierGroup.h"
#include "PraalineCore/Annotation/Sequence.h"

struct SequencesTableModelData;

class SequencesTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit SequencesTableModel(Praaline::Core::SpeakerAnnotationTierGroupMap &tiers,
                                 const QString &sequenceLevelID,
                                 const QList<QString> &sequenceAttributes,
                                 QObject *parent = nullptr);
    ~SequencesTableModel() override;

    // Table model implementation
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    // Additional functionality
    QString sequenceTierName() const;
    QList<QString> displayedSequenceAttributes() const;
    QPair<QString, int> modelRowToSpeakerAndSequenceIndex(int row) const;

    // Add and remove sequences
    bool addSequence(const QString &speakerID, Praaline::Core::Sequence *sequence);
    bool removeRows(int position, int rows, const QModelIndex &parent) override;

private:
    SequencesTableModelData *d;
};

#endif // SEQUENCESTABLEMODEL_H
