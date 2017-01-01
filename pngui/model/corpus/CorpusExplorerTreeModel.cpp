#include "pncore/structure/MetadataStructureAttribute.h"
#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/corpus/CorpusSpeaker.h"
using namespace Praaline::Core;

#include "observers/CorpusObserver.h"
#include "CorpusExplorerTreeModel.h"

struct CorpusExplorerTreeModelData {
    QList<QPointer<MetadataStructureAttribute> > attributes;
};


CorpusExplorerTreeModel::CorpusExplorerTreeModel(QList<QPointer<MetadataStructureAttribute> > attributes, QObject *parent) :
    ObserverTreeModel(parent), d(new CorpusExplorerTreeModelData)
{
    d->attributes = attributes;
}

CorpusExplorerTreeModel::~CorpusExplorerTreeModel()
{
    delete d;
}

int CorpusExplorerTreeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 2 + d->attributes.count();
}

int CorpusExplorerTreeModel::columnPosition(AbstractObserverItemModel::ColumnID column_id) const
{
    switch (column_id) {
    case ColumnSubjectID: return 1; break;
    case ColumnName: return 0; break;
    case ColumnChildCount: return -1; break;
    case ColumnAccess: return -1; break;
    case ColumnTypeInfo: return -1; break;
    case ColumnCategory: return -1; break;
    case ColumnLast: return 2 + d->attributes.count(); break;
    }
    return -1;
}

QVariant CorpusExplorerTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Horizontal) {
        if (section == 0) return tr("ID");
        else if (section == 1) return tr("Name");
        else if ((section - 2) >= 0 && (section - 2) < d->attributes.count())
            return d->attributes.at(section - 2)->name();
        else
            return QVariant();
    }
    return QVariant();
}

QVariant CorpusExplorerTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();

    QObject *obj = getObject(index);
    if (!obj) return ObserverTreeModel::data(index, role);

    CorpusExplorerTreeNodeCommunication *nodeCom = qobject_cast<CorpusExplorerTreeNodeCommunication *>(obj);
    if ((nodeCom) && (role == Qt::DisplayRole || role == Qt::EditRole)) {
        if (!(nodeCom->communication))
            return QVariant();
        if (index.column() == 0) return nodeCom->communication->property("ID");
        else if (index.column() == 1) return nodeCom->communication->property("name");
        else if ((index.column() - 2) >= 0 && (index.column() - 2) < d->attributes.count())
            return nodeCom->communication->property(d->attributes.at(index.column() - 2)->ID());
        else
            return QVariant();
    }
    CorpusExplorerTreeNodeSpeaker *nodeSpk = qobject_cast<CorpusExplorerTreeNodeSpeaker *>(obj);
    if ((nodeSpk) && (role == Qt::DisplayRole || role == Qt::EditRole)) {
        if (!(nodeSpk->speaker))
            return QVariant();
        if (index.column() == 0) return nodeSpk->speaker->property("ID");
        else if (index.column() == 1) return nodeSpk->speaker->property("name");
        else if ((index.column() - 2) >= 0 && (index.column() - 2) < d->attributes.count())
            return nodeSpk->speaker->property(d->attributes.at(index.column() - 2)->ID());
        else
            return QVariant();
    }
    return ObserverTreeModel::data(index, role);
}

Qt::ItemFlags CorpusExplorerTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;
    if (index.column() < columnCount())
        return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    return ObserverTreeModel::flags(index);
}

bool CorpusExplorerTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) return false;

    QObject *obj = getObject(index);
    if (!obj) return ObserverTreeModel::setData(index, value, role);

    CorpusExplorerTreeNodeCommunication *nodeCom = qobject_cast<CorpusExplorerTreeNodeCommunication *>(obj);
    if ((nodeCom) && (role == Qt::EditRole)) {
        if (!(nodeCom->communication))
            return false;
        if (index.column() == 0) return nodeCom->communication->setProperty("ID", value);
        else if (index.column() == 1) return nodeCom->communication->setProperty("name", value);
        else if ((index.column() - 2) >= 0 && (index.column() - 2) < d->attributes.count())
            return nodeCom->communication->setProperty(d->attributes.at(index.column() - 2)->ID(), value);
        else
            return false;
    }
    CorpusExplorerTreeNodeSpeaker *nodeSpk = qobject_cast<CorpusExplorerTreeNodeSpeaker *>(obj);
    if ((nodeSpk) && (role == Qt::EditRole)) {
        if (!(nodeSpk->speaker))
            return false;
        if (index.column() == 0) return nodeSpk->speaker->setProperty("ID", value);
        else if (index.column() == 1) return nodeSpk->speaker->setProperty("name", value);
        else if ((index.column() - 2) >= 0 && (index.column() - 2) < d->attributes.count())
            return nodeSpk->speaker->setProperty(d->attributes.at(index.column() - 2)->ID(), value);
        else
            return false;
    }
    return ObserverTreeModel::setData(index, value, role);
}

