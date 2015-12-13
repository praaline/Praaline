#include <QDebug>
#include "annotationstructuretreemodel.h"

AnnotationStructureTreeModel::AnnotationStructureTreeModel(AnnotationStructure *structure, bool readOnly, bool checkboxes, QObject *parent) :
    QAbstractItemModel(parent), m_readOnly(readOnly), m_checkboxes(checkboxes), m_structure(structure)
{

}

AnnotationStructureTreeModel::~AnnotationStructureTreeModel()
{
}

int AnnotationStructureTreeModel::rowCount(const QModelIndex &parent) const
{
    QObject *parentItem = getItem(parent);
    if (parentItem == m_structure)
        return m_structure->levelsCount();
    // otherwise
    AnnotationStructureLevel *level = qobject_cast<AnnotationStructureLevel *>(parentItem);
    if (level)
        return level->attributesCount();
    return 0;
}

int AnnotationStructureTreeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    // ID, Name, Description (for all), datatype, length, indexed, nameValueList (attributes), type, parent (levels)
    if (m_readOnly) return 2;
    return 9;
}

// private
QObject *AnnotationStructureTreeModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        QObject *item = static_cast<QObject *>(index.internalPointer());
        if (item) return item;
    }
    return static_cast<QObject *>(m_structure);
}

QModelIndex AnnotationStructureTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();
    QObject *parentItem = getItem(parent);
    if (m_structure == parentItem) {
        AnnotationStructureLevel *child = m_structure->level(row);
        return createIndex(row, column, child);
    }
    // otherwise
    AnnotationStructureLevel *level = qobject_cast<AnnotationStructureLevel *>(parentItem);
    if (level) {
        AnnotationStructureAttribute *child = level->attribute(row);
        return createIndex(row, column, child);
    }
    // otherwise
    return QModelIndex();
}

QModelIndex AnnotationStructureTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) return QModelIndex();

    QObject *childItem = getItem(index);
    QObject *parentItem = childItem->parent();

    AnnotationStructureLevel *level = qobject_cast<AnnotationStructureLevel *>(parentItem);
    AnnotationStructureAttribute *attribute = qobject_cast<AnnotationStructureAttribute *>(childItem);
    if (level && attribute) {
        return createIndex(m_structure->levels().indexOf(level), 0, parentItem);
    }
    return QModelIndex();
}

QString convertLevelTypeToString(int levelType)
{
    if      (levelType == AnnotationStructureLevel::IndependentLevel)   return "Independent";
    else if (levelType == AnnotationStructureLevel::GroupingLevel)      return "Grouping";
    else if (levelType == AnnotationStructureLevel::SequencesLevel)     return "Sequences";
    else if (levelType == AnnotationStructureLevel::TreeLevel)          return "Tree";
    return QString();
}

AnnotationStructureLevel::LevelType convertStringToLevelType(QString str)
{
    if      (str == "Independent")  return AnnotationStructureLevel::IndependentLevel;
    else if (str == "Grouping")     return AnnotationStructureLevel::GroupingLevel;
    else if (str == "Sequences")    return AnnotationStructureLevel::SequencesLevel;
    else if (str == "Tree")         return AnnotationStructureLevel::TreeLevel;
    return AnnotationStructureLevel::IndependentLevel;
}

QVariant AnnotationStructureTreeModel::headerData(int Level, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (Level) {
        case 0: return "ID"; break;
        case 1: return "Name"; break;
        case 2: return "Description"; break;
        case 3: return "Data type"; break;
        case 4: return "Length"; break;
        case 5: return "Indexed"; break;
        case 6: return "Value List"; break;
        case 7: return "Level Type"; break;
        case 8: return "Parent Level ID"; break;
        }
    }
    return QVariant();
}

Qt::ItemFlags AnnotationStructureTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) return 0;
    if (!m_readOnly) {
        if (m_checkboxes && (index.column() == 0) )
            return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
        else
            return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    } else {
        if (m_checkboxes && (index.column() == 0) )
            return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
    }
    return QAbstractItemModel::flags(index);
}

