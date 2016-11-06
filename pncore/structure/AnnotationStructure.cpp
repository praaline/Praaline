#include <QObject>
#include <QString>
#include <QList>
#include "AnnotationStructure.h"

namespace Praaline {
namespace Core {

AnnotationStructure::AnnotationStructure(QObject *parent) :
    QObject(parent)
{
}

AnnotationStructure::~AnnotationStructure()
{
    qDeleteAll(m_levels);
}

// ==========================================================================================================
// Annotation levels
// ==========================================================================================================

AnnotationStructureLevel *AnnotationStructure::level(int index) const
{
    return m_levels.value(index);
}

AnnotationStructureLevel *AnnotationStructure::level(const QString &ID) const
{
    foreach (AnnotationStructureLevel *level, m_levels) {
        if (level->ID() == ID) return level;
    }
    return 0;
}

int AnnotationStructure::getLevelIndexByID(const QString &ID) const
{
    for (int i = 0; i < m_levels.count(); i++ ) {
        if (m_levels[i]->ID() == ID) return i;
    }
    return -1;
}

int AnnotationStructure::levelsCount() const
{
    return m_levels.count();
}

bool AnnotationStructure::hasLevel(const QString &ID) const
{
    return (getLevelIndexByID(ID) != -1);
}

bool AnnotationStructure::hasLevels() const
{
    return !m_levels.isEmpty();
}

QStringList AnnotationStructure::levelIDs() const
{
    QStringList ret;
    foreach (QPointer<AnnotationStructureLevel> level, m_levels)
        if (level) ret << level->ID();
    return ret;
}

QList<AnnotationStructureLevel *> AnnotationStructure::levels() const
{
    return m_levels;
}

void AnnotationStructure::insertLevel(int index, AnnotationStructureLevel *level)
{
    if (!level) return;
    if (hasLevel(level->ID())) return;
    level->setParent(this);
    m_levels.insert(index, level);
}

void AnnotationStructure::addLevel(AnnotationStructureLevel *level)
{
    if (!level) return;
    if (hasLevel(level->ID())) return;
    level->setParent(this);
    m_levels << level;
}

void AnnotationStructure::swapLevels(int oldIndex, int newIndex)
{
    m_levels.swap(oldIndex, newIndex);
}

void AnnotationStructure::removeLevelAt(int i)
{
    m_levels.removeAt(i);
}

void AnnotationStructure::removeLevelByID(const QString &ID)
{
    int i = getLevelIndexByID(ID);
    if (i != -1)
        m_levels.removeAt(i);
}


// ==========================================================================================================
// Management
// ==========================================================================================================

void AnnotationStructure::clear()
{
    m_ID = "";
    qDeleteAll(m_levels);
    m_levels.clear();
    emit AnnotationStructureChanged();
}

} // namespace Core
} // namespace Praaline
