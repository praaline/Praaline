#ifndef SEQUENCETIER_H
#define SEQUENCETIER_H

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
#include <QList>
#include "AnnotationTier.h"
#include "Sequence.h"

namespace Praaline {
namespace Core {

class PRAALINE_CORE_SHARED_EXPORT SequenceTier : public AnnotationTier
{
    Q_OBJECT
public:
    // Constructors, destructor
    SequenceTier(const QString &name = QString(), AnnotationTier *baseTier = 0, QObject *parent = 0);
    SequenceTier(const QString &name, const QList<Sequence *> &sequences, AnnotationTier *baseTier = 0, QObject *parent = 0);
    SequenceTier(const SequenceTier *copy, QString name = QString(), QObject *parent = 0);
    virtual ~SequenceTier();

    // Implementation of AnnotationTier
    AnnotationTier::TierType tierType() const override
        { return AnnotationTier::TierType_Sequences; }
    int count() const override
        { return m_sequences.count(); }
    RealTime tMin() const override;
    RealTime tMax() const override;
    bool isEmpty() const override;
    void clear() override;
    Sequence *at(int index) const override;
    Sequence *first() const override;
    Sequence *last() const override;
    QStringList getDistinctLabels() const override;
    QList<QVariant> getDistinctValues(const QString &attributeID) const override;
    void replace(const QString &attributeID, const QString &before, const QString &after, Qt::CaseSensitivity cs = Qt::CaseSensitive) override;
    void fillEmptyWith(const QString &attributeID,const QString &filler) override;

    // Base tier
    AnnotationTier *baseTier() const;
    void setBaseTier(AnnotationTier *tier);

    // Accessors for Sequences
    Sequence* sequence(int index) const;
    QList<Sequence *> sequences() const;

    // Mutators for Sequences
    void addSequence(Sequence *sequence);
    void addSequences(QList<Sequence *> sequences);
    void removeSequenceAt(int i);

    // Access to base annotation elements
    QList<AnnotationElement *> sequenceElements(int sequenceIndex) const;

protected:
    AnnotationTier *m_baseTier;
    QList<Sequence *> m_sequences;

private:
    static bool compareSequences(Sequence *A, Sequence *B);
};

} // namespace Core
} // namespace Praaline

#endif // SEQUENCETIER_H
