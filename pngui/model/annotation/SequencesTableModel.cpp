#include <QObject>
#include <QString>
#include <QPointer>
#include <QAbstractTableModel>
#include <QDebug>

#include "PraalineCore/Annotation/AnnotationTierGroup.h"
#include "PraalineCore/Annotation/SequenceTier.h"
using namespace Praaline::Core;

#include "SequencesTableModel.h"

struct SequencesTableModelData {
    QString sequenceLevelID;
    QMap<QString, SequenceTier *> sequenceTiers;
    QList<QString> sequenceAttributes;

    SequencesTableModelData()
    {}
};

SequencesTableModel::SequencesTableModel(Praaline::Core::SpeakerAnnotationTierGroupMap &tiers,
                                         const QString &sequenceLevelID,
                                         const QList<QString> &sequenceAttributes,
                                         QObject *parent) :
    QAbstractTableModel(parent), d(new SequencesTableModelData())
{  
    d->sequenceLevelID = sequenceLevelID;
    d->sequenceAttributes = sequenceAttributes;
    foreach (const QString &speakerID, tiers.keys()) {
        AnnotationTierGroup *tiersSpk = tiers.value(speakerID);
        SequenceTier *sequenceTier = tiersSpk->getSequenceTierByName(d->sequenceLevelID);
        if (sequenceTier) d->sequenceTiers.insert(speakerID, sequenceTier);
    }
}

SequencesTableModel::~SequencesTableModel()
{
    delete d;
}

QModelIndex SequencesTableModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

QString SequencesTableModel::sequenceTierName() const
{
    return d->sequenceLevelID;
}

QList<QString> SequencesTableModel::displayedSequenceAttributes() const
{
    return d->sequenceAttributes;
}

int SequencesTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    int count(0);
    for (SequenceTier *tier : qAsConst(d->sequenceTiers)) {
        count += tier->count();
    }
    return count;
}

int SequencesTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return d->sequenceAttributes.count() + 5;
}

QVariant SequencesTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Horizontal) {
        if (section == 0)
            return tr("Speaker ID");
        else if (section == 1)
            return tr("From");
        else if (section == 2)
            return tr("To");
        else if (section == 3)
            return tr("Label");
        else if (section == 4)
            return tr("Sequence Text");
        else if ((section - 5) >= 0 && (section - 5) < d->sequenceAttributes.count()) {
            QString attributeID = d->sequenceAttributes.at(section - 5);
            return attributeID;
        }
        else
            return QVariant();
    }
    else if (orientation == Qt::Vertical) {
        return section + 1;
    }
    return QVariant();
}

QPair<QString, int> SequencesTableModel::modelRowToSpeakerAndSequenceIndex(int row) const
{
    if (row < 0) return QPair<QString, int>(QString(), -1); // invalid index
    int sequenceIndex(row);
    foreach (const QString &speakerID, d->sequenceTiers.keys()) {
        if (sequenceIndex - d->sequenceTiers[speakerID]->count() < 0) {
            return QPair<QString, int>(speakerID, sequenceIndex);
        } else {
            sequenceIndex = sequenceIndex - d->sequenceTiers[speakerID]->count();
        }

    }
    return QPair<QString, int>(QString(), -1);
}

QVariant SequencesTableModel::data(const QModelIndex &index, int role) const
{
    // Sanity checks
    if (!index.isValid()) return false;
    if (index.row() < 0 || index.row() >= rowCount()) return false;
    if (index.column() < 0 || index.column() >= columnCount()) return false;
    // Translate row index to (speakerID, sequence index) pair
    QPair<QString, int> indexPair = modelRowToSpeakerAndSequenceIndex(index.row());
    QString speakerID = indexPair.first;
    int sequenceIndex = indexPair.second;
    // Find the corresponding sequence (and ensure that it exists, i.e. that the model index is valid)
    SequenceTier *tier = d->sequenceTiers.value(speakerID);
    if (!tier) return QVariant();
    Sequence *sequence = tier->sequence(sequenceIndex);
    if (!sequence) return QVariant();
    int attributeIndex = index.column() - 5;
    // Return data
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        if (index.column() == 0) {
            return speakerID;
        }
        else if (index.column() == 1) {
            return sequence->indexFrom();
        }
        else if (index.column() == 2) {
            return sequence->indexTo();
        }
        else if (index.column() == 3) {
            return sequence->text();
        }
        else if (index.column() == 4) {
            return tier->baseTierText(sequenceIndex);
        }
        else if (attributeIndex >= 0 && attributeIndex < d->sequenceAttributes.count()) {
            QString attributeID = d->sequenceAttributes.at(attributeIndex);
            return sequence->attribute(attributeID);
        }
    }
    return QVariant();
}

