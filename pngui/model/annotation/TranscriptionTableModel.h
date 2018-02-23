#ifndef TRANSCRIPTIONTABLEMODEL_H
#define TRANSCRIPTIONTABLEMODEL_H

#include <QString>
#include <QPointer>
#include <QMap>
#include <QAbstractTableModel>
#include "pncore/base/RealTime.h"
#include "pncore/annotation/AnnotationTierGroup.h"

struct TranscriptionTableModelData;

class TranscriptionTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit TranscriptionTableModel(QObject *parent = nullptr);
    ~TranscriptionTableModel();

    QModelIndex parent(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

signals:

public slots:

private:
    TranscriptionTableModelData *d;
};

#endif // TRANSCRIPTIONTABLEMODEL_H
