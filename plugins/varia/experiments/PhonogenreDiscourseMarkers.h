#ifndef PHONOGENREDISCOURSEMARKERS_H
#define PHONOGENREDISCOURSEMARKERS_H

#include <QString>
#include <QPointer>

namespace Praaline {
namespace Core {
class CorpusCommunication;
}
}

class PhonogenreDiscourseMarkers
{
public:
    PhonogenreDiscourseMarkers();

    static QString readBackAnnotations(QPointer<Praaline::Core::CorpusCommunication> com);
    static QString statistics(QPointer<Praaline::Core::CorpusCommunication> com);
};

#endif // PHONOGENREDISCOURSEMARKERS_H