QVariant AnnotationStructureTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();
    if (role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::CheckStateRole)
        return QVariant();

    QObject *item = getItem(index);
    bool isAttribute = (qobject_cast<AnnotationStructureAttribute *>(item) != 0);

    if (role == Qt::CheckStateRole && index.column() == 0) {
        QString levelID, attributeID;
        if (isAttribute) {
            QModelIndex parentIndex = this->parent(index);
            levelID = getItem(parentIndex)->property("ID").toString();
            attributeID = item->property("ID").toString();
            if (m_selected.contains(QPair<QString, QString>(levelID, attributeID)))
                return Qt::Checked;
            else
                return Qt::Unchecked;
        } else {
            levelID = item->property("ID").toString();
            if (m_selected.contains(QPair<QString, QString>(levelID, "")))
                return Qt::Checked;
            else
                return Qt::Unchecked;
        }
    } else if (role == Qt::CheckStateRole && index.column() != 0)
        return QVariant();
    switch (index.column()) {
    case 0: return item->property("ID"); break;
    case 1: return item->property("name"); break;
    case 2: return item->property("description"); break;
    case 3: return item->property("datatype"); break;
    case 4: return item->property("datalength"); break;
    case 5: return item->property("indexed"); break;
    case 6: return item->property("nameValueList"); break;
    case 7: return (!isAttribute) ? convertLevelTypeToString(item->property("levelType").toInt()) : QVariant(); break;
    case 8: return (!isAttribute) ? item->property("parentLevelID") : QVariant(); break;
    }
    return QVariant();
}

bool AnnotationStructureTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    bool result = false;
    if (m_readOnly) {
        if ((!m_checkboxes) || (role != Qt::CheckStateRole))
            return false;
    } else {
        if (role != Qt::EditRole) return false;
    }

    QObject *item = getItem(index);
    bool isAttribute = (qobject_cast<AnnotationStructureAttribute *>(item) != 0);

    if (role == Qt::CheckStateRole && index.column() == 0) {
        QPair<QString, QString> pair;
        if (isAttribute) {
            QModelIndex parentIndex = this->parent(index);
            pair.first = getItem(parentIndex)->property("ID").toString();
            pair.second = item->property("ID").toString();
        } else {
            pair.first = item->property("ID").toString();
            pair.second = "";
        }
        if (m_selected.contains(pair)) {
            if (value == Qt::Unchecked) m_selected.removeOne(pair);
        } else {
            if (value == Qt::Checked) m_selected.append(pair);
        }
        emit dataChanged(index, index);
        return true;
    } else if (role == Qt::CheckStateRole && index.column() != 0)
        return false;

    if ((index.column() >= 7) && (isAttribute)) return false;
    switch (index.column()) {
    case 0: result = item->setProperty("ID", value); break;
    case 1: result = item->setProperty("name", value); break;
    case 2: result = item->setProperty("description", value); break;
    case 3: result = item->setProperty("datatype", value); break;
    case 4: result = item->setProperty("datalength", value.toInt()); break;
    case 5: result = item->setProperty("indexed", value.toBool()); break;
    case 6: result = item->setProperty("nameValueList", value); break;
    case 7: result = item->setProperty("levelType", convertStringToLevelType(value.toString())); break;
    case 8: result = item->setProperty("parentLevelID", value); break;
    }

    if (result)
        emit dataChanged(index, index);

    return result;
}

bool AnnotationStructureTreeModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    if (m_readOnly) return false;

    QObject *parentItem = getItem(parent);
    AnnotationStructureLevel *level = qobject_cast<AnnotationStructureLevel *>(parentItem);

    if (parentItem == m_structure) {
        // inserting a Level
        beginInsertRows(parent, position, position + rows - 1);
        m_structure->addLevel(new AnnotationStructureLevel());
        endInsertRows();
        return true;
    }
    else if (level) {
        // inserting an attribute
        beginInsertRows(parent, position, position + rows - 1);
        level->addAttribute(new AnnotationStructureAttribute());
        endInsertRows();
        return true;
    }
    return false;
}

bool AnnotationStructureTreeModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    if (m_readOnly) return false;

    QObject *parentItem = getItem(parent);
    AnnotationStructureLevel *level = qobject_cast<AnnotationStructureLevel *>(parentItem);

    if (parentItem == m_structure) {
        // removing a level
        beginRemoveRows(parent, position, position + rows - 1);
        m_structure->removeLevelAt(position);
        endRemoveRows();
        return true;
    }
    else if (level) {
        // removing an attribute
        beginRemoveRows(parent, position, position + rows - 1);
        level->removeAttributeAt(position);
        endRemoveRows();
        return true;
    }
    return false;
}

QList<QPair<QString, QString> > AnnotationStructureTreeModel::selectedLevelsAttributes() const
{
    return m_selected;
}
