#ifndef SPHINXRECOGNISER_H
#define SPHINXRECOGNISER_H

#include <QObject>
#include "PraalineCore/Corpus/CorpusCommunication.h"
#include "PraalineCore/Annotation/Interval.h"
#include "PraalineASR/PraalineASR_Global.h"

namespace Praaline {
namespace ASR {

struct SphinxRecogniserData;

class PRAALINE_ASR_SHARED_EXPORT SphinxRecogniser : public QObject
{
    Q_OBJECT
public:
    explicit SphinxRecogniser(QObject *parent = nullptr);
    ~SphinxRecogniser();

    // Configuration (global, user specified)
    void setAcousticModel(const QString &pathAcousticModel);
    void setLanguageModel(const QString &filenameLanguageModel);
    void setPronunciationDictionary(const QString &filenamePronunciationDictionary);
    void setMLLRMatrix(const QString &filenameMLLRMatrix);
    // Configuration (from Corpus Communication attributes)
    void setUseAcousticModelFromAttribute(bool use);
    void setUseLanguageModelFromAttribute(bool use);
    void setUseMLLRMatrixFromAttribute(bool use);
    void setAttributeNames(const QString &attributenameAcousticModel, const QString &attributenameLanguageModel,
                           const QString &attributenameMLLRMatrix);

    // Recipes
    bool recogniseUtterances_MFC(Praaline::Core::CorpusCommunication *com, QString recordingID,
                                 QList<Praaline::Core::Interval *> &utterances,
                                 QList<Praaline::Core::Interval *> &segmentation);
    bool readRecognitionResults(const QString &fileID, const QString &filenameHypotheses, const QString filenameSegmentation,
                                QList<Praaline::Core::Interval *> &utterances,
                                QList<Praaline::Core::Interval *> &segmentation);
signals:

public slots:

private:
    SphinxRecogniserData *d;
};

} // namespace ASR
} // namespace Praaline

#endif // SPHINXRECOGNISER_H
