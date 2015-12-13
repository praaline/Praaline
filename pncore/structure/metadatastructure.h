#ifndef METADATASTRUCTURE_H
#define METADATASTRUCTURE_H

#include "pncore_global.h"
#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>
#include <QHash>
#include "corpus/corpusobject.h"
#include "metadatastructuresection.h"

class PRAALINE_CORE_SHARED_EXPORT MetadataStructure : public QObject
{
    Q_OBJECT
public:
    explicit MetadataStructure(QObject *parent = 0);
    ~MetadataStructure();

    QString ID() const { return m_ID; }
    void setID(const QString &ID) { m_ID = ID; }

    // Management
    void clear(CorpusObject::Type what);
    void clearAll();

    // METADATA STRUCTURE SECTIONS by OBJECT
    QPointer<MetadataStructureSection> section(CorpusObject::Type what, int index) const;
    QPointer<MetadataStructureSection> section(CorpusObject::Type what, const QString &ID) const;
    int sectionIndexByID(CorpusObject::Type what, const QString &ID) const;
    int sectionsCount(CorpusObject::Type what) const;
    bool hasSections(CorpusObject::Type what) const;
    QList<QPointer<MetadataStructureSection> > sections(CorpusObject::Type what) const;
    void insertSection(CorpusObject::Type what, int index, MetadataStructureSection *section);
    void addSection(CorpusObject::Type what, MetadataStructureSection *section);
    void swapSections(CorpusObject::Type what, int oldIndex, int newIndex);
    void removeSectionAt(CorpusObject::Type what, int i);
    void removeSectionByID(CorpusObject::Type what, const QString &ID);
    // Direct access to attributes
    QPointer<MetadataStructureAttribute> attribute(CorpusObject::Type what, const QString &ID) const;
    QList<QPointer<MetadataStructureAttribute> > attributes(CorpusObject::Type what) const;
    CorpusObject::Type corpusObjectTypeOfSection(QPointer<MetadataStructureSection> section) const;

signals:
    void MetadataStructureChanged();

public slots:

protected:
    QString m_ID;
    QHash<CorpusObject::Type, QList<QPointer<MetadataStructureSection> > > m_sections;
};

#endif // METADATASTRUCTURE_H
