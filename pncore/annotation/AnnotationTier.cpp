#include <QString>
#include <QList>
#include <QHash>
#include "AnnotationTier.h"

namespace Praaline {
namespace Core {

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

} // namespace Core
} // namespace Praaline
