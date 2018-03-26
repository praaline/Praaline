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

#include "PFCPhonetiser.h"

struct PFCPhonetiserData {
    int i;
};

PFCPhonetiser::PFCPhonetiser() : d(new PFCPhonetiserData())
{
}

PFCPhonetiser::~PFCPhonetiser()
{
    delete d;
}

QString PFCPhonetiser::phonetiseFromDictionary(QPointer<CorpusCommunication> com)
{

}

QString PFCPhonetiser::phonetiseFromAdditionalList(QPointer<CorpusCommunication> com)
{

}

QString PFCPhonetiser::applyPFCLiaison(QPointer<CorpusCommunication> com)
{

}

QString PFCPhonetiser::applyPFCSchwa(QPointer<CorpusCommunication> com)
{

}
