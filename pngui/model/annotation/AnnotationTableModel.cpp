#include <QDebug>
#include "AnnotationTableModel.h"
#include "pncore/annotation/AnnotationElement.h"
using namespace Praaline::Core;

struct AnnotationTableModelData {
    AnnotationElement::ElementType elementType;
    QList<AnnotationElement *> elements;
    QStringList attributeIDs;
};

AnnotationTableModel::AnnotationTableModel(AnnotationElement::ElementType elementType,
                                           QList<AnnotationElement *> elements,
                                           QStringList attributeIDs, QObject *parent)
    : QAbstractTableModel(parent), d(new AnnotationTableModelData)
{
    d->elementType = elementType;
    d->elements = elements;
    d->attributeIDs = attributeIDs;
    qDebug() << d->elements.count() << d->attributeIDs.count();
}

AnnotationTableModel::~AnnotationTableModel()
{
    delete d;
}

QVariant AnnotationTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) return QVariant();
    if (orientation != Qt::Horizontal) return QVariant();
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
    return QVariant();
}

int AnnotationTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return d->elements.count();
}

int AnnotationTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    switch (d->elementType) {
    case AnnotationElement::Type_Point:     return d->attributeIDs.count() + 5; break;
    case AnnotationElement::Type_Interval:  return d->attributeIDs.count() + 6; break;
    case AnnotationElement::Type_Sequence:
    case AnnotationElement::Type_Relation:  return d->attributeIDs.count() + 5; break;
    case AnnotationElement::Type_Element:   return d->attributeIDs.count() + 3; break;
    }
    return d->attributeIDs.count() + 3;
}

QVariant AnnotationTableModel::data(const QModelIndex &index, int role) const
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

bool AnnotationTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
//    if (data(index, role) != value) {
//        emit dataChanged(index, index, QVector<int>() << role);
//        return true;
//    }
    return false;
}

Qt::ItemFlags AnnotationTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) return Qt::NoItemFlags;
    return QAbstractTableModel::flags(index); // | Qt::ItemIsEditable;
}
