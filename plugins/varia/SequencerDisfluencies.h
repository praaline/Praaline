#ifndef SEQUENCERDISFLUENCIES_H
#define SEQUENCERDISFLUENCIES_H

#include <QString>
#include <QList>
#include <QPointer>
#include "pncore/base/DataType.h"

namespace Praaline {
namespace Core {
class CorpusRepository;
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

    SequencerDisfluencies();
    ~SequencerDisfluencies();

    QStringList codesSkipped() const;
    void setCodesSkipped(const QStringList &codes);

    QStringList codesSingleToken() const;
    void setCodesSingleToken(const QStringList &codes);

    QStringList codesRepetitions() const;
    void setCodesRepetitions(const QStringList &codes);

    QStringList codesStructured() const;
    void setCodesStructured(const QStringList &codes);

    QStringList codesComplex() const;
    void setCodesComplex(const QStringList &codes);

    QString annotationLevel() const;
    void setAnnotationLevel(const QString &levelID);

    QString annotationAttribute() const;
    void setAnnotationAttribute(const QString &annotationID);

    QString sequencesLevel() const;
    void setSequencesLevel(const QString &levelID);

    QString getAllDistinctSequences(QPointer<Praaline::Core::CorpusCommunication> com);
    QString checkAnnotation(QPointer<Praaline::Core::CorpusCommunication> com);

private:
    SequencerDisfluenciesData *d;

    void addExtraDataToSequences(QList<Praaline::Core::Sequence *> sequences, QPointer<Praaline::Core::AnnotationTierGroup> tiers);

    void createDisfluencySequenceAnnotationLevel(Praaline::Core::CorpusRepository *repository);
    static void createAttribute(CorpusRepository *repository, AnnotationStructureLevel *level, const QString &prefix,
                                const QString &ID, const QString &name = QString(), const QString &description = QString(),
                                const Praaline::Core::DataType &datatype = Praaline::Core::DataType(Praaline::Core::DataType::VarChar, 256), int order = 0,
                                bool indexed = false, const QString &nameValueList = QString());
};

#endif // SEQUENCERDISFLUENCIES_H
