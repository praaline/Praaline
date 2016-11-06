#ifndef PLUGINPROMINENCEFEATURES_H
#define PLUGINPROMINENCEFEATURES_H

#include <QObject>
#include <QString>
#include <QList>
#include <QHash>
#include <QPair>
#include <QTextStream>
#include "pncore/base/RealValueList.h"
#include "pncore/annotation/Interval.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/annotation/AnnotationTierGroup.h"
using namespace Praaline::Core;

class PluginProminenceFeatures : public QObject
{
    Q_OBJECT
public:
    explicit PluginProminenceFeatures(QObject *parent = 0);

    IntervalTier *annotate(QString annotationID, const QString &filenameModel, bool withPOS, const QString &tierName,
                           IntervalTier *tier_syll, IntervalTier *tier_token, QString speakerID,
                           QTextStream &streamFeatures, QTextStream &streamFeaturesCRF);

    QString process(AnnotationTierGroup *txg, QString annotationID, QTextStream &out);

    static void prepareFeatures(QHash<QString, RealValueList> &features, IntervalTier *tier_syll, IntervalTier *tier_phones = 0);

    QString m_modelsPath;
signals:

public slots:

private:
    static QPair<int, int> windowNoPause(IntervalTier *tier_syll, int i, int windowLeft, int windowRight);
    static double mean(IntervalTier *tier_syll, QString attributeName, int i, int windowLeft, int windowRight, bool checkStylized);
    static double relative(IntervalTier *tier_syll, QString attributeName, int i, int windowLeft, int windowRight,
                           bool checkStylized, bool logarithmic);

    void outputRFACE(const QString &sampleID, IntervalTier *tier_syll, IntervalTier *tier_token,
                     QHash<QString, RealValueList> &features, QTextStream &out);
    void readRFACEprediction(QString filename, IntervalTier *tier_syll, QString attribute);
    void annotateRFACE(QString filenameModel, const QString &sampleID,
                       IntervalTier *tier_syll, IntervalTier *tier_token,
                       QHash<QString, RealValueList> &features, QString attributeOutput);
    void outputSVM(IntervalTier *tier_syll, IntervalTier *tier_token,
                   QHash<QString, RealValueList> &features, QTextStream &out);
    int outputCRF(IntervalTier *tier_syll, IntervalTier *tier_token,
                  QHash<QString, RealValueList> &features, bool withPOS, QTextStream &out,
                  bool createSequences = true);
    IntervalTier *annotateWithCRF(IntervalTier *tier_syll, IntervalTier *tier_token,
                                  QHash<QString, RealValueList> &features, bool withPOS,
                                  const QString &filenameModel, const QString &tier_name = "promise");

    QString m_currentAnnotationID;
};

#endif // PLUGINPROMINENCEFEATURES_H
