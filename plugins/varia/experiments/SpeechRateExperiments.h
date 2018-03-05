#ifndef SPEECHRATEEXPERIMENTS_H
#define SPEECHRATEEXPERIMENTS_H

#include <QString>

namespace Praaline {
namespace Core {
class CorpusRepository;
}
}

class SpeechRateExperiments
{
public:
    SpeechRateExperiments();

    bool readResultsFile(Praaline::Core::CorpusRepository *repository, const QString &filename);
};

#endif // SPEECHRATEEXPERIMENTS_H
