#ifndef ANNOTATIONTIERGROUP_H
#define ANNOTATIONTIERGROUP_H

#include "pncore_global.h"
#include <QObject>
#include <QString>
#include <QList>
#include <QPair>
#include <QStringList>
#include "base/RealTime.h"
#include "AnnotationTier.h"
#include "IntervalTier.h"
#include "PointTier.h"

namespace Praaline {
namespace Core {

class PRAALINE_CORE_SHARED_EXPORT AnnotationTierGroup : public QObject
{
    Q_OBJECT
public:
    explicit AnnotationTierGroup(QObject *parent = 0);
    virtual ~AnnotationTierGroup();

    QString ID() const { return m_ID; }
    void setID(const QString &ID) { m_ID = ID; }

    RealTime tMin() const;
    RealTime tMax() const;
    RealTime duration() const { return tMax() - tMin(); }

    AnnotationTier *tier(int index) const;
    int tiersCount() const;
    bool hasTiers() const;
    QList<AnnotationTier *> tiers() const;
    void insertTier(int index, AnnotationTier *tier);
    void insertTierReplacing(int index, AnnotationTier *tier);
    void addTier(AnnotationTier *tier);
    void addTierReplacing(AnnotationTier *tier);
    void swapTiers(int oldIndex, int newIndex);
    void removeTierAt(int i);
    void removeTierByName(const QString &name);
    void renameTier(const QString &nameBefore, const QString &nameAfter);

    // Tier accessors
    IntervalTier *getIntervalTierByName(const QString &name) const;
    IntervalTier *getIntervalTierByIndex(int index) const;
    PointTier *getPointTierByName(const QString &name) const;
    PointTier *getPointTierByIndex(int index) const;
    int getTierIndexByName(const QString &name) const;
    QStringList tierNames() const;

    // Methods
    void insertTierClone(int index, const AnnotationTier *tier, const QString &newName);
    bool reorderTiers(QStringList tierNamesInNewOrder);

signals:
    void tierInserted(AnnotationTier *tier);
    void tierDeleted(QString tierName);
    
public slots:

protected:
    QString m_ID;
    QList<AnnotationTier *> m_tiers;
    QString annotationID;
    QString speakerID;
};

} // namespace Core
} // namespace Praaline

#endif // ANNOTATIONTIERGROUP_H
