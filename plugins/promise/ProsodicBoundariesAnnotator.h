#ifndef PROSODICBOUNDARIESANNOTATOR_H
#define PROSODICBOUNDARIESANNOTATOR_H

#include <QObject>
#include <QString>
#include <QList>

#include "pncore/base/RealValueList.h"
#include "pncore/annotation/Interval.h"
#include "pncore/annotation/IntervalTier.h"

class ProsodicBoundariesAnnotator : public QObject
{
    Q_OBJECT
public:
    explicit ProsodicBoundariesAnnotator(QObject *parent = 0);

    static void prepareFeatures(QHash<QString, RealValueList> &features, Praaline::Core::IntervalTier *tier_syll);

    Praaline::Core::IntervalTier *annotate(
            QString annotationID, const QString &filenameModel, bool withPOS, const QString &tierName,
            Praaline::Core::IntervalTier *tier_syll, Praaline::Core::IntervalTier *tier_token, QString speakerID,
            QTextStream &streamFeatures, QTextStream &streamFeaturesCRF);

private:
    int outputCRF(Praaline::Core::IntervalTier *tier_syll, Praaline::Core::IntervalTier *tier_token,
                  QHash<QString, RealValueList> &features, bool withPOS, QTextStream &out,
                  bool createSequences = true);
    Praaline::Core::IntervalTier *annotateWithCRF(
            Praaline::Core::IntervalTier *tier_syll, Praaline::Core::IntervalTier *tier_token,
            QHash<QString, RealValueList> &features, bool withPOS,
            const QString &filenameModel, const QString &tier_name = "boundary_auto");

    static int quantize(double x, int factor, int max);

    QString m_currentAnnotationID;
};

#endif // PROSODICBOUNDARIESANNOTATOR_H
