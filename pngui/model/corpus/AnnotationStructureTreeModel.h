#ifndef ANNOTATIONSTRUCTURETREEMODEL_H
#define ANNOTATIONSTRUCTURETREEMODEL_H

#include <QObject>
#include <QList>
#include <QPair>
#include <QAbstractItemModel>

namespace Praaline {
namespace Core {
class AnnotationStructure;
}
}

struct AnnotationStructureTreeModelData;

class AnnotationStructureTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit AnnotationStructureTreeModel(Praaline::Core::AnnotationStructure *structure,
                                          bool readOnly = false, bool checkboxes = false,
                                          QObject *parent = nullptr);
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
    void renameAnnotationLevel(QString oldID, QString newID);
    void renameAnnotationAttribute(QString levelID, QString oldID, QString newID);
    void annotationLevelAttributeSelectionChanged(QString levelID, QString attributeID, bool selected);

public slots:

private:
    AnnotationStructureTreeModelData *d;
    QObject *getItem(const QModelIndex &index) const;
};

#endif // ANNOTATIONSTRUCTURETREEMODEL_H
