#ifndef EXPERIMENTCL_IMPORTFILES_H
#define EXPERIMENTCL_IMPORTFILES_H

#include <QString>
#include "PraalineCore/Base/RealTime.h"

namespace Praaline {
namespace Core {
class CorpusCommunication;
class CorpusRepository;
}
}

struct ExperimentCL_ImportFilesData;

class ExperimentCL_ImportFiles
{
public:
    ExperimentCL_ImportFiles();
    ~ExperimentCL_ImportFiles();

    void setCorpusRepository(Praaline::Core::CorpusRepository *rep);

    QString importCSV_ETU_Stroop(const QString &subjectID, const QString &language, bool isDualTask, const QString &filename);
    QString importCSV_ETU_ReadingSpan(const QString &subjectID, const QString &filename);

private:
    ExperimentCL_ImportFilesData *d;

    QHash<QString, RealTime> getDTMFTimes(const QString &subjectID, const QString &blockGroupID);
    void exportStroopTranscriptionTextgrid(const QString &subjectID, const QString &blockGroupID);
};

#endif // EXPERIMENTCL_IMPORTFILES_H
