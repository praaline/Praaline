#ifndef CORPUSEXPLORERTREEMODEL_H
#define CORPUSEXPLORERTREEMODEL_H

#include <QObject>
#include <QPointer>
#include <QAbstractItemModel>
#include <QItemSelection>

#include "QtilitiesCore/QtilitiesCore"
#include "QtilitiesCoreGui/QtilitiesCoreGui"
using namespace Qtilities;
using namespace QtilitiesCore;
using namespace QtilitiesCoreGui;

namespace Praaline {
namespace Core {
class MetadataStructureAttribute;
}
}

struct CorpusExplorerTreeModelData;

class CorpusExplorerTreeModel : public ObserverTreeModel
{
    Q_OBJECT
public:
    CorpusExplorerTreeModel(QList<QPointer<Praaline::Core::MetadataStructureAttribute> > attributes,
                            QObject *parent = 0);
    ~CorpusExplorerTreeModel();

    // ObserverTableModel virtual method overrides
    int columnPosition(AbstractObserverItemModel::ColumnID column_id) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);

signals:

public slots:

private:
    CorpusExplorerTreeModelData *d;
};

#endif // CORPUSEXPLORERTREEMODEL_H
