#include <QString>
#include <QList>
#include <QHash>
#include "annotationtier.h"

QString AnnotationTier::name() const
{
    return m_name;
}

void AnnotationTier::setName(const QString &name)
{
    if (m_name != name) {
        m_name = name;
        emit nameChanged();
    }
}

RealTime AnnotationTier::tMin() const
{
    return m_tMin;
}

RealTime AnnotationTier::tMax() const
{
    return m_tMax;
}

//int AnnotationTier::count() const
//{
//    return m_items.count();
//}

bool AnnotationTier::isEmpty() const
{
    return m_items.isEmpty();
}

QList<QString> AnnotationTier::getDistinctTextLabels() const
{
    QList<QString> ret;
    foreach (AnnotationElement *item, m_items) {
        if (!ret.contains(item->text()))
            ret.append(item->text());
    }
    return ret;
}

QList<QVariant> AnnotationTier::getDistinctAttributeValues(const QString &attributeID) const
{
    QList<QVariant> ret;
    foreach (AnnotationElement *item, m_items) {
        if (!ret.contains(item->attribute(attributeID)))
            ret.append(item->attribute(attributeID));
    }
    return ret;
}

void AnnotationTier::replaceTextLabels(const QString &before, const QString &after, Qt::CaseSensitivity cs)
{
    foreach (AnnotationElement *item, m_items) {
        item->replaceText(before, after, cs);
    }
}

void AnnotationTier::fillEmptyTextLabelsWith(const QString &filler)
{
    foreach (AnnotationElement *item, m_items) {
        if (item->text().isEmpty())
            item->setText(filler);
    }
}

void AnnotationTier::replaceAttributeText(const QString &attributeID, const QString &before, const QString &after, Qt::CaseSensitivity cs)
{
    foreach (AnnotationElement *item, m_items) {
        item->replaceAttributeText(attributeID, before, after, cs);
    }
}

void AnnotationTier::fillEmptyAttributeTextWith(const QString &attributeID,const QString &filler)
{
    foreach (AnnotationElement *item, m_items) {
        if (item->attribute(attributeID).toString().isEmpty())
            item->setAttribute(attributeID, filler);
    }
}

AnnotationElement *AnnotationTier::at(int index)
{
    return m_items.at(index);
}

AnnotationElement *AnnotationTier::first()
{
    if (isEmpty()) return 0;
    return m_items.first();
}

AnnotationElement *AnnotationTier::last()
{
    if (isEmpty()) return 0;
    return m_items.last();
}

QList<RealTime> AnnotationTier::times() const
{
    QList<RealTime> ret;
    foreach (AnnotationElement *item, m_items)
        ret << item->time();
    return ret;
}

void AnnotationTier::timeShift(const RealTime delta)
{
    m_tMin = m_tMin + delta;
    m_tMax = m_tMax + delta;
    foreach (AnnotationElement *item, m_items) {
        item->m_time = item->m_time + delta;
    }
}
