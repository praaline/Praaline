#ifndef SPEECHRATEEXPERIMENTS_H
#define SPEECHRATEEXPERIMENTS_H

#include <QString>

class SpeechRateExperiments
{
public:
    SpeechRateExperiments();

    bool readResultsFile(const QString &filename);
};

#endif // SPEECHRATEEXPERIMENTS_H
