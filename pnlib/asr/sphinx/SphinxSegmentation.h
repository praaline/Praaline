#ifndef SPHINXSEGMENTATION_H
#define SPHINXSEGMENTATION_H

#include <QObject>
#include <QList>
#include <QHash>
#include "pncore/annotation/IntervalTier.h"

namespace Praaline {
namespace ASR {

class SphinxSegmentation : public QObject
{
    Q_OBJECT
public:
    explicit SphinxSegmentation(QObject *parent = nullptr);
    ~SphinxSegmentation();

    static bool readSegmentationHypothesisFile(const QString &filename, QHash<QString, QList<Core::Interval *> > &utterances);
    static QList<Praaline::Core::Interval *> readContinuousFile(const QString &filename);

signals:

public slots:

private:
    static bool compareIntervals(Praaline::Core::Interval *A, Praaline::Core::Interval *B);
};

} // namespace ASR
} // namespace Praaline

#endif // SPHINXSEGMENTATION_H
