#include "TranscriptionTableModel.h"

struct TranscriptionTableModelData {
    int i;
};

TranscriptionTableModel::TranscriptionTableModel(QObject *parent) :
    QAbstractTableModel(parent), d(new TranscriptionTableModelData())
{

}

TranscriptionTableModel::~TranscriptionTableModel()
{
    delete d;
}

QModelIndex TranscriptionTableModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QModelIndex();
}

QVariant TranscriptionTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section)
    Q_UNUSED(orientation)
    Q_UNUSED(role)
    return QVariant();
}

int TranscriptionTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 0;
}

int TranscriptionTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 0;
}

QVariant TranscriptionTableModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(index)
    Q_UNUSED(role)
    return QVariant();
}

bool TranscriptionTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(index)
    Q_UNUSED(value)
    Q_UNUSED(role)
    return false;
}

Qt::ItemFlags TranscriptionTableModel::flags(const QModelIndex& index) const
{
    Q_UNUSED(index)
    return Qt::ItemFlags();
}

