#ifndef NAMEVALUELISTTABLEMODEL_H
#define NAMEVALUELISTTABLEMODEL_H

#include <QPointer>
#include <QAbstractTableModel>
#include "PraalineCore/Structure/NameValueList.h"

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
    explicit NameValueListTableModel(QPointer<Praaline::Core::NameValueList> nvl, QObject *parent = nullptr);
    ~NameValueListTableModel();

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    bool insertRows(int position, int rows, const QModelIndex &index=QModelIndex()) override;
    bool removeRows(int position, int rows, const QModelIndex &index=QModelIndex()) override;

private:
    NameValueListTableModelData *d;
};

#endif // NAMEVALUELISTTABLEMODEL_H
