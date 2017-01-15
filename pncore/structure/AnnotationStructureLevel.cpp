#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>
#include "StructureAttributeBase.h"
#include "AnnotationStructureAttribute.h"
#include "AnnotationStructureLevel.h"

namespace Praaline {
namespace Core {

AnnotationStructureLevel::AnnotationStructureLevel(QObject *parent) :
    StructureAttributeBase(parent)
{
}

AnnotationStructureLevel::AnnotationStructureLevel(const QString &ID, AnnotationStructureLevel::LevelType levelType,
                                                   const QString &name, const QString &description,
                                                   const QString &parentLevelID,
                                                   const DataType &datatype, int order, bool indexed, const QString &nameValueList,
                                                   QObject *parent) :
    StructureAttributeBase(ID, name, description, datatype, order, indexed, nameValueList, parent),
    m_levelType(levelType), m_parentLevelID(parentLevelID)
{
}

bool AnnotationStructureLevel::isLevelTypePrimary() const
{
    if ((m_levelType == AnnotationStructureLevel::IndependentPointsLevel) ||
        (m_levelType == AnnotationStructureLevel::IndependentIntervalsLevel) ||
        (m_levelType == AnnotationStructureLevel::GroupingLevel)) {
        return true;
    }
    // else
    return false;
}

// ==========================================================================================================
// ATTRIBUTES
// ==========================================================================================================

QPointer<AnnotationStructureAttribute> AnnotationStructureLevel::attribute(int index) const
{
    return m_attributes.value(index);
}

QPointer<AnnotationStructureAttribute> AnnotationStructureLevel::attribute(const QString &ID) const
{
    foreach (QPointer<AnnotationStructureAttribute> attribute, m_attributes) {
        if ((attribute) && (attribute->ID() == ID))
            return attribute;
    }
    return 0;
}

int AnnotationStructureLevel::attributeIndexByID(const QString &ID) const
{
    for (int i = 0; i < m_attributes.count(); i++ ) {
        if ((m_attributes[i]) && (m_attributes[i]->ID() == ID))
            return i;
    }
    return -1;
}

int AnnotationStructureLevel::attributesCount() const
{
    return m_attributes.count();
}

bool AnnotationStructureLevel::hasAttributes() const
{
    return !m_attributes.isEmpty();
}

bool AnnotationStructureLevel::hasAttribute(const QString &ID)
{
    return (attributeIndexByID(ID) != -1);
}

QStringList AnnotationStructureLevel::attributeIDs() const
{
    QStringList ret;
    foreach (QPointer<AnnotationStructureAttribute> attribute, m_attributes)
        if (attribute) ret << attribute->ID();
    return ret;
}

QList<QPointer<AnnotationStructureAttribute> > AnnotationStructureLevel::attributes() const
{
    return m_attributes;
}

void AnnotationStructureLevel::insertAttribute(int index, AnnotationStructureAttribute *attribute)
{
    if (!attribute) return;
    attribute->setParent(this);
    m_attributes.insert(index, attribute);
}

void AnnotationStructureLevel::addAttribute(AnnotationStructureAttribute *attribute)
{
    if (!attribute) return;
    attribute->setParent(this);
    m_attributes << attribute;
}

void AnnotationStructureLevel::swapAttribute(int oldIndex, int newIndex)
{
    m_attributes.swap(oldIndex, newIndex);
}

void AnnotationStructureLevel::removeAttributeAt(int i)
{
    if (i >= 0 && i < m_attributes.count()) {
        m_attributes.removeAt(i);
    }
}

void AnnotationStructureLevel::removeAttributeByID(const QString &ID)
{
    int i = attributeIndexByID(ID);
    if (i != -1)
        m_attributes.removeAt(i);
}

} // namespace Core
} // namespace Praaline
