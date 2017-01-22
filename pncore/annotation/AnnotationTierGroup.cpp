#include <QObject>
#include <QString>
#include <QList>
#include <QPair>
#include <QStringList>
#include "base/RealTime.h"
#include "AnnotationTier.h"
#include "PointTier.h"
#include "IntervalTier.h"
#include "SequenceTier.h"
#include "TreeTier.h"
#include "RelationTier.h"
#include "AnnotationTierGroup.h"

namespace Praaline {
namespace Core {

// ==========================================================================================================
// Constructors - destructors
// ==========================================================================================================
AnnotationTierGroup::AnnotationTierGroup(QObject *parent) :
    QObject(parent)
{
    m_ID = QString();
}

AnnotationTierGroup::~AnnotationTierGroup()
{
   // All tiers should be deleted as QObjects having this as their parent.
}

// ==========================================================================================================
// Global tMin and tMax: find it by checking all tiers in this annotation group
// ==========================================================================================================
RealTime AnnotationTierGroup::tMin() const
{
    AnnotationTier *tier;
    RealTime tMin = RealTime(0, 0);
    foreach (tier, m_tiers) {
        if (tier->tMin() < tMin)
            tMin = tier->tMin();
    }
    return tMin;
}
RealTime AnnotationTierGroup::tMax() const
{
    AnnotationTier *tier;
    RealTime tMax = RealTime(0, 0);
    foreach (tier, m_tiers) {
        if (tier->tMax() > tMax)
            tMax = tier->tMax();
    }
    return tMax;
}

// ==========================================================================================================
// Tier accessors and modifiers
// ==========================================================================================================
AnnotationTier *AnnotationTierGroup::tier(int index) const
{
    return m_tiers.value(index);
}

AnnotationTier *AnnotationTierGroup::tier(const QString &name) const
{
    foreach (AnnotationTier *tier, m_tiers) {
        if (tier->name() == name) return tier;
    }
    return Q_NULLPTR;
}

int AnnotationTierGroup::tiersCount() const
{
    return m_tiers.count();
}

bool AnnotationTierGroup::hasTiers() const
{
    return !m_tiers.isEmpty();
}

QList<AnnotationTier *> AnnotationTierGroup::tiers() const
{
    return m_tiers;
}

void AnnotationTierGroup::insertTier(int index, AnnotationTier *tier)
{
    if (!tier) return;
    tier->setParent(this);
    m_tiers.insert(index, tier);
    emit tierInserted(tier);
}

void AnnotationTierGroup::insertTierReplacing(int index, AnnotationTier *tier)
{
    if (!tier) return;
    // if a tier with the same name already exists, it will be replaced
    this->removeTierByName(tier->name());
    tier->setParent(this);
    m_tiers.insert(index, tier);
    emit tierInserted(tier);
}

void AnnotationTierGroup::addTier(AnnotationTier *tier)
{
    if (!tier) return;
    tier->setParent(this);
    m_tiers << tier;
    emit tierInserted(tier);
}

void AnnotationTierGroup::addTierReplacing(AnnotationTier *tier)
{
    if (!tier) return;
    // if a tier with the same name already exists, it will be replaced
    this->removeTierByName(tier->name());
    tier->setParent(this);
    m_tiers << tier;
    emit tierInserted(tier);
}

void AnnotationTierGroup::swapTiers(int oldIndex, int newIndex)
{
    m_tiers.swap(oldIndex, newIndex);
}

void AnnotationTierGroup::removeTierAt(int i)
{
    AnnotationTier *tier = m_tiers.at(i);
    if (!tier) return;
    QString name = tier->name();
    m_tiers.removeAt(i);
    delete tier;
    emit tierDeleted(name);
}

void AnnotationTierGroup::removeTierByName(const QString &name)
{
    int i = getTierIndexByName(name);
    if (i < 0) return;
    AnnotationTier *tier = m_tiers.at(i);
    m_tiers.removeAt(i);
    delete tier;
    emit tierDeleted(name);
}

int AnnotationTierGroup::getTierIndexByName(const QString &name) const
{
    for (int i = 0; i < m_tiers.count(); i++ ) {
        if (m_tiers[i]->name() == name)
            return i;
    }
    return -1;
}

IntervalTier *AnnotationTierGroup::getIntervalTierByName(const QString &name) const
{
    AnnotationTier *tier;
    foreach (tier, m_tiers) {
        if (tier->name() == name && tier->tierType() == AnnotationTier::TierType_Intervals)
            return (IntervalTier *)tier;
    }
    return 0;
}

IntervalTier *AnnotationTierGroup::getIntervalTierByIndex(int index) const
{
    if (index < 0 || index >= m_tiers.count()) return 0;
    AnnotationTier *tier = m_tiers.at(index);
    if (tier->tierType() == AnnotationTier::TierType_Intervals)
        return (IntervalTier *)tier;
    else
        return 0;
}

PointTier *AnnotationTierGroup::getPointTierByName(const QString &name) const
{
    AnnotationTier *tier;
    foreach (tier, m_tiers) {
        if (tier->name() == name && tier->tierType() == AnnotationTier::TierType_Points)
            return (PointTier *)tier;
    }
    return 0;
}

PointTier *AnnotationTierGroup::getPointTierByIndex(int index) const
{
    if (index < 0 || index >= m_tiers.count()) return 0;
    AnnotationTier *tier = m_tiers.at(index);
    if (tier->tierType() == AnnotationTier::TierType_Points)
        return (PointTier *)tier;
    else
        return 0;
}

QStringList AnnotationTierGroup::tierNames() const
{
    QStringList ret;
    for (int i = 0; i < m_tiers.count(); i++ ) {
        if (m_tiers[i])
            ret << m_tiers[i]->name();
    }
    return ret;
}

// Methods
void AnnotationTierGroup::insertTierClone(int index, const AnnotationTier *tier, const QString &newName)
{
    if (!tier) return;
    if (tier->tierType() == AnnotationTier::TierType_Intervals) {
        IntervalTier *newTier = new IntervalTier((IntervalTier *)tier, newName, this);
        newTier->setParent(this);
        m_tiers.insert(index, newTier);
        emit tierInserted(newTier);
    } else {
        PointTier *newTier = new PointTier((PointTier *)tier, newName, this);
        newTier->setParent(this);
        m_tiers.insert(index, newTier);
        emit tierInserted(newTier);
    }
}

void AnnotationTierGroup::renameTier(const QString &nameBefore, const QString &nameAfter)
{
    int i = this->getTierIndexByName(nameBefore);
    if (i < 0) return; // not found
    m_tiers[i]->setName(nameAfter);
}

bool AnnotationTierGroup::reorderTiers(QStringList tierNamesInNewOrder) {
    QList<AnnotationTier *> new_tiers;
    foreach (QString tierName, tierNamesInNewOrder) {
        int index = getTierIndexByName(tierName);
        if (index >= 0) {
            new_tiers << m_tiers[index];
        }
    }
    m_tiers = new_tiers;
    return true;
}

} // namespace Core
} // namespace Praaline
