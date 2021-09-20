#include <QDebug>
#include "AnnotationElementTableModel.h"
#include "PraalineCore/Annotation/AnnotationElement.h"
using namespace Praaline::Core;

struct AnnotationTableModelData {
    AnnotationElement::ElementType elementType;
    QList<AnnotationElement *> elements;
    QStringList attributeIDs;
};

AnnotationElementTableModel::AnnotationElementTableModel(Praaline::Core::AnnotationStructureLevel *level,
                                                         QList<AnnotationElement *> elements,
                                                         QStringList attributeIDs, QObject *parent)
    : QAbstractTableModel(parent), d(new AnnotationTableModelData)
{
    d->elementType = AnnotationElement::Type_Element;
    if (level) {
        if (level->levelType() == AnnotationStructureLevel::IndependentPointsLevel)
            d->elementType = AnnotationElement::Type_Point;
        else if ((level->levelType() == AnnotationStructureLevel::IndependentIntervalsLevel) ||
                 (level->levelType() == AnnotationStructureLevel::GroupingLevel))
            d->elementType = AnnotationElement::Type_Interval;
        else if (level->levelType() == AnnotationStructureLevel::SequencesLevel)
            d->elementType = AnnotationElement::Type_Sequence;
        else if (level->levelType() == AnnotationStructureLevel::RelationsLevel)
            d->elementType = AnnotationElement::Type_Relation;
    }
    d->elements = elements;
    d->attributeIDs = attributeIDs;
    // qDebug() << d->elements.count() << d->attributeIDs.count();
}

AnnotationElementTableModel::~AnnotationElementTableModel()
{
    delete d;
}

QVariant AnnotationElementTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) return QVariant();
    if (orientation == Qt::Horizontal) {
        if      (section == 0) return tr("annotationID");
        else if (section == 1) return tr("speakerID");
        int offset = 0;
        switch (d->elementType) {
        case AnnotationElement::Type_Point:
            if      (section == 2) return (tr("indexNo"));
            else if (section == 3) return (tr("time_nsec"));
            else if (section == 4) return (tr("text"));
            offset = 5;
            break;
        case AnnotationElement::Type_Interval:
            if      (section == 2) return (tr("indexNo"));
            else if (section == 3) return (tr("tMin_nsec"));
            else if (section == 4) return (tr("tMax_nsec"));
            else if (section == 5) return (tr("text"));
            offset = 6;
            break;
        case AnnotationElement::Type_Sequence:
            if      (section == 2) return (tr("indexFrom"));
            else if (section == 3) return (tr("indexTo"));
            else if (section == 4) return (tr("text"));
            offset = 5;
            break;
        case AnnotationElement::Type_Relation:
            if      (section == 2) return (tr("indexFrom"));
            else if (section == 3) return (tr("indexTo"));
            else if (section == 4) return (tr("text"));
            offset = 5;
            break;
        case AnnotationElement::Type_Element:
        default:
            if      (section == 2) return (tr("text"));
            offset = 3;
        }
        if ((section - offset) >= 0 && (section - offset) < d->attributeIDs.count()) {
            return d->attributeIDs.at(section - offset);
        }
    }
    else if (orientation == Qt::Vertical) {
        QString ret = QString::number(section + 1);
        AnnotationElement *element(0);
        if ((section >= 0) && (section < d->elements.count())) {
            element = d->elements.at(section);
            if (element->isNew()) return ret.append(" +");
            if (element->isDirty()) return ret.append(" #");
        }
        return ret;
    }
    return QVariant();
}

int AnnotationElementTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return d->elements.count();
}

int AnnotationElementTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    switch (d->elementType) {
    case AnnotationElement::Type_Point:     return d->attributeIDs.count() + 5; break;
    case AnnotationElement::Type_Interval:  return d->attributeIDs.count() + 6; break;
    case AnnotationElement::Type_Sequence:  return d->attributeIDs.count() + 5; break;
    case AnnotationElement::Type_Relation:  return d->attributeIDs.count() + 5; break;
    case AnnotationElement::Type_Element:   return d->attributeIDs.count() + 3; break;
    }
    return d->attributeIDs.count() + 3;
}

