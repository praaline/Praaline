#ifndef KALDIFORCEDALIGNER_H
#define KALDIFORCEDALIGNER_H

#include <QString>

class KaldiForcedAligner
{
public:
    KaldiForcedAligner();

    void mfcc_func(const QString &mfcc_directory, const QString &log_directory,
                   const QString &job_name, const QString &mfcc_config_path);

};

#endif // KALDIFORCEDALIGNER_H
