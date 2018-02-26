#ifndef LANGUAGEDEFINITION_H
#define LANGUAGEDEFINITION_H

#include <QString>
#include <QPointer>

namespace Praaline {
namespace Core {
class CorpusCommunication;
}
}

class LanguageDefinition
{
public:
    LanguageDefinition();

    static QString checkCharacters(QPointer<Praaline::Core::CorpusCommunication> com, const QString &levelID);

};

#endif // LANGUAGEDEFINITION_H
