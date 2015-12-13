#ifndef METADATASTRUCTUREATTRIBUTE_H
#define METADATASTRUCTUREATTRIBUTE_H

#include "pncore_global.h"
#include <QObject>
#include <QString>
#include "structurebase.h"

class PRAALINE_CORE_SHARED_EXPORT MetadataStructureAttribute : public StructureBase
{
    Q_OBJECT
public:
    explicit MetadataStructureAttribute(QObject *parent = 0);
    MetadataStructureAttribute(const QString &ID, QObject *parent = 0);
    MetadataStructureAttribute(const QString &ID, const QString &name, const QString &description,
                               const QString &datatype, int datalength, QObject *parent = 0);
    MetadataStructureAttribute(const MetadataStructureAttribute *other, QObject *parent = 0);

    // Data

signals:

public slots:

protected:
};

#endif // METADATASTRUCTUREATTRIBUTE_H
