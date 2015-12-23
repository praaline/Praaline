#ifndef SPHINXRECOGNISER_H
#define SPHINXRECOGNISER_H

#include <QObject>
#include "pncore/corpus/corpus.h"

struct SphinxRecogniserData;

class SphinxRecogniser : public QObject
{
    Q_OBJECT
public:
    explicit SphinxRecogniser(QObject *parent = 0);
    ~SphinxRecogniser();

    void setUseMLLR(bool use);
    void setUseSpecialisedLM(bool use);
    void setAttributeNames(const QString &attributenameAcousticModel, const QString &attributenameLanguageModel);
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
