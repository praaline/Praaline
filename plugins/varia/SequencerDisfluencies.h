#ifndef SEQUENCERDISFLUENCIES_H
#define SEQUENCERDISFLUENCIES_H

#include <QString>
#include <QList>
#include <QPointer>

namespace Praaline {
namespace Core {
class CorpusCommunication;
class AnnotationTierGroup;
class Sequence;
}
}

struct SequencerDisfluenciesData;

class SequencerDisfluencies
{
public:
    enum StructuredDisfluencyState {
        Outside,
        Reparandum,
        Interregnum,
        Reparans
    };

    QStringList codesSkipped() const;
    QStringList codesSingleToken() const;
    QStringList codesRepetitions() const;
    QStringList codesStructured() const;
    QStringList codesComplex() const;

    SequencerDisfluencies();
    ~SequencerDisfluencies();

    QString getAllDistinctSequences(QPointer<Praaline::Core::CorpusCommunication> com);
    QString checkAnnotation(QPointer<Praaline::Core::CorpusCommunication> com);

private:
    SequencerDisfluenciesData *d;

    void addExtraDataToSequences(QList<Praaline::Core::Sequence *> sequences, QPointer<Praaline::Core::AnnotationTierGroup> tiers);
};

#endif // SEQUENCERDISFLUENCIES_H
