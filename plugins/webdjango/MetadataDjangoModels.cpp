#include "MetadataDjangoModels.h"

#include <memory>
#include <QString>

#include "PraalineCore/Structure/MetadataStructure.h"
using namespace Praaline::Core;


struct MetadataDjangoModelsData {
    MetadataStructure *structure;
};

MetadataDjangoModels::MetadataDjangoModels()
{
}

MetadataDjangoModels::~MetadataDjangoModels()
{
}

void MetadataDjangoModels::setMetadataStructure(Praaline::Core::MetadataStructure *structure)
{
    d->structure = structure;
}


QString MetadataDjangoModels::modelCorpus() const
{
    QString ret;
    return ret;
}

QString MetadataDjangoModels::modelCorpusCommunication() const
{
    QString ret;
    return ret;
}

QString MetadataDjangoModels::modelCorpusSpeaker() const
{
    QString ret;
    return ret;
}

QString MetadataDjangoModels::modelCorpusParticipation() const
{
    QString ret;
    return ret;
}

QString MetadataDjangoModels::modelCorpusRecording() const
{
    QString ret;
    return ret;
}

QString MetadataDjangoModels::modelCorpusAnnotation() const
{
    QString ret;
    return ret;
}
