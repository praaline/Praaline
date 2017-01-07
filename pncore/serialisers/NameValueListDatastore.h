#ifndef NAMEVALUELISTDATASTORE_H
#define NAMEVALUELISTDATASTORE_H

#include "pncore_global.h"
#include <QString>
#include <QPointer>
#include <QMap>
#include "structure/NameValueList.h"

namespace Praaline {
namespace Core {

class PRAALINE_CORE_SHARED_EXPORT NameValueListDatastore
{
public:
    NameValueListDatastore() {}
    virtual ~NameValueListDatastore() {}

    virtual NameValueList *getNameValueList(const QString &listID) = 0;
    virtual QStringList getAllNameValueListIDs() = 0;
    virtual QMap<QString, QPointer<NameValueList> > getAllNameValueLists() = 0;
    virtual bool createNameValueList(NameValueList *list) = 0;
    virtual bool updateNameValueList(NameValueList *list) = 0;
    virtual bool deleteNameValueList(const QString &listID) = 0;
};

} // namespace Core
} // namespace Praaline

#endif // NAMEVALUELISTDATASTORE_H
