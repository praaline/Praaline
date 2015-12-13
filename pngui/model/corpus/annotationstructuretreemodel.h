#ifndef ANNOTATIONSTRUCTURETREEMODEL_H
#define ANNOTATIONSTRUCTURETREEMODEL_H

#include <QObject>
#include <QList>
#include <QPair>
#include <QAbstractItemModel>
#include "pncore/structure/annotationstructure.h"

class AnnotationStructureTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit AnnotationStructureTreeModel(AnnotationStructure *structure, bool readOnly = false, bool checkboxes = false,
                                          QObject *parent = 0);
    ~AnnotationStructureTreeModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    bool insertRows(int position, int rows, const QModelIndex &parent = QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex());

    QList<QPair<QString, QString> > selectedLevelsAttributes() const;

signals:

public slots:

private:
    bool m_readOnly;
    bool m_checkboxes;
    AnnotationStructure *m_structure;
    QObject *getItem(const QModelIndex &index) const;
    QList<QPair<QString, QString> > m_selected;
};

#endif // ANNOTATIONSTRUCTURETREEMODEL_H
