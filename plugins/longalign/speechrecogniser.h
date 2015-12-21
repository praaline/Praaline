#ifndef SPEECHRECOGNISER_H
#define SPEECHRECOGNISER_H

#include <QString>
#include "pncore/RealTime.h"
#include "pncore/corpus/corpusrecording.h"

class SpeechRecogniser
{
public:
    SpeechRecogniser();
    ~SpeechRecogniser();

    void setFilenameAcousticModel(const QString &filename);
    void setFilenameLanguageModel(const QString &filename);
    void setFilenamePronuncationDictionary(const QString &filename);

    int recogniseContinuous(CorpusRecording *recording);

private:
    QString m_filenameAcousticModel;
    QString m_filenameLanguageModel;
    QString m_filenamePronunciationDictionary;

};

#endif // SPEECHRECOGNISER_H
