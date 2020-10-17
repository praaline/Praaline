#ifndef METADATADJANGOMODELS_H
#define METADATADJANGOMODELS_H

#include <memory>
#include <QString>

namespace Praaline {
namespace Core {
class MetadataStructure;
}
}

struct MetadataDjangoModelsData;

class MetadataDjangoModels
{
public:
    MetadataDjangoModels();
    ~MetadataDjangoModels();

    QString modelCorpus() const;
    QString modelCorpusCommunication() const;
    QString modelCorpusSpeaker() const;
    QString modelCorpusParticipation() const;
    QString modelCorpusRecording() const;
    QString modelCorpusAnnotation() const;

    void setMetadataStructure(Praaline::Core::MetadataStructure *structure);

private:
    std::unique_ptr<MetadataDjangoModelsData> d;
};

#endif // METADATADJANGOMODELS_H
