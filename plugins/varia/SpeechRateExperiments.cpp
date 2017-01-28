#include <QString>
#include <QFile>
#include <QTextStream>
#include "SpeechRateExperiments.h"

SpeechRateExperiments::SpeechRateExperiments()
{
}

bool SpeechRateExperiments::readResultsFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open( QIODevice::ReadOnly | QIODevice::Text )) return false;
    QTextStream stream(&file);

    do {
        QString line = stream.readLine().trimmed();


    } while (!stream.atEnd());
    file.close();
    return true;
}
