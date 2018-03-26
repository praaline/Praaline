#include <QString>
#include <QList>
#include <QPointer>
#include <QMap>

#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
using namespace Praaline::Core;

#include "PFCAligner.h"

struct PFCAlignerData {
    int i;
};

PFCAligner::PFCAligner() : d(new PFCAlignerData())
{
}

PFCAligner::~PFCAligner()
{
    delete d;
}
