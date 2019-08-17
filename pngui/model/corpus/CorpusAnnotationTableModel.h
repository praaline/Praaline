#ifndef CORPUSANNOTATIONTABLEMODEL_H
#define CORPUSANNOTATIONTABLEMODEL_H

#include <QObject>
#include <QPointer>
#include <QList>
#include <QAbstractTableModel>

namespace Praaline {
namespace Core {
class CorpusAnnotation;
class MetadataStructureAttribute;
}
}

struct CorpusAnnotationTableModelData;

class CorpusAnnotationTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    CorpusAnnotationTableModel(QList<Praaline::Core::CorpusAnnotation *> items,
                               QList<Praaline::Core::MetadataStructureAttribute *> attributes,
                               bool multiCommunication = true, QObject *parent = nullptr);
    ~CorpusAnnotationTableModel();

    void refresh(const QString &corpusID = QString());
    bool saveAll();

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
    CorpusAnnotationTableModelData *d;
};

#endif // CORPUSANNOTATIONTABLEMODEL_H
