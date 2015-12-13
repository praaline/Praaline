#ifndef CORPUSRECORDINGTABLEMODEL_H
#define CORPUSRECORDINGTABLEMODEL_H

#include <QObject>
#include <QPointer>
#include <QList>
#include <QAbstractTableModel>
#include "pncore/corpus/corpus.h"
#include "pncore/corpus/corpusrecording.h"
#include "pncore/structure/metadatastructure.h"

class CorpusRecordingTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit CorpusRecordingTableModel(QList<QPointer<CorpusRecording> > items,
                                       QList<QPointer<MetadataStructureAttribute> > attributes,
                                       bool multiCommunication = false, QObject *parent = 0);
    ~CorpusRecordingTableModel();

    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex());

signals:

public slots:

private:
    bool m_multiCommunication;
    QList<QPointer<CorpusRecording> > m_items;
    QList<QPointer<MetadataStructureAttribute> > m_attributes;
};

#endif // CORPUSRECORDINGTABLEMODEL_H
