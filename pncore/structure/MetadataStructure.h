#ifndef METADATASTRUCTURE_H
#define METADATASTRUCTURE_H

/*
    Praaline - Core module - Corpus Structure Definition
    Copyright (c) 2011-2017 George Christodoulides

    This program or module is free software: you can redistribute it
    and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version. It is provided
    for educational purposes and is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
    the GNU General Public License for more details.
*/

#include "pncore_global.h"
#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>
#include <QHash>
#include "corpus/CorpusObject.h"
#include "MetadataStructureSection.h"

namespace Praaline {
namespace Core {

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
    QStringList attributeIDs(CorpusObject::Type what) const;
    QStringList attributeNames(CorpusObject::Type what) const;
    CorpusObject::Type corpusObjectTypeOfSection(QPointer<MetadataStructureSection> section) const;
    // Defaults
    static QString defaultSectionID(CorpusObject::Type type);
    static QStringList basicAttributeIDs(CorpusObject::Type type);
    static QStringList basicAttributeNames(CorpusObject::Type type);
    QStringList allAttributeIDs(CorpusObject::Type what) const;
    QStringList allAttributeNames(CorpusObject::Type what) const;

signals:
    void MetadataStructureChanged();

public slots:

protected:
    QString m_ID;
    QHash<CorpusObject::Type, QList<QPointer<MetadataStructureSection> > > m_sections;
};

} // namespace Core
} // namespace Praaline

#endif // METADATASTRUCTURE_H
