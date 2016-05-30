#ifndef ANALYSERPITCH_H
#define ANALYSERPITCH_H

#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>
#include <QStandardItemModel>
#include "pncore/statistics/StatisticalMeasureDefinition.h"

struct AnalyserPitchData;
class Corpus;
class CorpusCommunication;
class Interval;

class AnalyserPitch : public QObject
{
    Q_OBJECT
public:
    explicit AnalyserPitch(QObject *parent = 0);
    virtual ~AnalyserPitch();

    static QList<QString> groupingLevels();
    static QList<QString> measureIDs(const QString &groupingLevel);
    static StatisticalMeasureDefinition measureDefinition(const QString &groupingLevel, const QString &measureID);

    double measure(const QString &groupingLevel, const QString &key, const QString &measureID) const;

    QPointer<QStandardItemModel> model();

    QString calculate(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com);
    QString calculate(QPointer<Corpus> corpus, const QString &communicationID, const QString &annotationID,
                      const QList<Interval *> &units = QList<Interval *>());


signals:

public slots:

private:
    AnalyserPitchData *d;
};

#endif // ANALYSERPITCH_H
