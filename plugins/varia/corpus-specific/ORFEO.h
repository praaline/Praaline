#ifndef ORFEO_H
#define ORFEO_H

#include <QString>
#include <QPointer>

namespace Praaline {
namespace Core {
class CorpusCommunication;
}
}

struct ORFEOData;

class ORFEO
{
public:
    ORFEO();
    ~ORFEO();

    QString updateSoundFiles(Praaline::Core::CorpusCommunication *com);
    QString readMetadata(Praaline::Core::CorpusCommunication *com);
    QString readOrfeoFile(Praaline::Core::CorpusCommunication *com);
    QString mapTokensToDisMo(Praaline::Core::CorpusCommunication *com);
    QString phonetise(Praaline::Core::CorpusCommunication *com);
    QString phonetiseOOV();
    QString createUtterances(Praaline::Core::CorpusCommunication *com);
    QString align(Praaline::Core::CorpusCommunication *com);

    QString createSentenceUnits(Praaline::Core::CorpusCommunication *com);

private:
    ORFEOData *d;
};

#endif // ORFEO_H
