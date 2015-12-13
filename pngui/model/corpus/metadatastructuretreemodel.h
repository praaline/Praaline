#ifndef METADATASTRUCTURETREEMODEL_H
#define METADATASTRUCTURETREEMODEL_H

#include <QObject>
#include <QStandardItem>
#include <QAbstractItemModel>
#include "pncore/structure/metadatastructure.h"

class MetadataStructureTreeModelCategory : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString ID READ getID)
public:
    MetadataStructureTreeModelCategory(QString ID, CorpusObject::Type type, QString defaultSectionName,
                                       QObject *parent = 0) :
        QObject(parent), ID(ID), type(type), defaultSectionName(defaultSectionName) {}
    QString ID;
    CorpusObject::Type type;
    QString defaultSectionName;
    QString getID() { return ID; }
};

class MetadataStructureTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    MetadataStructureTreeModel(MetadataStructure *structure, QObject *parent = 0);
    ~MetadataStructureTreeModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    bool addAttribute(const QModelIndex &index, const QString &ID);

signals:

public slots:

private:
    MetadataStructure *m_structure;
    QList<MetadataStructureTreeModelCategory *> m_corpusObjectCategories;

    QObject *getItem(const QModelIndex &index) const;
};

#endif // METADATASTRUCTURETREEMODEL_H
