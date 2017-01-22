#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>

#include "pncore/corpus/Corpus.h"
#include "pncore/corpus/CorpusAnnotation.h"
#include "pncore/structure/MetadataStructure.h"
using namespace Praaline::Core;

#include "CorpusAnnotationTableModel.h"

struct CorpusAnnotationTableModelData {
    bool multiCommunication;
    QList<QPointer<CorpusAnnotation> > items;
    QList<QPointer<MetadataStructureAttribute> > attributes;
};

CorpusAnnotationTableModel::CorpusAnnotationTableModel(QList<QPointer<CorpusAnnotation> > items,
                                                       QList<QPointer<MetadataStructureAttribute> > attributes,
                                                       bool multiCommunication, QObject *parent) :
    QAbstractTableModel(parent), d(new CorpusAnnotationTableModelData)
{
    d->items = items;
    d->attributes = attributes;
    d->multiCommunication = multiCommunication;
}

CorpusAnnotationTableModel::~CorpusAnnotationTableModel()
{
    delete d;
}

QModelIndex CorpusAnnotationTableModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int CorpusAnnotationTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return d->items.count();
}

int CorpusAnnotationTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    int basicPropertiesCount = 3;
    return basicPropertiesCount + d->attributes.count();
}

QVariant CorpusAnnotationTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Horizontal) {
        if (section == 0) return tr("Communication ID");
        else if (section == 1) return tr("Annotation ID");
        else if (section == 2) return tr("Name");
        else if ((section - 3) >= 0 && (section - 3) < d->attributes.count())
            return d->attributes.at(section - 3)->name();
        else
            return QVariant();
    }
    else if (orientation == Qt::Vertical) {
        QString ret = QString::number(section + 1);
        QPointer<CorpusAnnotation> item(0);
        if ((section >= 0) && (section < d->items.count())) {
            item  = d->items.at(section);
            if (item->isNew()) return ret.append(" +");
            if (item->isDirty()) return ret.append(" #");
        }
        return ret;
    }
    return QVariant();
}

QVariant CorpusAnnotationTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (index.row() >= d->items.count() || index.row() < 0)
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        QPointer<CorpusAnnotation> annot = d->items.at(index.row());
        if (!annot) return QVariant();
        if (index.column() == 0) return annot->property("communicationID");
        else if (index.column() == 1) return annot->property("ID");
        else if (index.column() == 2) return annot->property("name");
        else if ((index.column() - 3) >= 0 && (index.column() - 3) < d->attributes.count())
            return annot->property(d->attributes.at(index.column() - 3)->ID());
        else
            return QVariant();

    }
    return QVariant();
}


Qt::ItemFlags CorpusAnnotationTableModel::flags(const QModelIndex &index) const
{
    if (index.column() < columnCount())
        return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    return QAbstractTableModel::flags(index);
}

bool CorpusAnnotationTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;
    if (index.row() >= d->items.count() || index.row() < 0)
        return false;

    if (role == Qt::EditRole) {
        QPointer<CorpusAnnotation> annot = d->items.at(index.row());
        if (!annot) return false;
        // communicationID is read-only
        if      (index.column() == 1) return annot->setProperty("ID", value);
        else if (index.column() == 2) return annot->setProperty("name", value);
        else if ((index.column() - 3) >= 0 && (index.column() - 3) < d->attributes.count())
            return annot->setProperty(d->attributes.at(index.column() - 3)->ID(), value);
        else
            return false;

    }
    return QAbstractTableModel::setData(index, value, role);
}

bool CorpusAnnotationTableModel::insertRows(int position, int rows, const QModelIndex &index)
{
    return false;
}

bool CorpusAnnotationTableModel::removeRows(int position, int rows, const QModelIndex &index)
{
    return false;
}

