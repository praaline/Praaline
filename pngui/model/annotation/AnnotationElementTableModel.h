#ifndef ANNOTATIONTABLEMODEL_H
#define ANNOTATIONTABLEMODEL_H

#include <QAbstractTableModel>
#include "PraalineCore/Annotation/AnnotationElement.h"

struct AnnotationTableModelData;

class AnnotationElementTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit AnnotationElementTableModel(Praaline::Core::AnnotationElement::ElementType elementType,
                                         QList<Praaline::Core::AnnotationElement *> elements,
                                         QStringList attributeIDs, QObject *parent = nullptr);
    ~AnnotationElementTableModel();

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

public slots:
    void modelSavedInDatabase();

private:
    AnnotationTableModelData *d;
};

#endif // ANNOTATIONTABLEMODEL_H
