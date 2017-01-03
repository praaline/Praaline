#include "AnnotationElement.h"

namespace Praaline {
namespace Core {

AnnotationElement::AnnotationElement()
{
}

AnnotationElement::AnnotationElement(const QString &text) :
    m_text(text)
{
}

AnnotationElement::AnnotationElement(const QString &text, const QHash<QString, QVariant> &attributes) :
    m_text(text), m_attributes(attributes)
{
}

AnnotationElement::AnnotationElement(const AnnotationElement &other) :
    m_text(other.m_text), m_attributes(other.m_attributes)
{
}

} // namespace Core
} // namespace Praaline
