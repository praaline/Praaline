#include <QPointer>
#include "pncore/structure/NameValueList.h"
using namespace Praaline::Core;
#include "NameValueListTableModel.h"

struct NameValueListTableModelData {
    QPointer<NameValueList> nvl;
};

NameValueListTableModel::NameValueListTableModel(QPointer<Praaline::Core::NameValueList> nvl, QObject *parent) :
    QAbstractTableModel(parent), d(new NameValueListTableModelData)
{
    d->nvl = nvl;
}

NameValueListTableModel::~NameValueListTableModel(){
    delete d;
}

QVariant NameValueListTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0: return tr("Value"); break;
        case 1: return tr("Name"); break;
        default:
            return QVariant();
        }
    }
    return QVariant();
}

int NameValueListTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (!d->nvl) return 0;
    return d->nvl->count();
}

int NameValueListTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (!d->nvl) return 0;
    return 2;
}

QVariant NameValueListTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();
    if (!d->nvl) return QVariant ();
    if (index.row() >= d->nvl->count() || index.row() < 0) return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case 0: return d->nvl->value(index.row());          break;
        case 1: return d->nvl->displayString(index.row());  break;
        default:
            return QVariant();
        }
    }
    return QVariant();
}

bool NameValueListTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) return false;
    if (role != Qt::EditRole) return false;
    if (!d->nvl) return false;
    if (data(index, role) != value) {
        switch (index.column()) {
        case 0:
            if (d->nvl->setValue(index.row(), value)) {
                emit dataChanged(index, index, QVector<int>() << role);
                return true;
            }
            break;
        case 1:
            if (d->nvl->setDisplayString(index.row(), value.toString())) {
                emit dataChanged(index, index, QVector<int>() << role);
                return true;
            }
            break;
        }
    }
    return false;
}

Qt::ItemFlags NameValueListTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) return Qt::NoItemFlags;
    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

bool NameValueListTableModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    if (!d->nvl) return false;
    beginInsertRows(QModelIndex(), position, position + rows - 1);
    for (int row = 0; row < rows; row++) {
        d->nvl->append(QString(), QVariant());
    }
    endInsertRows();
    return true;
}

bool NameValueListTableModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    if (!d->nvl) return false;
    beginRemoveRows(QModelIndex(), position, position + rows - 1);
    for (int row = 0; row < rows; ++row) {
        d->nvl->removeAt(position);
    }
    endRemoveRows();
    return true;
}

