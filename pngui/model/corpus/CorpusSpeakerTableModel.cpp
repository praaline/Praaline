#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>

#include "pncore/corpus/Corpus.h"
#include "pncore/corpus/CorpusSpeaker.h"
#include "pncore/structure/MetadataStructure.h"
using namespace Praaline::Core;

#include "CorpusSpeakerTableModel.h"

struct CorpusSpeakerTableModelData {
    bool multiCorpus;
    QList<QPointer<CorpusSpeaker> > items;
    QList<QPointer<MetadataStructureAttribute> > attributes;
    QPointer<Corpus> corpus;
};

CorpusSpeakerTableModel::CorpusSpeakerTableModel(QList<QPointer<CorpusSpeaker> > items,
                                                 QList<QPointer<MetadataStructureAttribute> > attributes,
                                                 QPointer<Corpus> corpus, bool multiCorpus, QObject *parent)
    : QAbstractTableModel(parent), d(new CorpusSpeakerTableModelData)
{
    d->items = items;
    d->attributes = attributes;
    d->corpus = corpus;
    d->multiCorpus = multiCorpus;
}

CorpusSpeakerTableModel::~CorpusSpeakerTableModel()
{
    delete d;
}

QModelIndex CorpusSpeakerTableModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int CorpusSpeakerTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return d->items.count();
}

int CorpusSpeakerTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    int basicPropertiesCount = (d->multiCorpus) ? 3 : 2;
    return basicPropertiesCount + d->attributes.count();
}

QVariant CorpusSpeakerTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Horizontal) {
        if (d->multiCorpus) {
            if (section == 0) return tr("Corpus ID");
            else if (section == 1) return tr("Speaker ID");
            else if (section == 2) return tr("Name");
            else if ((section - 3) >= 0 && (section - 3) < d->attributes.count()) {
                QPointer<MetadataStructureAttribute> attr = d->attributes.at(section - 3);
                return (attr) ? attr->name() : QVariant();
            } else
                return QVariant();
        } else {
            if (section == 0) return tr("Speaker ID");
            else if (section == 1) return tr("Name");
            else if ((section - 2) >= 0 && (section - 2) < d->attributes.count())
                return d->attributes.at(section - 2)->name();
            else
                return QVariant();
        }
    }
    return QVariant();
}

QVariant CorpusSpeakerTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (index.row() >= d->items.count() || index.row() < 0)
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        QPointer<CorpusSpeaker> spk = d->items.at(index.row());
        if (!spk) return QVariant();
        if (d->multiCorpus) {
            if (index.column() == 0) return spk->property("corpusID");
            else if (index.column() == 1) return spk->property("ID");
            else if (index.column() == 2) return spk->property("name");
            else if ((index.column() - 3) >= 0 && (index.column() - 3) < d->attributes.count())
                return spk->property(d->attributes.at(index.column() - 3)->ID());
            else
                return QVariant();
        } else {
            if (index.column() == 0) return spk->property("ID");
            else if (index.column() == 1) return spk->property("name");
            else if ((index.column() - 2) >= 0 && (index.column() - 2) < d->attributes.count())
                return spk->property(d->attributes.at(index.column() - 2)->ID());
            else
                return QVariant();
        }
    }
    return QVariant();
}


Qt::ItemFlags CorpusSpeakerTableModel::flags(const QModelIndex &index) const
{
    if (index.column() < columnCount())
        return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    return QAbstractTableModel::flags(index);
}

bool CorpusSpeakerTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;
    if (index.row() >= d->items.count() || index.row() < 0)
        return false;

    if (role == Qt::EditRole) {
        QPointer<CorpusSpeaker> spk = d->items.at(index.row());
        if (!spk) return false;
        if (d->multiCorpus) {
            if      (index.column() == 0) return spk->setProperty("corpusID", value);
            else if (index.column() == 1) return spk->setProperty("ID", value);
            else if (index.column() == 2) return spk->setProperty("name", value);
            else if ((index.column() - 3) >= 0 && (index.column() - 3) < d->attributes.count())
                return spk->setProperty(d->attributes.at(index.column() - 3)->ID(), value);
            else
                return false;
        } else {
            if      (index.column() == 0) return spk->setProperty("ID", value);
            else if (index.column() == 1) return spk->setProperty("name", value);
            else if ((index.column() - 2) >= 0 && (index.column() - 2) < d->attributes.count())
                return spk->setProperty(d->attributes.at(index.column() - 2)->ID(), value);
            else
                return false;
        }
    }
    return QAbstractTableModel::setData(index, value, role);
}

bool CorpusSpeakerTableModel::insertRows(int position, int rows, const QModelIndex &index)
{
    return false;
}

bool CorpusSpeakerTableModel::removeRows(int position, int rows, const QModelIndex &index)
{
    return false;
}

