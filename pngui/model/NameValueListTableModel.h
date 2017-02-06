#ifndef NAMEVALUELISTTABLEMODEL_H
#define NAMEVALUELISTTABLEMODEL_H

#include <QPointer>
#include <QAbstractTableModel>
#include "pncore/structure/NameValueList.h"

namespace Praaline {
namespace Core {
class NameValueList;
}
}

struct NameValueListTableModelData;

class NameValueListTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit NameValueListTableModel(QPointer<Praaline::Core::NameValueList> nvl, QObject *parent = 0);
    ~NameValueListTableModel();

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex& index) const Q_DECL_OVERRIDE;

    bool insertRows(int position, int rows, const QModelIndex &index=QModelIndex()) Q_DECL_OVERRIDE;
    bool removeRows(int position, int rows, const QModelIndex &index=QModelIndex()) Q_DECL_OVERRIDE;

private:
    NameValueListTableModelData *d;
};

#endif // NAMEVALUELISTTABLEMODEL_H
