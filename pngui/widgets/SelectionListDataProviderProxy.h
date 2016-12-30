#ifndef SELECTIONLISTDATAPROVIDERPROXY_H
#define SELECTIONLISTDATAPROVIDERPROXY_H

#include <QObject>
#include <QVariant>
#include <QIdentityProxyModel>

class SelectionListDataProviderProxy : public QIdentityProxyModel
{
    Q_OBJECT
public:
    SelectionListDataProviderProxy(QObject* parent);
    ~SelectionListDataProviderProxy();
    QVariant data(const QModelIndex & proxyIndex, int role = Qt::DisplayRole) const;
};

#endif // SELECTIONLISTDATAPROVIDERPROXY_H
