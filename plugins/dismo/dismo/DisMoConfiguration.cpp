#include <QString>
#include <QDir>
#include "DisMoConfiguration.h"

DisMoConfiguration::DisMoConfiguration()
{

}

QString DisMoConfiguration::resourcesPath()
{
    QString pluginsPath = QDir::homePath() + "/Praaline/plugins";
    return pluginsPath + "/dismo";
}

QString DisMoConfiguration::filePathTokeniserKeepTogether(const QString &language)
{
    return resourcesPath() + "/lex/tokenizetogether_" + language + ".txt";
}

QString DisMoConfiguration::filePathDictionary(const QString &language)
{
    return resourcesPath() + "/lex/dismodic_" + language;
}

QString DisMoConfiguration::filePathMWUDictionary(const QString &language)
{
    return resourcesPath() + "/lex/mwudic_" + language + ".txt";
}

QString DisMoConfiguration::filePathCRFTest()
{
    QString toolsPath = QDir::homePath() + "/Praaline/tools";
    return toolsPath + "/crf/crf_test";
}
