#include <QObject>
#include <QString>
#include <QList>
#include <QHash>
#include <QMetaEnum>
#include "metadatastructure.h"

MetadataStructure::MetadataStructure(QObject *parent) :
    QObject(parent)
{
}

MetadataStructure::~MetadataStructure()
{
    foreach (CorpusObject::Type type, m_sections.keys())
        qDeleteAll(m_sections[type]);
}

// ==========================================================================================================
// Metadata structure sections
// ==========================================================================================================

QPointer<MetadataStructureSection> MetadataStructure::section(CorpusObject::Type what, int index) const
{
    if (!m_sections.contains(what)) return 0;
    return m_sections[what].value(index);
}

QPointer<MetadataStructureSection> MetadataStructure::section(CorpusObject::Type what, const QString &ID) const
{
    if (!m_sections.contains(what)) return 0;
    foreach (QPointer<MetadataStructureSection> section, m_sections[what]) {
        if ((section) && section->ID() == ID)
            return section;
    }
    return 0;
}

int MetadataStructure::sectionIndexByID(CorpusObject::Type what, const QString &ID) const
{
    if (!m_sections.contains(what)) return -1;
    for (int i = 0; i < m_sections[what].count(); i++ ) {
        if ((m_sections[what][i]) && (m_sections[what][i]->ID() == ID))
            return i;
    }
    return -1;
}

int MetadataStructure::sectionsCount(CorpusObject::Type what) const
{
    if (!m_sections.contains(what)) return 0;
    return m_sections[what].count();
}

bool MetadataStructure::hasSections(CorpusObject::Type what) const
{
    if (!m_sections.contains(what)) return false;
    return !m_sections[what].isEmpty();
}

QList<QPointer<MetadataStructureSection> > MetadataStructure::sections(CorpusObject::Type what) const
{
    return m_sections[what];
}

void MetadataStructure::insertSection(CorpusObject::Type what, int index, MetadataStructureSection *section)
{
    if (!section) return;
    section->setParent(this);
    m_sections[what].insert(index, section);
    emit MetadataStructureChanged();
}

void MetadataStructure::addSection(CorpusObject::Type what, MetadataStructureSection *section)
{
    if (!section) return;
    section->setParent(this);
    m_sections[what] << section;
    emit MetadataStructureChanged();
}

void MetadataStructure::swapSections(CorpusObject::Type what, int oldIndex, int newIndex)
{
    m_sections[what].swap(oldIndex, newIndex);
    emit MetadataStructureChanged();
}

void MetadataStructure::removeSectionAt(CorpusObject::Type what, int i)
{
    m_sections[what].removeAt(i);
    emit MetadataStructureChanged();
}

void MetadataStructure::removeSectionByID(CorpusObject::Type what, const QString &ID)
{
    int i = sectionIndexByID(what, ID);
    if (i != -1) {
        m_sections[what].removeAt(i);
        emit MetadataStructureChanged();
    }
}

QPointer<MetadataStructureAttribute> MetadataStructure::attribute(CorpusObject::Type what, const QString &ID) const
{
    foreach (QPointer<MetadataStructureSection> section, m_sections[what]) {
        if (!section) continue;
        foreach (QPointer<MetadataStructureAttribute> attribute, section->attributes())
            if ((attribute) && (attribute->ID() == ID))
                return attribute;
    }
    return 0;
}

QList<QPointer<MetadataStructureAttribute> > MetadataStructure::attributes(CorpusObject::Type what) const
{
    QList<QPointer<MetadataStructureAttribute> > ret;
    if (!m_sections.contains(what)) return ret;
    foreach (QPointer<MetadataStructureSection> section, m_sections[what]) {
        if (!section) continue;
        foreach (QPointer<MetadataStructureAttribute> attribute, section->attributes()) {
            if (attribute) ret << attribute;
        }
    }
    return ret;
}

// ==========================================================================================================
// Management
// ==========================================================================================================

void MetadataStructure::clear(CorpusObject::Type what)
{
    qDeleteAll(m_sections[what]);
    m_sections[what].clear();
    emit MetadataStructureChanged();
}

void MetadataStructure::clearAll()
{
    m_ID = "";
    foreach (CorpusObject::Type type, m_sections.keys()) {
        qDeleteAll(m_sections[type]);
        m_sections[type].clear();
    }
    emit MetadataStructureChanged();
}

CorpusObject::Type MetadataStructure::corpusObjectTypeOfSection(QPointer<MetadataStructureSection> section) const
{
    foreach (CorpusObject::Type type, m_sections.keys()) {
        if (m_sections[type].contains(section))
            return type;
    }
    return CorpusObject::Type_Undefined;
}

