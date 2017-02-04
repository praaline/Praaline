#include <QObject>
#include <QString>
#include <QList>
#include <QHash>
#include <QMetaEnum>
#include "MetadataStructure.h"

namespace Praaline {
namespace Core {

MetadataStructure::MetadataStructure(QObject *parent) :
    QObject(parent)
{
    // There is always a default metadata section for each corpus object type
    m_sections[CorpusObject::Type_Corpus].append(
                new MetadataStructureSection("corpus", tr("Corpus"), tr("Corpus Metadata"), 0, this));
    m_sections[CorpusObject::Type_Communication].append(
                new MetadataStructureSection("communication", tr("Communication"), tr("Communication Metadata"), 0, this));
    m_sections[CorpusObject::Type_Speaker].append(
                new MetadataStructureSection("speaker", tr("Speaker"), tr("Speaker Metadata"), 0, this));
    m_sections[CorpusObject::Type_Recording].append(
                new MetadataStructureSection("recording", tr("Recording"), tr("Recording Metadata"), 0, this));
    m_sections[CorpusObject::Type_Annotation].append(
                new MetadataStructureSection("annotation", tr("Annotation"), tr("Annotation Metadata"), 0, this));
    m_sections[CorpusObject::Type_Participation].append(
                new MetadataStructureSection("participation", tr("Participation"), tr("Participation Metadata"), 0, this));
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
    if (index == 0) index = 1; // you cannot insert before the default
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
    if ((oldIndex == 0) || (newIndex == 0)) return;
    m_sections[what].swap(oldIndex, newIndex);
    emit MetadataStructureChanged();
}

void MetadataStructure::removeSectionAt(CorpusObject::Type what, int i)
{
    if (i == 0) return;
    m_sections[what].removeAt(i);
    emit MetadataStructureChanged();
}

void MetadataStructure::removeSectionByID(CorpusObject::Type what, const QString &ID)
{
    int i = sectionIndexByID(what, ID);
    if (i != -1 && i != 0) {
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

QStringList MetadataStructure::attributeIDs(CorpusObject::Type what) const
{
    QStringList ret;
    if (!m_sections.contains(what)) return ret;
    foreach (QPointer<MetadataStructureSection> section, m_sections[what]) {
        if (!section) continue;
        ret << section->attributeIDs();
    }
    return ret;
}

QStringList MetadataStructure::attributeNames(CorpusObject::Type what) const
{
    QStringList ret;
    if (!m_sections.contains(what)) return ret;
    foreach (QPointer<MetadataStructureSection> section, m_sections[what]) {
        if (!section) continue;
        ret << section->attributeNames();
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
    if (what == CorpusObject::Type_Corpus) {
        m_sections[CorpusObject::Type_Corpus].append(
                    new MetadataStructureSection("corpus", tr("Corpus"), tr("Corpus Metadata"), 0, this));
    } else if (what == CorpusObject::Type_Communication) {
        m_sections[CorpusObject::Type_Communication].append(
                    new MetadataStructureSection("communication", tr("Communication"), tr("Communication Metadata"), 0, this));
    } else if (what == CorpusObject::Type_Speaker) {
        m_sections[CorpusObject::Type_Speaker].append(
                    new MetadataStructureSection("speaker", tr("Speaker"), tr("Speaker Metadata"), 0, this));
    } else if (what == CorpusObject::Type_Recording) {
        m_sections[CorpusObject::Type_Recording].append(
                    new MetadataStructureSection("recording", tr("Recording"), tr("Recording Metadata"), 0, this));
    } else if (what == CorpusObject::Type_Annotation) {
        m_sections[CorpusObject::Type_Annotation].append(
                    new MetadataStructureSection("annotation", tr("Annotation"), tr("Annotation Metadata"), 0, this));
    } else if (what == CorpusObject::Type_Participation) {
        m_sections[CorpusObject::Type_Participation].append(
                    new MetadataStructureSection("participation", tr("Participation"), tr("Participation Metadata"), 0, this));
    }
    emit MetadataStructureChanged();
}

void MetadataStructure::clearAll()
{
    m_ID = "";
    foreach (CorpusObject::Type what, m_sections.keys()) {
        clear(what);
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

// static
QString MetadataStructure::defaultSectionID(CorpusObject::Type type)
{
    if      (type == CorpusObject::Type_Corpus)         return "corpus";
    else if (type == CorpusObject::Type_Communication)  return "communication";
    else if (type == CorpusObject::Type_Speaker)        return "speaker";
    else if (type == CorpusObject::Type_Recording)      return "recording";
    else if (type == CorpusObject::Type_Annotation)     return "annotation";
    else if (type == CorpusObject::Type_Participation)  return "participation";
    return QString();
}

// static
QStringList MetadataStructure::basicAttributeIDs(CorpusObject::Type type)
{
    QStringList attributeIDs;
    if      (type == CorpusObject::Type_Corpus)
        attributeIDs << "corpusID" << "corpusName" << "description";
    else if (type == CorpusObject::Type_Communication)
        attributeIDs << "communicationID" << "corpusID" << "communicationName";
    else if (type == CorpusObject::Type_Speaker)
        attributeIDs << "speakerID" << "corpusID" << "speakerName";
    else if (type == CorpusObject::Type_Recording)
        attributeIDs << "recordingID" << "communicationID" << "recordingName" << "filename" << "format" << "duration"
                     << "channels" << "sampleRate" << "precisionBits" << "bitRate" << "encoding" << "fileSize" << "checksumMD5";
    else if (type == CorpusObject::Type_Annotation)
        attributeIDs << "annotationID" << "communicationID" << "annotationName" << "recordingID";
    else if (type == CorpusObject::Type_Participation)
        attributeIDs << "corpusID" << "communicationID" << "speakerID" << "role";
    return attributeIDs;
}

// static
QStringList MetadataStructure::basicAttributeNames(CorpusObject::Type type)
{
    QStringList attributeNames;
    if      (type == CorpusObject::Type_Corpus)
        attributeNames << "Corpus ID" << "Corpus Name" << "Description";
    else if (type == CorpusObject::Type_Communication)
        attributeNames << "Communication ID" << "Corpus ID" << "Communication Name";
    else if (type == CorpusObject::Type_Speaker)
        attributeNames << "Speaker ID" << "Corpus ID" << "Speaker Name";
    else if (type == CorpusObject::Type_Recording)
        attributeNames << "Recording ID" << "Communication ID" << "Recording Name" << "Filename" << "Format" << "Duration"
                     << "Channels" << "Sample Rate" << "Precision Bits" << "Bitrate" << "Encoding" << "File Size" << "Checksum MD5";
    else if (type == CorpusObject::Type_Annotation)
        attributeNames << "Annotation ID" << "Communication ID" << "Annotation Name" << "Recording ID";
    else if (type == CorpusObject::Type_Participation)
        attributeNames << "Corpus ID" << "Communication ID" << "Speaker ID" << "Role";
    return attributeNames;
}

QStringList MetadataStructure::allAttributeIDs(CorpusObject::Type what) const
{
    QStringList ret;
    ret << basicAttributeIDs(what);
    if (!m_sections.contains(what)) return ret;
    foreach (QPointer<MetadataStructureSection> section, m_sections[what]) {
        if (!section) continue;
        ret << section->attributeIDs();
    }
    return ret;
}

QStringList MetadataStructure::allAttributeNames(CorpusObject::Type what) const
{
    QStringList ret;
    ret << basicAttributeNames(what);
    if (!m_sections.contains(what)) return ret;
    foreach (QPointer<MetadataStructureSection> section, m_sections[what]) {
        if (!section) continue;
        ret << section->attributeNames();
    }
    return ret;
}

} // namespace Core
} // namespace Praaline
