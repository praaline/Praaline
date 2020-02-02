#ifndef PHONETISATIONTABLEMODEL_H
#define PHONETISATIONTABLEMODEL_H

#include <QString>
#include <QPointer>
#include <QMap>
#include <QAbstractTableModel>
#include "PraalineCore/Base/RealTime.h"
#include "PraalineCore/Annotation/AnnotationTierGroup.h"

struct PhonetisationTableModelData;

class PhonetisationTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit PhonetisationTableModel(Praaline::Core::SpeakerAnnotationTierGroupMap &tiers,
                                     const QString &tiernameTokens,
                                     const QString &attributeOrthographic, const QString &attributePhonetisation,
                                     QObject *parent = nullptr);
    ~PhonetisationTableModel();

    QModelIndex parent(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    bool setUtterance(const QString &speakerID, int indexStart, int indexEnd);

private:
    PhonetisationTableModelData *d;
};

#endif // PHONETISATIONTABLEMODEL_H
