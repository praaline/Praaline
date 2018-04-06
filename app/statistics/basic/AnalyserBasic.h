#ifndef ANALYSERBASIC_H
#define ANALYSERBASIC_H

#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>
#include <QStandardItemModel>
#include "pncore/base/RealTime.h"

namespace Praaline {
namespace Core {
class Corpus;
class CorpusCommunication;
class Interval;
class IntervalTier;
class StatisticalMeasureDefinition;
}
}

struct AnalyserBasicData;

class AnalyserBasic : public QObject
{
    Q_OBJECT
public:
    explicit AnalyserBasic(QObject *parent = 0);
    virtual ~AnalyserBasic();

    static QList<QString> groupingLevels();
    static QList<QString> measureIDs(const QString &groupingLevel);
    static Praaline::Core::StatisticalMeasureDefinition measureDefinition(const QString &groupingLevel, const QString &measureID);

    double measure(const QString &groupingLevel, const QString &key, const QString &measureID) const;

    QPointer<QStandardItemModel> model();

    void setMetadataAttributesCommunication(const QStringList &attributeIDs);
    void setMetadataAttributesSpeaker(const QStringList &attributeIDs);
    void setMetadataAttributesRecording(const QStringList &attributeIDs);
    void setMetadataAttributesAnnotation(const QStringList &attributeIDs);

    QString calculate(QPointer<Praaline::Core::Corpus> corpus);

private:
    AnalyserBasicData *d;
};

#endif // ANALYSERBASIC_H