Qt::ItemFlags SequencesTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;
    // Editable columns: Label and any attributes
    if ((index.column() == 3) || (index.column() >= 5))
        return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
    return QAbstractTableModel::flags(index);
}

bool SequencesTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    // Sanity checks
    if (!index.isValid()) return false;
    if (role != Qt::EditRole) return false;
    if (index.row() < 0 || index.row() >= rowCount()) return false;
    if (index.column() < 0 || index.column() >= columnCount()) return false;
    // Translate row index to (speakerID, sequence index) pair
    QPair<QString, int> indexPair = modelRowToSpeakerAndSequenceIndex(index.row());
    QString speakerID = indexPair.first;
    int sequenceIndex = indexPair.second;
    // Find the corresponding sequence (and ensure that it exists, i.e. that the model index is valid)
    SequenceTier *tier = d->sequenceTiers.value(speakerID);
    if (!tier) return false;
    Sequence *sequence = tier->sequence(sequenceIndex);
    if (!sequence) return false;
    int attributeIndex = index.column() - 5;
    // Update data
    if (data(index, role) != value) {
        if ((index.column() == 0) || (index.column() == 1) || (index.column() == 2) || (index.column() == 4)) {
            return false; // non-editable columns
        }
        else if (index.column() == 3) {
            sequence->setText(value.toString());
        }
        else if (attributeIndex >= 0 && attributeIndex < d->sequenceAttributes.count()) {
            QString attributeID = d->sequenceAttributes.at(attributeIndex);
            sequence->setAttribute(attributeID, value);
        }
        emit dataChanged(index, index, QVector<int>() << role);
        return true;
    }
    return false;
}

bool SequencesTableModel::addSequence(const QString &speakerID, Sequence *sequence)
{
    int row(0);
    foreach (const QString &speakerID_iter, d->sequenceTiers.keys()) {
        if (speakerID_iter == speakerID) {
            int indexOfAddedSequence(0);
            while (indexOfAddedSequence < d->sequenceTiers[speakerID]->count() &&
                   (d->sequenceTiers[speakerID]->at(indexOfAddedSequence)->compare(*sequence) < 0)) {
                indexOfAddedSequence++;
            }
            beginInsertRows(QModelIndex(), row + indexOfAddedSequence, row + indexOfAddedSequence);
            qDebug() << "SequencesTableModel::addSequence calls beginInsertRows with row = " << row + indexOfAddedSequence;
            d->sequenceTiers[speakerID]->addSequence(sequence);
            endInsertRows();
            return true;
        }
        row = row + d->sequenceTiers.value(speakerID_iter)->count();
    }
    return false;
}

bool SequencesTableModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    beginRemoveRows(QModelIndex(), position, position + rows - 1);
    for (int row = 0; row < rows; ++row) {
        QPair<QString, int> indexPair = modelRowToSpeakerAndSequenceIndex(position);
        QString speakerID = indexPair.first; int sequenceIndex = indexPair.second;
        if ((d->sequenceTiers.contains(speakerID)) && (sequenceIndex >= 0) && (sequenceIndex < d->sequenceTiers.value(speakerID)->count())) {
            d->sequenceTiers[speakerID]->removeSequenceAt(sequenceIndex);
        }
    }
    endRemoveRows();
    return true;
}
