#include <QString>
#include <QTextStream>
#include "SequenceTier.h"
#include "IntervalTier.h"
#include "AnnotationTier.h"

namespace Praaline {
namespace Core {

// ==========================================================================================================
// Constructors - destructor
// ==========================================================================================================
SequenceTier::SequenceTier(const QString &name, QObject *parent) :
    AnnotationTier(parent)
{
    m_name = name;
}

SequenceTier::SequenceTier(const QString &name, const QList<Sequence *> &sequences, QObject *parent) :
    AnnotationTier(parent)
{
    m_name = name;
    m_sequences = sequences;
    if (m_sequences.count() == 0) return;
    qSort(m_sequences.begin(), m_sequences.end(), SequenceTier::compareSequences);
}

SequenceTier::SequenceTier(const SequenceTier *copy, QString name, QObject *parent) :
    AnnotationTier(parent)
{
    if (!copy) return;
    m_name = (name.isEmpty()) ? copy->name() : name;
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

QList<QString> SequenceTier::getDistinctTextLabels() const
{
    QList<QString> ret;
    foreach (Sequence *sequence, m_sequences) {
        if (!ret.contains(sequence->text())) ret << sequence->text();
    }
    return ret;
}

QList<QVariant> SequenceTier::getDistinctAttributeValues(const QString &attributeID) const
{
    QList<QVariant> ret;
    foreach (Sequence *sequence, m_sequences) {
        if (!ret.contains(sequence->attribute(attributeID))) ret.append(sequence->attribute(attributeID));
    }
    return ret;
}

void SequenceTier::replaceTextLabels(const QString &before, const QString &after, Qt::CaseSensitivity cs)
{
    foreach (Sequence *sequence, m_sequences) {
        sequence->replaceText(before, after, cs);
    }
}

void SequenceTier::fillEmptyTextLabelsWith(const QString &filler)
{
    foreach (Sequence *sequence, m_sequences) {
        if (sequence->text().isEmpty()) sequence->setText(filler);
    }
}


void SequenceTier::replaceAttributeText(const QString &attributeID, const QString &before, const QString &after, Qt::CaseSensitivity cs)
{
    foreach (Sequence *sequence, m_sequences) {
        sequence->replaceAttributeText(attributeID, before, after, cs);
    }
}

void SequenceTier::fillEmptyAttributeTextWith(const QString &attributeID,const QString &filler)
{
    foreach (Sequence *sequence, m_sequences) {
        if (sequence->attribute(attributeID).toString().isEmpty())
            sequence->setAttribute(attributeID, filler);
    }
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

} // namespace Core
} // namespace Praaline
