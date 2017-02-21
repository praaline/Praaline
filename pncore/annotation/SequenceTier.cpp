#include <QString>
#include <QTextStream>
#include "Sequence.h"
#include "SequenceTier.h"
#include "IntervalTier.h"
#include "AnnotationTier.h"

namespace Praaline {
namespace Core {

// ==========================================================================================================
// Constructors - destructor
// ==========================================================================================================
SequenceTier::SequenceTier(const QString &name, AnnotationTier *baseTier, QObject *parent) :
    AnnotationTier(parent), m_baseTier(baseTier)
{
    m_name = name;
}

SequenceTier::SequenceTier(const QString &name, const QList<Sequence *> &sequences, AnnotationTier *baseTier, QObject *parent) :
    AnnotationTier(parent), m_baseTier(baseTier)
{
    m_name = name;
    m_sequences = sequences;
    if (m_sequences.count() == 0) return;
    qSort(m_sequences.begin(), m_sequences.end(), SequenceTier::compareSequences);
}

SequenceTier::SequenceTier(const SequenceTier *copy, QString name, QObject *parent) :
    AnnotationTier(parent), m_baseTier(0)
{
    if (!copy) return;
    m_name = (name.isEmpty()) ? copy->name() : name;
    m_baseTier = copy->baseTier();
    // deep copy of Sequences
    foreach (Sequence *sequence, copy->sequences()) {
        m_sequences << new Sequence(*sequence);
    }
}

SequenceTier::~SequenceTier()
{
    qDeleteAll(m_sequences);
}

// ==============================================================================================================================
// Implementation of AnnotationTier
// ==============================================================================================================================

RealTime SequenceTier::tMin() const
{
    if (m_baseTier) return m_baseTier->tMin();
    return m_tMin;
}

RealTime SequenceTier::tMax() const
{
    if (m_baseTier) return m_baseTier->tMax();
    return m_tMax;
}

bool SequenceTier::isEmpty() const
{
    return m_sequences.isEmpty();
}

void SequenceTier::clear()
{
    qDeleteAll(m_sequences);
    m_sequences.clear();
}

Sequence *SequenceTier::at(int index) const
{
    return m_sequences.at(index);
}

Sequence *SequenceTier::first() const
{
    if (isEmpty()) return Q_NULLPTR;
    return m_sequences.first();
}

Sequence *SequenceTier::last() const
{
    if (isEmpty()) return Q_NULLPTR;
    return m_sequences.last();
}

QStringList SequenceTier::getDistinctLabels() const
{
    QStringList ret;
    foreach (Sequence *sequence, m_sequences) {
        if (!ret.contains(sequence->text())) ret << sequence->text();
    }
    return ret;
}

QList<QVariant> SequenceTier::getDistinctValues(const QString &attributeID) const
{
    QList<QVariant> ret;
    foreach (Sequence *sequence, m_sequences) {
        QVariant value = (attributeID.isEmpty()) ? sequence->text() : sequence->attribute(attributeID);
        if (!ret.contains(value)) ret << value;
    }
    return ret;
}

void SequenceTier::replace(const QString &attributeID, const QString &before, const QString &after, Qt::CaseSensitivity cs)
{
    foreach (Sequence *sequence, m_sequences) {
        if (attributeID.isEmpty())
            sequence->replaceText(before, after, cs);
        else
            sequence->replaceAttributeText(attributeID, before, after, cs);
    }
}

void SequenceTier::fillEmptyWith(const QString &attributeID, const QString &filler)
{
    foreach (Sequence *sequence, m_sequences) {
        if (attributeID.isEmpty()) {
            if (sequence->text().isEmpty())
                sequence->setText(filler);
        } else {
            if (sequence->attribute(attributeID).toString().isEmpty())
                sequence->setAttribute(attributeID, filler);
        }
    }
}

// ==============================================================================================================================
// Base tier
// ==============================================================================================================================

AnnotationTier *SequenceTier::baseTier() const
{
    return m_baseTier;
}

void SequenceTier::setBaseTier(AnnotationTier *tier)
{
    m_baseTier = tier;
}

// ==============================================================================================================================
// Accessors and mutators for Sequences
// ==============================================================================================================================

Sequence *SequenceTier::sequence(int index) const
{
    return m_sequences.value(index);
}

QList<Sequence *> SequenceTier::sequences() const
{
    return m_sequences;
}

bool SequenceTier::compareSequences(Sequence *A, Sequence *B) {
    if (A->indexFrom() == B->indexFrom())
        return (A->indexTo() < B->indexTo());
    // else
    return (A->indexFrom() < B->indexFrom());
}

void SequenceTier::addSequence(Sequence *sequence)
{
    m_sequences << sequence;
    qSort(m_sequences.begin(), m_sequences.end(), SequenceTier::compareSequences);
}

void SequenceTier::addSequences(QList<Sequence *> sequences)
{
    m_sequences << sequences;
    qSort(m_sequences.begin(), m_sequences.end(), SequenceTier::compareSequences);
}

void SequenceTier::removeSequenceAt(int i)
{
    m_sequences.removeAt(i);
}

QList<AnnotationElement *> SequenceTier::sequenceElements(int sequenceIndex) const
{
    QList<AnnotationElement *> elements;
    if (!m_baseTier) return elements;
    if (sequenceIndex < 0 || sequenceIndex >= m_sequences.count()) {
        int from = m_sequences.at(sequenceIndex)->indexFrom();
        int to = m_sequences.at(sequenceIndex)->indexTo();
        for (int i = from; i <= to; ++i) {
            if (i > 0 && i < m_baseTier->count()) elements << m_baseTier->at(i);
        }
    }
    return elements;
}

} // namespace Core
} // namespace Praaline
