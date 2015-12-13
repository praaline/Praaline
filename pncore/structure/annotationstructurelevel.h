#ifndef ANNOTATIONSTRUCTURELEVEL_H
#define ANNOTATIONSTRUCTURELEVEL_H

#include "pncore_global.h"
#include <QObject>
#include <QPointer>
#include <QString>
#include <QStringList>
#include "structurebase.h"
#include "annotationstructureattribute.h"

class PRAALINE_CORE_SHARED_EXPORT AnnotationStructureLevel : public StructureBase
{
    Q_OBJECT
    Q_PROPERTY(LevelType levelType READ levelType WRITE setLevelType)
    Q_PROPERTY(QString parentLevelID READ parentLevelID WRITE setParentLevelID)
    Q_ENUMS(LevelType)

public:
    enum LevelType {
        IndependentLevel = 0,
        GroupingLevel = 1,
        SequencesLevel = 2,
        TreeLevel = 3
    };

    explicit AnnotationStructureLevel(QObject *parent = 0);
    AnnotationStructureLevel(const QString &ID, LevelType type = IndependentLevel,
                             const QString &name = QString(), const QString &description = QString(),
                             const QString &parentLevelID = QString(),
                             const QString &datatype = "varchar", int datalength = 1024, bool indexed = false, const QString &nameValueList = QString(),
                             QObject *parent = 0);

    // Data
    LevelType levelType() const { return m_levelType; }
    void setLevelType(LevelType type) { m_levelType = type; }
    QString parentLevelID() const { return m_parentLevelID; }
    void setParentLevelID(const QString &parentLevelID) { m_parentLevelID = parentLevelID; }

    // ATTRIBUTES
    // Accessors
    QPointer<AnnotationStructureAttribute> attribute(int index) const;
    QPointer<AnnotationStructureAttribute> attribute(const QString &ID) const;
    int attributeIndexByID(const QString &ID) const;
    int attributesCount() const;
    bool hasAttributes() const;
    bool hasAttribute(const QString &ID);
    QStringList attributeIDs() const;
    QList<QPointer<AnnotationStructureAttribute> > attributes() const;
    void insertAttribute(int index, AnnotationStructureAttribute *attribute);
    void addAttribute(AnnotationStructureAttribute *attribute);
    void swapAttribute(int oldIndex, int newIndex);
    void removeAttributeAt(int i);
    void removeAttributeByID(const QString &ID);

signals:
    
public slots:

protected:
    LevelType m_levelType;      // Level type (e.g. independent or grouping)
    QString m_parentLevelID;    // Parent level ID for grouping levels (e.g. phone for syll)
    QList<QPointer<AnnotationStructureAttribute> > m_attributes;
};

#endif // CORPUSANNOTATIONLEVEL_H

