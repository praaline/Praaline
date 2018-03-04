#ifndef PROSODICBOUNDARIES_H
#define PROSODICBOUNDARIES_H

#include <QString>
#include <QPair>
#include <QTextStream>

#include "pncore/corpus/Corpus.h"
#include "pncore/annotation/IntervalTier.h"

struct ProsodicBoundariesData;

class ProsodicBoundaries
{
public:
    ProsodicBoundaries();
    ~ProsodicBoundaries();

    QStringList additionalAttributeIDs() const;
    void setAdditionalAttributeIDs(const QStringList &attributeIDs);

    bool isLexical(Praaline::Core::Interval *token) const;
    QString categorise_CLI_INT_LEX(Praaline::Core::Interval *token) const;

    /// Analyse syllables from a given list of indices and return results in a QStringList
    QStringList analyseBoundaryListToStrings(Praaline::Core::Corpus *corpus, const QString &annotationID,
                                             const QString &speakerID, QList<int> syllIndices);

    /// Analyse syllables from a given list of indices and write results to a stream
    void analyseBoundaryListToStream(QTextStream &out, Praaline::Core::Corpus *corpus, const QString &annotationID,
                                     const QString &speakerID, QList<int> syllIndices);

    /// Analyse all syllables in a given annotation and return results in a QStringList
    QStringList analyseAnnotationToStrings(Praaline::Core::Corpus *corpus, const QString &annotationID);

    /// Analyse all syllables in a given annotation and write results to a stream
    void analyseAnnotationToStream(QTextStream &out, Praaline::Core::Corpus *corpus, const QString &annotationID);

    /// Analyse syllables at the end of sequences and return results in a QStringList
    QStringList analyseLastSyllOfSequenceToStrings(Praaline::Core::Corpus *corpus, const QString &annotationID,
                                                   const QString &sequenceLevelID);

    /// Analyse sylalbles at the end of sequences and write results to a stream
    void analyseLastSyllOfSequenceToStream(QTextStream &out, Praaline::Core::Corpus *corpus, const QString &annotationID,
                                           const QString &sequenceLevelID);

    QString headerLineForTables() const;
private:
    ProsodicBoundariesData *d;
};

#endif // PROSODICBOUNDARIES_H
