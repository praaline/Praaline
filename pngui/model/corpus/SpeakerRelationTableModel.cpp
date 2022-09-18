#include <QObject>
#include <QPointer>
#include <QString>
#include <QAbstractTableModel>
#include <QDebug>

#include "PraalineCore/Corpus/CorpusCommunication.h"
#include "PraalineCore/Corpus/CorpusCommunicationSpeakerRelation.h"
using namespace Praaline::Core;

#include "SpeakerRelationTableModel.h"

struct SpeakerRelationTableModelData {
    QPointer<CorpusCommunication> com;

    SpeakerRelationTableModelData()
    {}
};


SpeakerRelationTableModel::SpeakerRelationTableModel(Praaline::Core::CorpusCommunication *com, QObject *parent) :
    QAbstractTableModel(parent), d(new SpeakerRelationTableModelData())
{
    d->com = QPointer<CorpusCommunication>(com);
}

SpeakerRelationTableModel::~SpeakerRelationTableModel()
{
    delete d;
}

// Table model implementation
QModelIndex SpeakerRelationTableModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int SpeakerRelationTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    if (d->com)
        return d->com->speakerRelations().count();
    return 0;
}

int SpeakerRelationTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return 4;
}

QVariant SpeakerRelationTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Horizontal) {
        if (section == 0)
            return tr("Speaker ID 1");
        else if (section == 1)
            return tr("Speaker ID 2");
        else if (section == 2)
            return tr("Relation");
        else if (section == 3)
            return tr("Notes");
        else
            return QVariant();
    }
    else if (orientation == Qt::Vertical) {
        return section + 1;
    }
    return QVariant();
}

Qt::ItemFlags SpeakerRelationTableModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;
    // Editable columns: all
    if ((index.column() >= 0) || (index.column() <= 3))
        return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
    return QAbstractTableModel::flags(index);
}

QVariant SpeakerRelationTableModel::data(const QModelIndex &index, int role) const
{
    // Sanity checks
    if (!index.isValid()) return false;
    if (index.row() < 0 || index.row() >= rowCount()) return false;
    if (index.column() < 0 || index.column() >= columnCount()) return false;
    if (!d->com) return false;
    // Find the corresponding relation
    const CorpusCommunicationSpeakerRelation &relation = d->com->speakerRelations().at(index.row());
    // Return data
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        if (index.column() == 0) {
            return relation.speakerID_1();
        }
        else if (index.column() == 1) {
            return relation.speakerID_2();
        }
        else if (index.column() == 2) {
            return relation.relation();
        }
        else if (index.column() == 3) {
            return relation.notes();
        }
    }
    return QVariant();
}

bool SpeakerRelationTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    // Sanity checks
    if (!index.isValid()) return false;
    if (role != Qt::EditRole) return false;
    if (index.row() < 0 || index.row() >= rowCount()) return false;
    if (index.column() < 0 || index.column() >= columnCount()) return false;
    if (!d->com) return false;
    // Find the corresponding relation
    const CorpusCommunicationSpeakerRelation &existing_relation = d->com->speakerRelations().at(index.row());
    // Change data, if needed
    if (data(index, role) != value) {
        if (index.column() == 0) {
            CorpusCommunicationSpeakerRelation new_relation =
                    CorpusCommunicationSpeakerRelation(value.toString(), existing_relation.speakerID_2(),
                                                       existing_relation.relation(), existing_relation.notes());
            d->com->speakerRelations()[index.row()] = new_relation;
        }
        else if (index.column() == 1) {
            CorpusCommunicationSpeakerRelation new_relation =
                    CorpusCommunicationSpeakerRelation(existing_relation.speakerID_1(), value.toString(),
                                                       existing_relation.relation(), existing_relation.notes());
            d->com->speakerRelations()[index.row()] = new_relation;
        }
        else if (index.column() == 2) {
            d->com->speakerRelations()[index.row()].setRelation(value.toString());
        }
        else if (index.column() == 3) {
            d->com->speakerRelations()[index.row()].setNotes(value.toString());
        }
        emit dataChanged(index, index, QVector<int>() << role);
        return true;
    }
    return false;
}

// Add and remove relations
bool SpeakerRelationTableModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    if (!d->com) return false;
    beginInsertRows(QModelIndex(), position, position + rows - 1);
    for (int row = 0; row < rows; row++) {
        // Create fictitious unique speaker IDs
        int speakerID_1 = 1, speakerID_2 = 2;
        while (d->com->hasSpeakerRelation(QString("S%1").arg(speakerID_1), QString("S%1").arg(speakerID_2))) {
            speakerID_1++; speakerID_2++;
            if (speakerID_1 > 10000) return false; // something is wrong, it's not normal to have so many speakers
            if (speakerID_2 > 10000) return false; // something is wrong, it's not normal to have so many speakers
        }
        d->com->insertSpeakerRelation(CorpusCommunicationSpeakerRelation(QString("S%1").arg(speakerID_1),
                                                                         QString("S%1").arg(speakerID_2)));
    }
    endInsertRows();
    return true;
}

bool SpeakerRelationTableModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    if (!d->com) return false;
    beginRemoveRows(QModelIndex(), position, position + rows - 1);
    for (int row = 0; row < rows; ++row) {
        d->com->speakerRelations().removeAt(position);
    }
    endRemoveRows();
    return true;
}
