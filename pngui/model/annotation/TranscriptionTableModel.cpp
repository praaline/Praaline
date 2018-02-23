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

}

QVariant TranscriptionTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{

}

int TranscriptionTableModel::rowCount(const QModelIndex &parent) const
{

}

int TranscriptionTableModel::columnCount(const QModelIndex &parent) const
{

}

QVariant TranscriptionTableModel::data(const QModelIndex &index, int role) const
{

}

bool TranscriptionTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{

}

Qt::ItemFlags TranscriptionTableModel::flags(const QModelIndex& index) const
{

}

