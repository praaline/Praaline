#ifndef ANALYSERPITCH_H
#define ANALYSERPITCH_H

#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>
#include <QStandardItemModel>

namespace Praaline {
namespace Core {
class Corpus;
class CorpusCommunication;
class Interval;
class IntervalTier;
class StatisticalMeasureDefinition;
}
}

struct AnalyserPitchData;

class AnalyserPitch : public QObject
{
    Q_OBJECT
public:
    explicit AnalyserPitch(QObject *parent = 0);
    virtual ~AnalyserPitch();

    static QList<QString> groupingLevels();
    static QList<QString> measureIDs(const QString &groupingLevel);
    static Praaline::Core::StatisticalMeasureDefinition measureDefinition(const QString &groupingLevel, const QString &measureID);

    double measure(const QString &groupingLevel, const QString &key, const QString &measureID) const;

    QPointer<QStandardItemModel> model();

    QString calculate(QPointer<Praaline::Core::Corpus> corpus, QPointer<Praaline::Core::CorpusCommunication> com);
    QString calculate(QPointer<Praaline::Core::Corpus> corpus, const QString &communicationID, const QString &annotationID,
                      const QList<Praaline::Core::Interval *> &units = QList<Praaline::Core::Interval *>());


signals:

public slots:

private:
    AnalyserPitchData *d;
};

#endif // ANALYSERPITCH_H
