#ifndef ANNOTATIONSTRUCTUREATTRIBUTE_H
#define ANNOTATIONSTRUCTUREATTRIBUTE_H

#include "pncore_global.h"
#include <QObject>
#include <QString>
#include "structurebase.h"

class PRAALINE_CORE_SHARED_EXPORT AnnotationStructureAttribute : public StructureBase
{
    Q_OBJECT
    Q_PROPERTY(QString statLevelOfMeasurement READ statLevelOfMeasurement  WRITE setStatLevelOfMeasurement)

public:
    explicit AnnotationStructureAttribute(QObject *parent = 0);
    AnnotationStructureAttribute(const AnnotationStructureAttribute *other, QObject *parent = 0);
    AnnotationStructureAttribute(const QString &ID, const QString &name = QString(), const QString &description = QString(),
                                 const QString &datatype = "varchar", int datalength = 256, bool indexed = false, const QString &nameValueList = QString(),
                                 QObject *parent = 0);

    // Data
    QString statLevelOfMeasurement() const { return m_statLevelOfMeasurement; }
    void setStatLevelOfMeasurement(const QString &statLevelOfMeasurement) { m_statLevelOfMeasurement = statLevelOfMeasurement; }

signals:
    
public slots:

protected:
    QString m_statLevelOfMeasurement;
};

#endif // CORPUSANNOTATIONATTRIBUTE_H
