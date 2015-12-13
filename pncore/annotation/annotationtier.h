#ifndef ANNOTATIONTIER_H
#define ANNOTATIONTIER_H
/*
    Praaline - Annotation module
    Copyright (c) 2011-12 George Christodoulides

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
#include "annotationelement.h"

class QTextStream;

class PRAALINE_CORE_SHARED_EXPORT AnnotationTier : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(RealTime tMin READ tMin)
    Q_PROPERTY(RealTime tMax READ tMax)

public:
    enum TierType {
        TierType_Intervals,
        TierType_Points,
        TierType_Grouping,
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
    virtual int count() const = 0; // TODO : change that
    virtual bool isEmpty() const;

    virtual QList<QString> getDistinctTextLabels() const;
    virtual QList<QVariant> getDistinctAttributeValues(const QString &attributeID) const;

    virtual void replaceTextLabels(const QString &before, const QString &after, Qt::CaseSensitivity cs = Qt::CaseSensitive);
    virtual void fillEmptyTextLabelsWith(const QString &filler);
    virtual void replaceAttributeText(const QString &attributeID, const QString &before, const QString &after, Qt::CaseSensitivity cs = Qt::CaseSensitive);
    virtual void fillEmptyAttributeTextWith(const QString &attributeID,const QString &filler);

    virtual AnnotationElement *at(int index);
    virtual AnnotationElement *first();
    virtual AnnotationElement *last();

    virtual QList<RealTime> times() const;
    virtual void timeShift(const RealTime delta);

signals:
    void nameChanged();

protected:
    QString m_name;
    RealTime m_tMin;
    RealTime m_tMax;
    QList<AnnotationElement *> m_items;
};

#endif // ANNOTATIONTIER_H
