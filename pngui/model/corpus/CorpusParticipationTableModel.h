#ifndef CORPUSPARTICIPATIONTABLEMODEL_H
#define CORPUSPARTICIPATIONTABLEMODEL_H

#include <QObject>
#include <QPointer>
#include <QList>
#include <QAbstractTableModel>

namespace Praaline {
namespace Core {
class CorpusParticipation;
class MetadataStructureAttribute;
}
}

struct CorpusParticipationTableModelData;

class CorpusParticipationTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit CorpusParticipationTableModel(QList<Praaline::Core::CorpusParticipation *> items,
                                           QList<Praaline::Core::MetadataStructureAttribute *> attributes,
                                           bool multiCorpus = false, QObject *parent = nullptr);
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
