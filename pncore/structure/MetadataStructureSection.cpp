#include <QObject>
#include <QString>
#include <QList>
#include "MetadataStructureAttribute.h"
#include "MetadataStructureSection.h"

namespace Praaline {
namespace Core {

MetadataStructureSection::MetadataStructureSection(QObject *parent) :
    QObject(parent)
{
}

MetadataStructureSection::MetadataStructureSection(const QString &ID, const QString &name,
                                                   const QString &description, int itemOrder,
                                                   QObject *parent) :
    QObject(parent), m_ID(ID), m_name(name), m_description(description), m_itemOrder(itemOrder)
{
    if (name.isEmpty()) m_name = ID;
}

MetadataStructureSection::~MetadataStructureSection()
{
}

// ==========================================================================================================
// ATTRIBUTES
// ==========================================================================================================

QPointer<MetadataStructureAttribute> MetadataStructureSection::attribute(int index) const
{
    return m_attributes.value(index);
}

QPointer<MetadataStructureAttribute> MetadataStructureSection::attribute(const QString &ID) const
{
    foreach (QPointer<MetadataStructureAttribute> attribute, m_attributes) {
        if ((attribute) && (attribute->ID() == ID))
            return attribute;
    }
    return 0;
}

int MetadataStructureSection::attributeIndexByID(const QString &ID) const
{
    for (int i = 0; i < m_attributes.count(); i++ ) {
        if ((m_attributes[i]) && (m_attributes[i]->ID() == ID))
            return i;
    }
    return -1;
}

int MetadataStructureSection::attributesCount() const
{
    return m_attributes.count();
}

bool MetadataStructureSection::hasAttributes() const
{
    return !m_attributes.isEmpty();
}

bool MetadataStructureSection::hasAttribute(const QString &ID)
{
    return (attributeIndexByID(ID) != -1);
}

QStringList MetadataStructureSection::attributeIDs() const
{
    QStringList ret;
    foreach (QPointer<MetadataStructureAttribute> attribute, m_attributes)
        if (attribute) ret << attribute->ID();
    return ret;
}

QStringList MetadataStructureSection::attributeNames() const
{
    QStringList ret;
    foreach (QPointer<MetadataStructureAttribute> attribute, m_attributes)
        if (attribute) ret << attribute->name();
    return ret;
}

QList<QPointer<MetadataStructureAttribute> > MetadataStructureSection::attributes() const
{
    return m_attributes;
}

void MetadataStructureSection::insertAttribute(int index, MetadataStructureAttribute *attribute)
{
    if (!attribute) return;
    attribute->setParent(this);
    m_attributes.insert(index, attribute);
}

void MetadataStructureSection::addAttribute(MetadataStructureAttribute *attribute)
{
    if (!attribute) return;
    attribute->setParent(this);
    m_attributes << attribute;
}

void MetadataStructureSection::swapAttribute(int oldIndex, int newIndex)
{
    m_attributes.swap(oldIndex, newIndex);
}

void MetadataStructureSection::removeAttributeAt(int i)
{
    if (i >= 0 && i < m_attributes.count()) {
        m_attributes.removeAt(i);
    }
}

void MetadataStructureSection::removeAttributeByID(const QString &ID)
{
    int i = attributeIndexByID(ID);
    if (i != -1) {
        m_attributes.removeAt(i);
    }
}

} // namespace Core
} // namespace Praaline
