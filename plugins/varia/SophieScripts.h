#ifndef SOPHIESCRIPTS_H
#define SOPHIESCRIPTS_H

#include <QString>

namespace Praaline {
namespace Core {
class CorpusCommunication;
}
}

struct SophieScriptsData;

class SophieScripts
{
public:
    SophieScripts();
    ~SophieScripts();
    QString importPhonAnnotation(Praaline::Core::CorpusCommunication *com);

private:
    SophieScriptsData *d;
};

#endif // SOPHIESCRIPTS_H
