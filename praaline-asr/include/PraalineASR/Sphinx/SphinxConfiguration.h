#ifndef SPHINXCONFIGURATION_H
#define SPHINXCONFIGURATION_H

#include <QString>
#include "PraalineASR/PraalineASR_Global.h"

namespace Praaline {
namespace ASR {

class PRAALINE_ASR_SHARED_EXPORT SphinxConfiguration
{
public:
    SphinxConfiguration();
    SphinxConfiguration(const SphinxConfiguration &other);

    void setConfig(const SphinxConfiguration &config);

    QString name() const
    { return m_name; }
    void setName(const QString &name)
    { m_name = name; }

    QString languageID() const
    { return m_languageID; }
    void setLanguageID(const QString &languageID)
    { m_languageID = languageID; }

    QString directoryAcousticModel() const
    { return m_directoryAcousticModel; }
    void setDirectoryAcousticModel(const QString &directoryAcousticModel)
    { m_directoryAcousticModel = directoryAcousticModel; }

    QString filenameLanguageModel() const
    { return m_filenameLanguageModel; }
    void setFilenameLanguageModel(const QString &filenameLanguageModel)
    { m_filenameLanguageModel = filenameLanguageModel; }

    QString filenamePronunciationDictionary() const
    { return m_filenamePronunciationDictionary; }
    void setFilenamePronunciationDictionary(const QString &filenamePronunciationDictionary)
    { m_filenamePronunciationDictionary = filenamePronunciationDictionary; }

    QString filenameMLLRMatrix() const
    { return m_filenameMLLRMatrix; }
    void setFilenameMLLRMatrix(const QString &filenameMLLRMatrix)
    { m_filenameMLLRMatrix = filenameMLLRMatrix; }

    static QString sphinxPath();
    static QString defaultModelsPath();

private:
    QString m_name;
    QString m_languageID;
    QString m_directoryAcousticModel;
    QString m_filenameLanguageModel;
    QString m_filenamePronunciationDictionary;
    QString m_filenameMLLRMatrix;
};

} // namespace ASR
} // namespace Praaline

#endif // SPHINXCONFIGURATION_H
