#include <QPointer>
#include <QString>
#include <QList>
#include <QMap>
#include <QHash>

#include "pncore/corpus/Corpus.h"
#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
using namespace Praaline::Core;

#include "CorpusCommunicationSplicer.h"

struct CorpusCommunicationSplicerData {
    CorpusCommunicationSplicerData() :
        repositorySource(0), repositoryDestination(0)
    { }
    CorpusRepository *repositorySource;
    CorpusRepository *repositoryDestination;
};

CorpusCommunicationSplicer::CorpusCommunicationSplicer() :
    d(new CorpusCommunicationSplicerData)
{
}

CorpusCommunicationSplicer::~CorpusCommunicationSplicer()
{
    delete d;
}

void CorpusCommunicationSplicer::setRepositorySource(CorpusRepository *repository)
{

}

void CorpusCommunicationSplicer::setRepositoryDestination(CorpusRepository *repository)
{

}
