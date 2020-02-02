#include <QPointer>
#include <QString>
#include <QList>
#include <QMap>
#include <QHash>

#include "PraalineCore/Corpus/Corpus.h"
#include "PraalineCore/Corpus/CorpusCommunication.h"
#include "PraalineCore/Annotation/IntervalTier.h"
#include "PraalineCore/Annotation/AnnotationTierGroup.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"
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
