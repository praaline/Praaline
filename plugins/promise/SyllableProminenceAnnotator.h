#ifndef SYLLABLEPROMINENCEANNOTATOR_H
#define SYLLABLEPROMINENCEANNOTATOR_H

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

class SyllableProminenceAnnotator : public QObject
{
    Q_OBJECT
public:
    explicit SyllableProminenceAnnotator(QObject *parent = 0);

    static void prepareFeatures(QHash<QString, RealValueList> &features,
                                Praaline::Core::IntervalTier *tier_syll, Praaline::Core::IntervalTier *tier_phones = 0);

    Praaline::Core::IntervalTier *annotate(
            QString annotationID, const QString &filenameModel, bool withPOS, const QString &tierName,
            Praaline::Core::IntervalTier *tier_syll, Praaline::Core::IntervalTier *tier_token, QString speakerID,
            QTextStream &streamFeatures, QTextStream &streamFeaturesCRF);

    QString process(Praaline::Core::AnnotationTierGroup *txg, QString annotationID, QTextStream &out);

    QString modelsPath() const { return m_modelsPath; }
    void setModelsPath(const QString &modelsPath) { m_modelsPath = modelsPath; }

signals:

public slots:

private:
    void outputRFACE(const QString &sampleID,
                     Praaline::Core::IntervalTier *tier_syll, Praaline::Core::IntervalTier *tier_token,
                     QHash<QString, RealValueList> &features, QTextStream &out);
    void readRFACEprediction(QString filename, Praaline::Core::IntervalTier *tier_syll, QString attribute);
    void annotateRFACE(QString filenameModel, const QString &sampleID,
                       Praaline::Core::IntervalTier *tier_syll, Praaline::Core::IntervalTier *tier_token,
                       QHash<QString, RealValueList> &features, QString attributeOutput);
    void outputSVM(Praaline::Core::IntervalTier *tier_syll, Praaline::Core::IntervalTier *tier_token,
                   QHash<QString, RealValueList> &features, QTextStream &out);
    int outputCRF(Praaline::Core::IntervalTier *tier_syll, Praaline::Core::IntervalTier *tier_token,
                  QHash<QString, RealValueList> &features, bool withPOS, QTextStream &out,
                  bool createSequences = true);
    Praaline::Core::IntervalTier *annotateWithCRF(
            Praaline::Core::IntervalTier *tier_syll, Praaline::Core::IntervalTier *tier_token,
            QHash<QString, RealValueList> &features, bool withPOS,
            const QString &filenameModel, const QString &tier_name = "promise");

    QString m_modelsPath;
    QString m_currentAnnotationID;
};

#endif // SYLLABLEPROMINENCEANNOTATOR_H
