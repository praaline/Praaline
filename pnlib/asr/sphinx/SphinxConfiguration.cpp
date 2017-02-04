#include <QDir>
#include <QCoreApplication>
#include "SphinxConfiguration.h"

namespace Praaline {
namespace ASR {

SphinxConfiguration::SphinxConfiguration()
{
}

SphinxConfiguration::SphinxConfiguration(const SphinxConfiguration &other) :
    m_name(other.name()), m_languageID(other.languageID()),
    m_directoryAcousticModel(other.directoryAcousticModel()),
    m_filenameLanguageModel(other.filenameLanguageModel()),
    m_filenamePronunciationDictionary(other.filenamePronunciationDictionary()),
    m_filenameMLLRMatrix(other.filenameMLLRMatrix())
{
}

void SphinxConfiguration::setConfig(const SphinxConfiguration &config)
{
    m_name = config.name();
    m_languageID = config.languageID();
    m_directoryAcousticModel = config.directoryAcousticModel();
    m_filenameLanguageModel = config.filenameLanguageModel();
    m_filenamePronunciationDictionary = config.filenamePronunciationDictionary();
    m_filenameMLLRMatrix = config.filenameMLLRMatrix();
}

// static
QString SphinxConfiguration::sphinxPath()
{
#ifdef Q_OS_WIN
    QString appPath = QCoreApplication::applicationDirPath();
    return appPath + "/plugins/aligner/sphinx/";
#endif
#ifdef Q_OS_MAC
    return "/usr/local/bin/";
#else
    return "/usr/local/bin/";
#endif
}

// static
QString SphinxConfiguration::defaultModelsPath()
{
    QString appPath = QCoreApplication::applicationDirPath();
#ifdef Q_OS_MAC
    QDir dir(appPath);
    dir.cdUp(); dir.cdUp(); dir.cdUp();
    appPath = dir.absolutePath() + "/";
#endif
    return appPath + "/plugins/aligner/sphinx/";
}

} // namespace ASR
} // namespace Praaline
