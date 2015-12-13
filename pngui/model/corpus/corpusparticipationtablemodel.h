#ifndef CORPUSPARTICIPATIONTABLEMODEL_H
#define CORPUSPARTICIPATIONTABLEMODEL_H

#include <QObject>
#include <QPointer>
#include <QList>
#include <QAbstractTableModel>
#include "pncore/corpus/corpus.h"
#include "pncore/corpus/corpusparticipation.h"
#include "pncore/structure/metadatastructure.h"

class CorpusParticipationTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit CorpusParticipationTableModel(QList<QPointer<CorpusParticipation> > items,
                                           QList<QPointer<MetadataStructureAttribute> > attributes,
                                           QPointer<Corpus> corpus, bool multiCorpus = false, QObject *parent = 0);
    ~CorpusParticipationTableModel();

    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    bool insertRows(int position, int rows, const QModelIndex &index=QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &index=QModelIndex());

signals:

public slots:

private:
    bool m_multiCorpus;
    QList<QPointer<CorpusParticipation> > m_items;
    QList<QPointer<MetadataStructureAttribute> > m_attributes;
    QPointer<Corpus> m_corpus;
};

#endif // CORPUSPARTICIPATIONTABLEMODEL_H
