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

    QString importCSV_ETU_FR01(const QString &subjectID, const QString &filename);
    QString importCSV_ETU_FR02(const QString &subjectID, const QString &filename);
    QString importCSV_ETU_FR03(const QString &subjectID, const QString &filename);
    QString importCSV_ETU_EN01(const QString &subjectID, const QString &filename);
    QString importCSV_ETU_EN02(const QString &subjectID, const QString &filename);
    QString importCSV_ETU_EN03(const QString &subjectID, const QString &filename);

private:
    ExperimentCL_ImportFilesData *d;

    QHash<QString, RealTime> getDTMFTimes(const QString &subjectID, const QString &blockGroupID);
    void exportTranscriptionTextgrid(const QString &subjectID, const QString &blockGroupID);
};

#endif // EXPERIMENTCL_IMPORTFILES_H
