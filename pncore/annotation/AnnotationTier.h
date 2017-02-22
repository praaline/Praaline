#ifndef ANNOTATIONTIER_H
#define ANNOTATIONTIER_H

/*
    Praaline - Core module - Annotation
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
#include <QString>
#include <QHash>
#include <QList>
#include "base/RealTime.h"

class QTextStream;

namespace Praaline {
namespace Core {

class AnnotationElement;

class PRAALINE_CORE_SHARED_EXPORT AnnotationTier : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(RealTime tMin READ tMin)
    Q_PROPERTY(RealTime tMax READ tMax)

public:
    enum TierType {
        TierType_Points,
        TierType_Intervals,
        TierType_Grouping,
        TierType_Sequences,
        TierType_Tree,
        TierType_Relations
    };

    AnnotationTier(QObject *parent = 0) : QObject(parent) {}
    virtual ~AnnotationTier() {}

    virtual TierType tierType() const = 0;
    virtual QString name() const;
    virtual void setName(const QString &name);
    virtual RealTime tMin() const;
    virtual RealTime tMax() const;

    virtual int count() const = 0;
    virtual bool isEmpty() const = 0;
    virtual void clear() = 0;
    virtual AnnotationElement *at(int index) const = 0;
    virtual AnnotationElement *first() const = 0;
    virtual AnnotationElement *last() const = 0;

    virtual QStringList getDistinctLabels() const = 0;
    virtual QList<QVariant> getDistinctValues(const QString &attributeID) const = 0;
    virtual void replace(const QString &attributeID, const QString &before, const QString &after, Qt::CaseSensitivity cs = Qt::CaseSensitive) = 0;
    virtual void fillEmptyWith(const QString &attributeID, const QString &filler) = 0;

signals:
    void nameChanged();

protected:
    QString m_name;
    RealTime m_tMin;
    RealTime m_tMax;
};

} // namespace Core
} // namespace Praaline

#endif // ANNOTATIONTIER_H
