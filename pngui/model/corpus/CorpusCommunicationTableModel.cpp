#include "pncore/corpus/Corpus.h"
#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/structure/MetadataStructure.h"
using namespace Praaline::Core;

#include "CorpusCommunicationTableModel.h"

struct CorpusCommunicationTableModelData {
    CorpusCommunicationTableModelData() : multiCorpus(false) {}

    bool multiCorpus;
    QList<QPointer<CorpusCommunication> > items;
    QList<QPointer<MetadataStructureAttribute> > attributes;
    QPointer<Corpus> corpus;
};

CorpusCommunicationTableModel::CorpusCommunicationTableModel(QList<QPointer<CorpusCommunication> > items,
                                                             QList<QPointer<MetadataStructureAttribute> > attributes,
                                                             QPointer<Corpus> corpus, bool multiCorpus, QObject *parent)
    : QAbstractTableModel(parent), d(new CorpusCommunicationTableModelData)
{
    d->items = items;
    d->attributes = attributes;
    d->corpus = corpus;
    d->multiCorpus = multiCorpus;
}

CorpusCommunicationTableModel::~CorpusCommunicationTableModel()
{
    delete d;
}

QModelIndex CorpusCommunicationTableModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int CorpusCommunicationTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return d->items.count();
}

int CorpusCommunicationTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    int basicPropertiesCount = (d->multiCorpus) ? 4 : 3;
    return basicPropertiesCount + d->attributes.count();
}

QVariant CorpusCommunicationTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Horizontal) {
        if (d->multiCorpus) {
            if (section == 0) return tr("Corpus ID");
            else if (section == 1) return tr("Communication ID");
            else if (section == 2) return tr("Name");
            else if (section == 3) return tr("Duration");
            else if ((section - 4) >= 0 && (section - 4) < d->attributes.count()) {
                QPointer<MetadataStructureAttribute> attr = d->attributes.at(section - 4);
                if (attr) return attr->name(); else return QVariant();
            }
            else
                return QVariant();
        } else {
            if (section == 0) return tr("Communication ID");
            else if (section == 1) return tr("Name");
            else if (section == 2) return tr("Duration");
            else if ((section - 3) >= 0 && (section - 3) < d->attributes.count()) {
                QPointer<MetadataStructureAttribute> attr = d->attributes.at(section - 3);
                if (attr) return attr->name(); else return QVariant();
            }
            else
                return QVariant();
        }
    }
    return QVariant();
}

QVariant CorpusCommunicationTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (index.row() >= d->items.count() || index.row() < 0)
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        QPointer<CorpusCommunication> com = d->items.at(index.row());
        if (!com) return QVariant();
        if (d->multiCorpus) {
            if (index.column() == 0) return com->property("corpusID");
            else if (index.column() == 1) return com->property("ID");
            else if (index.column() == 2) return com->property("name");
            else if (index.column() == 3) return com->property("durationSec");
            else if ((index.column() - 4) >= 0 && (index.column() - 4) < d->attributes.count())
                return com->property(d->attributes.at(index.column() - 4)->ID());
            else
                return QVariant();
        } else {
            if (index.column() == 0) return com->property("ID");
            else if (index.column() == 1) return com->property("name");
            else if (index.column() == 2) return com->property("durationSec");
            else if ((index.column() - 3) >= 0 && (index.column() - 3) < d->attributes.count())
                return com->property(d->attributes.at(index.column() - 3)->ID());
            else
                return QVariant();
        }
    }
    return QVariant();
}


Qt::ItemFlags CorpusCommunicationTableModel::flags(const QModelIndex &index) const
{
    if (index.column() < columnCount())
        return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    return QAbstractTableModel::flags(index);
}

bool CorpusCommunicationTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;
    if (index.row() >= d->items.count() || index.row() < 0)
        return false;

    if (role == Qt::EditRole) {
        QPointer<CorpusCommunication> com = d->items.at(index.row());
        if (!com) return false;
        if (d->multiCorpus) {
            if      (index.column() == 0) return com->setProperty("corpusID", value);
            else if (index.column() == 1) return com->setProperty("ID", value);
            else if (index.column() == 2) return com->setProperty("name", value);
            else if ((index.column() - 4) >= 0 && (index.column() - 4) < d->attributes.count())
                return com->setProperty(d->attributes.at(index.column() - 4)->ID(), value);
            else
                return false;
        } else {
            if      (index.column() == 0) return com->setProperty("ID", value);
            else if (index.column() == 1) return com->setProperty("name", value);
            else if ((index.column() - 3) >= 0 && (index.column() - 3) < d->attributes.count())
                return com->setProperty(d->attributes.at(index.column() - 3)->ID(), value);
            else
                return false;
        }
    }
    return QAbstractTableModel::setData(index, value, role);
}

bool CorpusCommunicationTableModel::insertRows(int position, int rows, const QModelIndex &index)
{
    return false;
}

bool CorpusCommunicationTableModel::removeRows(int position, int rows, const QModelIndex &index)
{
    return false;
}

