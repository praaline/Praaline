#ifndef PHONETISEREXTERNAL_H
#define PHONETISEREXTERNAL_H

#include <QString>
#include <QPointer>

namespace Praaline {
namespace Core {
class CorpusCommunication;
}
}

struct PhonetiserExternalData;

class PhonetiserExternal
{
public:
    PhonetiserExternal();
    ~PhonetiserExternal();

    QString outputFilesPath() const;
    void setOutputFilesPath(const QString &path);

    QString attributePhonetisationOfTokens() const;
    void setAttributePhonetisationOfTokens(const QString &attributeID);

    QString exportToPhonetiser(QPointer<Praaline::Core::CorpusCommunication> com);
    QString importFromPhonetiser(QPointer<Praaline::Core::CorpusCommunication> com, bool fromTranscriptionTier = false);

private:
    PhonetiserExternalData *d;
};

#endif // PHONETISEREXTERNAL_H
