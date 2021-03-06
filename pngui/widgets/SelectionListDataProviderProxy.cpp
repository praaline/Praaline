#include <QObject>
#include <QIdentityProxyModel>
#include "SelectionListDataProviderProxy.h"

SelectionListDataProviderProxy::SelectionListDataProviderProxy(QObject* parent) :
    QIdentityProxyModel(parent)
{
}

SelectionListDataProviderProxy::~SelectionListDataProviderProxy()
{
}

QVariant SelectionListDataProviderProxy::data(const QModelIndex & proxyIndex, int role) const
{
    return QIdentityProxyModel::data(proxyIndex, role);
}