QVariant AnnotationElementTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();
    if ((index.row() < 0 ) || (index.row() >= d->elements.count())) return QVariant();
    if ((role != Qt::DisplayRole) && (role != Qt::EditRole)) return QVariant();
    AnnotationElement *element = d->elements.at(index.row());
    if      (index.column() == 0) return element->attribute("annotationID");
    else if (index.column() == 1) return element->attribute("speakerID");
    int offset = 0;
    switch (d->elementType) {
    case AnnotationElement::Type_Point:
        if      (index.column() == 2) return element->attribute("indexNo");
        else if (index.column() == 3) return element->attribute("tMin_nsec");
        else if (index.column() == 4) return element->attribute("text");
        offset = 5;
        break;
    case AnnotationElement::Type_Interval:
        if      (index.column() == 2) return element->attribute("indexNo");
        else if (index.column() == 3) return element->attribute("tMin_nsec");
        else if (index.column() == 4) return element->attribute("tMax_nsec");
        else if (index.column() == 5) return element->attribute("text");
        offset = 6;
        break;
    case AnnotationElement::Type_Sequence:
        if      (index.column() == 2) return element->attribute("indexFrom");
        else if (index.column() == 3) return element->attribute("indexTo");
        else if (index.column() == 4) return element->attribute("text");
        offset = 5;
        break;
    case AnnotationElement::Type_Relation:
        if      (index.column() == 2) return element->attribute("indexFrom");
        else if (index.column() == 3) return element->attribute("indexTo");
        else if (index.column() == 4) return element->attribute("text");
        offset = 5;
        break;
    case AnnotationElement::Type_Element:
    default:
        if      (index.column() == 2) return element->attribute("text");
        offset = 3;
    }
    if ((index.column() - offset) >= 0 && (index.column() - offset) < d->attributeIDs.count()) {
        return element->attribute(d->attributeIDs.at(index.column()- offset));
    }
    return QVariant();
}

bool AnnotationElementTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) return false;
    if (index.row() < 0 || index.row() >= d->elements.count()) return false;
    if ((role == Qt::EditRole) && (data(index, role) != value)) {
        AnnotationElement *element = d->elements.at(index.row());
        QString attributeID;
        int offset = 0;
        switch (d->elementType) {
        case AnnotationElement::Type_Point:
            if      (index.column() == 2) return false; // indexNo
            else if (index.column() == 3) attributeID = "tMin_nsec";
            else if (index.column() == 4) attributeID = "text";
            offset = 5;
            break;
        case AnnotationElement::Type_Interval:
            if      (index.column() == 2) return false; // indexNo
            else if (index.column() == 3) attributeID = "tMin_nsec";
            else if (index.column() == 4) attributeID = "tMax_nsec";
            else if (index.column() == 5) attributeID = "text";
            offset = 6;
            break;
        case AnnotationElement::Type_Sequence:
            if      (index.column() == 2) return false; // indexFrom
            else if (index.column() == 3) return false; // indexTo
            else if (index.column() == 4) attributeID = "text";
            offset = 5;
            break;
        case AnnotationElement::Type_Relation:
            if      (index.column() == 2) return false; // indexFrom
            else if (index.column() == 3) return false; // indexTo
            else if (index.column() == 4) attributeID = "text";
            offset = 5;
            break;
        case AnnotationElement::Type_Element:
        default:
            if      (index.column() == 2) attributeID = "text";
            offset = 3;
        }
        if ((index.column() - offset) >= 0 && (index.column() - offset) < d->attributeIDs.count()) {
            attributeID = d->attributeIDs.at(index.column()- offset);
        }
        element->setAttribute(attributeID, value);
        emit dataChanged(index, index, QVector<int>() << role);
        return true;
    }
    return false;
}

Qt::ItemFlags AnnotationElementTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) return Qt::NoItemFlags;
    switch (d->elementType) {
    case AnnotationElement::Type_Point:
    case AnnotationElement::Type_Interval:
        if (index.column() >= 3) return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
        break;
    case AnnotationElement::Type_Sequence:
        if (index.column() >= 4) return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
        break;
    case AnnotationElement::Type_Relation:
        if (index.column() >= 4) return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
        break;
    case AnnotationElement::Type_Element:
    default:
        if (index.column() >= 2) return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
        break;
    }
    return QAbstractTableModel::flags(index);
}

void AnnotationElementTableModel::modelSavedInDatabase()
{
    emit headerDataChanged(Qt::Vertical, 0, d->elements.count());
}


