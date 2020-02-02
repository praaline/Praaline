#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>

#include "PraalineCore/Corpus/Corpus.h"
#include "PraalineCore/Corpus/CorpusRecording.h"
#include "PraalineCore/Structure/MetadataStructure.h"
using namespace Praaline::Core;

#include "CorpusRecordingTableModel.h"

struct CorpusRecordingTableModelData {
    bool multiCommunication;
    QList<CorpusRecording *> items;
    QList<MetadataStructureAttribute *> attributes;
};

CorpusRecordingTableModel::CorpusRecordingTableModel(QList<CorpusRecording *> items,
                                                     QList<MetadataStructureAttribute *> attributes,
                                                     bool multiCommunication, QObject *parent)
    : QAbstractTableModel(parent), d(new CorpusRecordingTableModelData)
{
    d->items = items;
    d->attributes = attributes;
    d->multiCommunication = multiCommunication;
}

CorpusRecordingTableModel::~CorpusRecordingTableModel()
{
    delete d;
}

QModelIndex CorpusRecordingTableModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int CorpusRecordingTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return d->items.count();
}

int CorpusRecordingTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    int basicPropertiesCount = 12;
    return basicPropertiesCount + d->attributes.count();
}

QVariant CorpusRecordingTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Horizontal) {
        if (section == 0) return tr("Communication ID");
        else if (section == 1) return tr("Recording ID");
        else if (section == 2) return tr("Name");
        else if (((section - 3) >= 0) && ((section - 3) < d->attributes.count()))
            return d->attributes.at(section - 3)->name();
        else if ((section - d->attributes.count()) == 3) return tr("File name");
        else if ((section - d->attributes.count()) == 4) return tr("Format");
        else if ((section - d->attributes.count()) == 5) return tr("Duration");
        else if ((section - d->attributes.count()) == 6) return tr("Channels");
        else if ((section - d->attributes.count()) == 7) return tr("Sample rate");
        else if ((section - d->attributes.count()) == 8) return tr("Precision (bits)");
        else if ((section - d->attributes.count()) == 9) return tr("Bitrate");
        else if ((section - d->attributes.count()) == 10) return tr("File size");
        else if ((section - d->attributes.count()) == 11) return tr("Checksum MD5");
        else
            return QVariant();
    }
    else if (orientation == Qt::Vertical) {
        QString ret = QString::number(section + 1);
        CorpusRecording *item(0);
        if ((section >= 0) && (section < d->items.count())) {
            item  = d->items.at(section);
            if (item->isNew()) return ret.append(" +");
            if (item->isDirty()) return ret.append(" #");
        }
        return ret;
    }
    return QVariant();
}

QVariant CorpusRecordingTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (index.row() >= d->items.count() || index.row() < 0)
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        CorpusRecording *rec = d->items.at(index.row());
        if (!rec) return QVariant();
        if      (index.column() == 0) return rec->property("communicationID");
        else if (index.column() == 1) return rec->property("ID");
        else if (index.column() == 2) return rec->property("name");
        else if (((index.column() - 3) >= 0) && ((index.column() - 3) < d->attributes.count()))
            return rec->property(d->attributes.at(index.column() - 3)->ID());
        else if ((index.column() - d->attributes.count()) == 3) return rec->property("filename");
        else if ((index.column() - d->attributes.count()) == 4) return rec->property("format");
        else if ((index.column() - d->attributes.count()) == 5) return rec->property("durationSec");
        else if ((index.column() - d->attributes.count()) == 6) return rec->property("channels");
        else if ((index.column() - d->attributes.count()) == 7) return rec->property("sampleRate");
        else if ((index.column() - d->attributes.count()) == 8) return rec->property("precisionBits");
        else if ((index.column() - d->attributes.count()) == 9) return rec->property("bitRate");
        else if ((index.column() - d->attributes.count()) == 10) return rec->property("fileSize");
        else if ((index.column() - d->attributes.count()) == 11) return rec->property("checksumMD5");
        else
            return QVariant();
    }
    return QVariant();
}


Qt::ItemFlags CorpusRecordingTableModel::flags(const QModelIndex &index) const
{
    if (index.column() < columnCount())
        return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    return QAbstractTableModel::flags(index);
}

bool CorpusRecordingTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;
    if (index.row() >= d->items.count() || index.row() < 0)
        return false;

    if (role == Qt::EditRole) {
        CorpusRecording *rec = d->items.at(index.row());
        if (!rec) return false;
        // communicationID is read-only
        if      (index.column() == 1) return rec->setProperty("ID", value);
        else if (index.column() == 2) return rec->setProperty("name", value);
        else if ((index.column() - 3) >= 0 && (index.column() - 3) < d->attributes.count())
            return rec->setProperty(d->attributes.at(index.column() - 3)->ID(), value);
        else if ((index.column() - d->attributes.count()) == 3)
            return rec->setProperty("fileName", value);
        else
            return false; // the rest of sound file properties are read-only

    }
    return QAbstractTableModel::setData(index, value, role);
}

bool CorpusRecordingTableModel::insertRows(int position, int rows, const QModelIndex &index)
{
    return false;
}

bool CorpusRecordingTableModel::removeRows(int position, int rows, const QModelIndex &index)
{
    return false;
}

