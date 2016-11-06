#ifndef SPHINXRECOGNISER_H
#define SPHINXRECOGNISER_H

#include <QObject>
#include "pncore/corpus/Corpus.h"

using namespace Praaline::Core;

struct SphinxRecogniserData;

class SphinxRecogniser : public QObject
{
    Q_OBJECT
public:
    explicit SphinxRecogniser(QObject *parent = 0);
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
    bool recogniseUtterances_MFC(QPointer<CorpusCommunication> com, QString recordingID,
                                 QList<Interval *> &utterances, QList<Interval *> &segmentation);
    bool readRecognitionResults(const QString &fileID, const QString &filenameHypotheses, const QString filenameSegmentation,
                                QList<Interval *> &utterances, QList<Interval *> &segmentation);
signals:

public slots:

private:
    SphinxRecogniserData *d;
};

#endif // SPHINXRECOGNISER_H
