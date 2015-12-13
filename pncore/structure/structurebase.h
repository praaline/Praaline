#ifndef STRUCTUREBASE_H
#define STRUCTUREBASE_H

#include "pncore_global.h"
#include <QObject>

class PRAALINE_CORE_SHARED_EXPORT StructureBase : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString ID READ ID WRITE setID)
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString description READ description WRITE setDescription)
    Q_PROPERTY(QString datatype READ datatype WRITE setDatatype)
    Q_PROPERTY(int datalength READ datalength WRITE setDatalength)
    Q_PROPERTY(bool indexed READ indexed WRITE setIndexed)
    Q_PROPERTY(QString nameValueList READ nameValueList WRITE setNameValueList)

public:
    explicit StructureBase(QObject *parent = 0);
    StructureBase(const QString &ID, QObject *parent = 0);
    StructureBase(const QString &ID, const QString &name, const QString &description, const QString &datatype, int datalength,
                  bool indexed = false, const QString &nameValueList = QString(), QObject *parent = 0);
    StructureBase(const StructureBase *other, QObject *parent = 0);
    virtual ~StructureBase() {}

    // Data
    virtual QString ID() const { return m_ID; }
    virtual void setID(const QString &ID) { m_ID = ID; }
    virtual QString name() const { return m_name; }
    virtual void setName(const QString &name) { m_name = name; }
    virtual QString description() const { return m_description; }
    virtual void setDescription(const QString &description) { m_description = description; }
    virtual QString datatype() const { return m_datatype; }
    virtual void setDatatype(const QString &datatype) { m_datatype = datatype; }
    virtual int datalength() const { return m_datalength; }
    virtual void setDatalength(int length) { m_datalength = length; }
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
    QString m_datatype;         // Data type (e.g. double)
    int m_datalength;           // Data type length
    bool m_indexed;             // Should create database index?
    QString m_nameValueList;    // Use a name-value list (controlled vocabulary)
};

#endif // STRUCTUREBASE_H
