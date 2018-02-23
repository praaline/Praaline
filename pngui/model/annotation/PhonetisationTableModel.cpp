
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/annotation/IntervalTier.h"
using namespace Praaline::Core;

#include "PhonetisationTableModel.h"

struct PhonetisationTableModelData {
    PhonetisationTableModelData() :
        attributeOrthographic(""), attributePhonetisation("phonetisation"),
        indexStart(-1), indexEnd(-1)
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
    d->tiers = tiers;
    d->tiernameTokens = tiernameTokens;
    d->attributeOrthographic = attributeOrthographic;
    d->attributePhonetisation = attributePhonetisation;
    // No utterance selected yet
    d->indexStart = d->indexEnd = -1;
}

PhonetisationTableModel::~PhonetisationTableModel()
{
    delete d;
}

QModelIndex PhonetisationTableModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int PhonetisationTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 2;
}

int PhonetisationTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if ((d->indexStart < 0) || (d->indexEnd < 0)) return 0;
    if (d->indexEnd < d->indexStart) return 0;
    return d->indexEnd - d->indexStart;
}

QVariant PhonetisationTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Horizontal) {
        return section + 1;
    }
    if (orientation == Qt::Vertical) {
        if (section == 0) return tr("Orthographic");
        if (section == 1) return tr("Phonetisation");
    }
    return QVariant();
}

QVariant PhonetisationTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    QPointer<AnnotationTierGroup> spk_tiers = d->tiers.value(d->speakerID, 0);
    if (!spk_tiers) return QVariant();
    IntervalTier *tier_tokens = spk_tiers->getIntervalTierByName(d->tiernameTokens);
    if (!tier_tokens) return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        int intervalIndex = d->indexStart + index.column();
        if ((intervalIndex < 0) || (intervalIndex >= tier_tokens->count())) return QVariant();
        Interval *token = tier_tokens->at(intervalIndex);
        if (index.row() == 0) {
            return (d->attributeOrthographic.isEmpty()) ? token->text() : token->attribute(d->attributeOrthographic).toString();
        }
        else if (index.row() == 1) {
            return (d->attributePhonetisation.isEmpty()) ? token->text() : token->attribute(d->attributePhonetisation).toString();
        }
    }
    return QVariant();
}

bool PhonetisationTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) return false;
    if (role != Qt::EditRole) return false;
    if (index.row() < 0 || index.row() >= rowCount()) return false;
    if (index.column() < 0 || index.column() >= columnCount()) return false;
    if (data(index, role) == value) return false;

    QPointer<AnnotationTierGroup> spk_tiers = d->tiers.value(d->speakerID, 0);
    if (!spk_tiers) return false;
    IntervalTier *tier_tokens = spk_tiers->getIntervalTierByName(d->tiernameTokens);
    if (!tier_tokens) return false;

    int intervalIndex = d->indexStart + index.column();
    if ((intervalIndex < 0) || (intervalIndex >= tier_tokens->count())) return false;
    Interval *token = tier_tokens->at(intervalIndex);
    if (index.row() == 0) {
        if (d->attributeOrthographic.isEmpty())
            token->setText(value.toString());
        else
            token->setAttribute(d->attributeOrthographic, value);
        emit dataChanged(index, index, QVector<int>() << role);
        return true;
    }
    else if (index.row() == 1) {
        if (d->attributePhonetisation.isEmpty())
            token->setText(value.toString());
        else
            token->setAttribute(d->attributePhonetisation, value);
        emit dataChanged(index, index, QVector<int>() << role);
        return true;
    }
    return false;
}

Qt::ItemFlags PhonetisationTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;
    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

bool PhonetisationTableModel::setUtterance(const QString &speakerID, int indexStart, int indexEnd)
{
    QPointer<AnnotationTierGroup> spk_tiers = d->tiers.value(speakerID, 0);
    if (!spk_tiers) return false;
    IntervalTier *tier_tokens = spk_tiers->getIntervalTierByName(d->tiernameTokens);
    if (!tier_tokens) return false;
    if ((indexStart < 0) || (indexStart >= tier_tokens->count())) return false;
    if ((indexEnd < 0) || (indexEnd >= tier_tokens->count())) return false;
    emit beginResetModel();
    d->speakerID = speakerID;
    d->indexStart = indexStart;
    d->indexEnd = indexEnd;
    emit endResetModel();
    return true;
}
