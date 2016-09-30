#include "pncore/structure/metadatastructure.h"
#include "metadatastructuretreemodel.h"

MetadataStructureTreeModel::MetadataStructureTreeModel(MetadataStructure *structure, QObject *parent) :
    QAbstractItemModel(parent), m_structure(structure)
{
    MetadataStructureTreeModelCategory *category;
    category = new MetadataStructureTreeModelCategory("Corpus Metadata", CorpusObject::Type_Corpus, "corpus", this);
    m_corpusObjectCategories << category;
    category = new MetadataStructureTreeModelCategory("Communication Metadata", CorpusObject::Type_Communication, "communication", this);
    m_corpusObjectCategories << category;
    category = new MetadataStructureTreeModelCategory("Recording Metadata", CorpusObject::Type_Recording, "recording", this);
    m_corpusObjectCategories << category;
    category = new MetadataStructureTreeModelCategory("Annotation Metadata", CorpusObject::Type_Annotation, "annotation", this);
    m_corpusObjectCategories << category;
    category = new MetadataStructureTreeModelCategory("Speaker Metadata", CorpusObject::Type_Speaker, "speaker", this);
    m_corpusObjectCategories << category;
    category = new MetadataStructureTreeModelCategory("Participation Metadata", CorpusObject::Type_Participation, "participation", this);
    m_corpusObjectCategories << category;
}

MetadataStructureTreeModel::~MetadataStructureTreeModel()
{
    qDeleteAll(m_corpusObjectCategories);
}

int MetadataStructureTreeModel::rowCount(const QModelIndex &parent) const
{
    QObject *parentItem = getItem(parent);
    if (parentItem == m_structure)
        return m_corpusObjectCategories.count();
    // otherwise
    MetadataStructureTreeModelCategory *category = qobject_cast<MetadataStructureTreeModelCategory *>(parentItem);
    if (category)
        return m_structure->sectionsCount(category->type);
    // otherwise
    MetadataStructureSection *section = qobject_cast<MetadataStructureSection *>(parentItem);
    if (section)
        return section->attributesCount();
    return 0;
}

int MetadataStructureTreeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    // ID, Name, Description (for all), datatype, length, indexed, nameValueList (for attributes)
    return 7;
}

// private
QObject *MetadataStructureTreeModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        QObject *item = static_cast<QObject *>(index.internalPointer());
        if (item) return item;
    }
    return static_cast<QObject *>(m_structure);
}

QModelIndex MetadataStructureTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();
    QObject *parentItem = getItem(parent);
    if (m_structure == parentItem) {
        MetadataStructureTreeModelCategory *child = m_corpusObjectCategories.at(row);
        return createIndex(row, column, child);
    }
    // otherwise
    MetadataStructureTreeModelCategory *category = qobject_cast<MetadataStructureTreeModelCategory *>(parentItem);
    if (category) {
        MetadataStructureSection *child = m_structure->section(category->type, row);
        return createIndex(row, column, child);
    }
    // otherwise
    MetadataStructureSection *section = qobject_cast<MetadataStructureSection *>(parentItem);
    if (section) {
        MetadataStructureAttribute *child = section->attribute(row);
        return createIndex(row, column, child);
    }
    // otherwise
    return QModelIndex();
}

QModelIndex MetadataStructureTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) return QModelIndex();

    QObject *childItem = getItem(index);
    if (!childItem) return QModelIndex();
    QObject *parentItem = childItem->parent();

    MetadataStructureSection *section;
    MetadataStructureAttribute *attribute;

    if (parentItem == this)
        return QModelIndex(); // categories
    // otherwise : the parent of a section is a category
    section = qobject_cast<MetadataStructureSection *>(childItem);
    if (section) {
        CorpusObject::Type type = m_structure->corpusObjectTypeOfSection(section);
        int i;
        for (i = 0; i < m_corpusObjectCategories.count(); ++i)
            if (m_corpusObjectCategories.at(i)->type == type) break;
        if (i < m_corpusObjectCategories.count())
            return createIndex(m_structure->sections(type).indexOf(section), 0, m_corpusObjectCategories.at(i));
        else return QModelIndex();
    }
    // otherwise : the parent of an attribute is a section
    section = qobject_cast<MetadataStructureSection *>(parentItem);
    attribute = qobject_cast<MetadataStructureAttribute *>(childItem);
    if (section && attribute) {
        return createIndex(section->attributes().indexOf(attribute), 0, parentItem);
    }
    return QModelIndex();
}

QVariant MetadataStructureTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();
    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    QObject *item = getItem(index);
    bool isAttribute = (qobject_cast<MetadataStructureAttribute *>(item) != 0);
    switch (index.column()) {
    case 0: return item->property("ID"); break;
    case 1: return item->property("name"); break;
    case 2: return item->property("description"); break;
    case 3: return (isAttribute) ? item->property("datatype") : QVariant(); break;
    case 4: return (isAttribute) ? item->property("datalength").toInt() : QVariant(); break;
    case 5: return (isAttribute) ? item->property("indexed").toBool() : QVariant(); break;
    case 6: return (isAttribute) ? item->property("nameValueList") : QVariant(); break;
    }
    return QVariant();
}

QVariant MetadataStructureTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case 0: return "ID"; break;
        case 1: return "Name"; break;
        case 2: return "Description"; break;
        case 3: return "Data type"; break;
        case 4: return "Length"; break;
        case 5: return "Indexed"; break;
        case 6: return "Value List"; break;
        }
    }
    return QVariant();
}

Qt::ItemFlags MetadataStructureTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) return 0;
    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool MetadataStructureTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    bool result = false;
    if (role != Qt::EditRole)
        return false;

    QObject *item = getItem(index);
    bool isAttribute = (qobject_cast<MetadataStructureAttribute *>(item) != 0);
    if ((index.column() >=3) && (!isAttribute)) return false;
    switch (index.column()) {
    case 0: result = item->setProperty("ID", value); break;
    case 1: result = item->setProperty("name", value); break;
    case 2: result = item->setProperty("description", value); break;
    case 3: result = item->setProperty("datatype", value); break;
    case 4: result = item->setProperty("datalength", value); break;
    case 5: result = item->setProperty("indexed", value.toBool()); break;
    case 6: result = item->setProperty("nameValueList", value); break;
    }

    if (result)
        emit dataChanged(index, index);

    return result;
}

