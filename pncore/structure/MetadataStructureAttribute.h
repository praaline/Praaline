#ifndef METADATASTRUCTUREATTRIBUTE_H
#define METADATASTRUCTUREATTRIBUTE_H

#include "pncore_global.h"
#include <QObject>
#include <QString>
#include <QVariant>
#include "StructureBase.h"

namespace Praaline {
namespace Core {

class PRAALINE_CORE_SHARED_EXPORT MetadataStructureAttribute : public StructureBase
{
    Q_OBJECT
    Q_PROPERTY(bool mandatory READ mandatory WRITE setMandatory)
    Q_PROPERTY(QVariant defaultValue READ defaultValue WRITE setDefaultValue)

public:
    explicit MetadataStructureAttribute(QObject *parent = 0);
    MetadataStructureAttribute(const QString &ID, QObject *parent = 0);
    MetadataStructureAttribute(const QString &ID, const QString &name, const QString &description,
                               const DataType &datatype, int order = 0, QObject *parent = 0);
    MetadataStructureAttribute(const MetadataStructureAttribute *other, QObject *parent = 0);

    // Data
    QString sectionID() const;
    bool mandatory() const { return m_mandatory; }
    void setMandatory(bool mandatory) { m_mandatory = mandatory; }
    QVariant defaultValue() const { return m_defaultValue; }
    void setDefaultValue(const QVariant &defaultValue) { m_defaultValue = defaultValue; }

signals:

public slots:

protected:
    bool m_mandatory;
    QVariant m_defaultValue;
};

} // namespace Core
} // namespace Praaline

#endif // METADATASTRUCTUREATTRIBUTE_H
