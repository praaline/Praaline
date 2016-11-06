#ifndef CHUNKANNOTATOR_H
#define CHUNKANNOTATOR_H


#include <QObject>
#include <QString>
#include <QList>
#include "pncore/corpus/Corpus.h"
#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/corpus/CorpusAnnotation.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/annotation/AnnotationTierGroup.h"
using namespace Praaline::Core;
#include "crfannotator.h"

class ChunkAnnotator : public CRFAnnotator
{
public:
    ChunkAnnotator(IntervalTier *tier_tokens, IntervalTier *tier_chunks, const QString &attributePOS);
    ~ChunkAnnotator();

    IntervalTier *getChunkTier();

    void annotate(const QString &filenameModel);
    void writeTraining(QTextStream &out);

protected:
    virtual bool isEndOfSequence(int i);
    virtual bool skipUnit(int i);
    virtual void writeUnitToCRF(QTextStream &out, int i, bool isTraining);
    virtual void decodeUnitFromCRF(const QString &line, int i);

private:
    IntervalTier *m_tier_tokens;
    IntervalTier *m_tier_chunks;
    QString m_attributePOS;
    bool m_usingBIO;
};

#endif // CHUNKANNOTATOR_H
