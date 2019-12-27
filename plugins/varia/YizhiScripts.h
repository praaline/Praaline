#ifndef YIZHISCRIPTS_H
#define YIZHISCRIPTS_H

#include <QString>
#include <QPointer>

namespace Praaline {
namespace Core {
class CorpusCommunication;
}
}

struct PhonetisedSyllable;
struct YizhiScriptsData;

class YizhiScripts
{
public:
    YizhiScripts();
    ~YizhiScripts();

    QString readPhonetisationFile();
    QString createAnnotation(Praaline::Core::CorpusCommunication *com);

private:
    YizhiScriptsData *d;
};

#endif // YIZHISCRIPTS_H
