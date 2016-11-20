#ifndef STRUCTUREBASE_H
#define STRUCTUREBASE_H

#include "pncore_global.h"
#include "base/DataType.h"
#include <QObject>

namespace Praaline {
namespace Core {

class PRAALINE_CORE_SHARED_EXPORT StructureBase : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString ID READ ID WRITE setID)
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString description READ description WRITE setDescription)
    Q_PROPERTY(QString datatypeString READ datatypeString)
    Q_PROPERTY(quint64 datatypePrecision READ datatypePrecision)
    Q_PROPERTY(int order READ order WRITE setOrder)
    Q_PROPERTY(bool indexed READ indexed WRITE setIndexed)
    Q_PROPERTY(QString nameValueList READ nameValueList WRITE setNameValueList)

public:
    explicit StructureBase(QObject *parent = 0);
    StructureBase(const QString &ID, QObject *parent = 0);
    StructureBase(const QString &ID, const QString &name, const QString &description, const DataType &datatype,
                  int order = 0, bool indexed = false, const QString &nameValueList = QString(), QObject *parent = 0);
    StructureBase(const StructureBase *other, QObject *parent = 0);
    virtual ~StructureBase() {}

    // Data
    virtual QString ID() const { return m_ID; }
    virtual void setID(const QString &ID) { m_ID = ID; }
    virtual QString name() const { return m_name; }
    virtual void setName(const QString &name) { m_name = name; }
    virtual QString description() const { return m_description; }
    virtual void setDescription(const QString &description) { m_description = description; }
    virtual DataType datatype() const { return m_datatype; }
    virtual void setDatatype(const DataType &datatype) { m_datatype = datatype; }
    virtual QString datatypeString() const { return m_datatype.string(); }
    virtual quint64 datatypePrecision() const { return m_datatype.precision(); }
    virtual int order() const { return m_order; }
    virtual void setOrder(int order) { m_order = order; }
    virtual bool indexed() const { return m_indexed; }
    virtual void setIndexed(bool indexed) { m_indexed = indexed; }
    virtual QString nameValueList() const { return m_nameValueList; }
    virtual void setNameValueList(const QString &nameValueList) { m_nameValueList = nameValueList; }

signals:

public slots:

protected:
    QString m_ID;               // ID (e.g. f0min)
    QString m_name;             // User-friendly name (e.g. F0 Minimum)
    QString m_description;      // Description (e.g. Fundamental frequency (f0) minimum, in semitones)
    DataType m_datatype;        // Data type (e.g. double) and precision
    int m_order;                // Used to order attributes in a container (e.g. inside a metadata section)
                                // for presentation purposes
    bool m_indexed;             // Should create database index?
    QString m_nameValueList;    // Use a name-value list (controlled vocabulary)
};

} // namespace Core
} // namespace Praaline

#endif // STRUCTUREBASE_H
