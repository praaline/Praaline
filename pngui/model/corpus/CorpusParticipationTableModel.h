#ifndef CORPUSPARTICIPATIONTABLEMODEL_H
#define CORPUSPARTICIPATIONTABLEMODEL_H

#include <QObject>
#include <QPointer>
#include <QList>
#include <QAbstractTableModel>

namespace Praaline {
namespace Core {
class Corpus;
class CorpusParticipation;
class MetadataStructureAttribute;
}
}

struct CorpusParticipationTableModelData;

class CorpusParticipationTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit CorpusParticipationTableModel(QList<QPointer<Praaline::Core::CorpusParticipation> > items,
                                           QList<QPointer<Praaline::Core::MetadataStructureAttribute> > attributes,
                                           QPointer<Praaline::Core::Corpus> corpus, bool multiCorpus = false, QObject *parent = 0);
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
    CorpusParticipationTableModelData *d;
};

#endif // CORPUSPARTICIPATIONTABLEMODEL_H
