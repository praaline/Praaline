
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/annotation/IntervalTier.h"
using namespace Praaline::Core;

#include "PhonetisationTableModel.h"

struct PhonetisationTableModelData {
    PhonetisationTableModelData() :
        attributeOrthographic(""), attributePhonetisation("phonetisation")
    {}

    QMap<QString, QPointer<AnnotationTierGroup> > tiers;
    QString tiernameTokens;
    QString attributeOrthographic;
    QString attributePhonetisation;
    QString speakerID;
    int indexStart;
    int indexEnd;
};

PhonetisationTableModel::PhonetisationTableModel(QMap<QString, QPointer<Praaline::Core::AnnotationTierGroup> > &tiers,
                                                 const QString &tiernameTokens,
                                                 const QString &attributeOrthographic, const QString &attributePhonetisation,
                                                 QObject *parent) :
    QAbstractTableModel(parent), d(new PhonetisationTableModelData)
{
}

PhonetisationTableModel::~PhonetisationTableModel()
{
    delete d;
}

QVariant PhonetisationTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    // FIXME: Implement me!
}

int PhonetisationTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    // FIXME: Implement me!
}

int PhonetisationTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    // FIXME: Implement me!
}

QVariant PhonetisationTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    // FIXME: Implement me!
    return QVariant();
}

bool PhonetisationTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (data(index, role) != value) {
        // FIXME: Implement me!
        emit dataChanged(index, index, QVector<int>() << role);
        return true;
    }
    return false;
}

Qt::ItemFlags PhonetisationTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEditable; // FIXME: Implement me!
}
