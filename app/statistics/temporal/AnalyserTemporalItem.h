#ifndef ANALYSERTEMPORALITEM_H
#define ANALYSERTEMPORALITEM_H

#include <QObject>
#include <QPointer>
#include <QTextStream>
#include "pncore/statistics/StatisticalMeasureDefinition.h"

struct AnalyserTemporalItemData;

namespace Praaline {
namespace Core {
class CorpusCommunication;
}
}

class AnalyserTemporalItem : public QObject
{
    Q_OBJECT
public:
    explicit AnalyserTemporalItem(QObject *parent = 0);
    virtual ~AnalyserTemporalItem();

    static QStringList measureIDsForCommunication();
    static QStringList measureIDsForSpeaker();
    static QStringList vectorMeasureIDsForCommunication();
    static QStringList vectorMeasureIDsForSpeaker();
    static Praaline::Core::StatisticalMeasureDefinition measureDefinition(const QString &measureID);

    QStringList speakerIDs() const;
    double measureCom(const QString &measureID) const;
    double measureSpk(const QString &speakerID, const QString &measureID) const;
    QList<double> vectorMeasureCom(const QString &measureID) const;
    QList<double> vectorMeasureSpk(const QString &speakerID, const QString &measureID) const;

    void analyse(QPointer<Praaline::Core::CorpusCommunication> com);

signals:

public slots:

private:
    AnalyserTemporalItemData *d;
};

#endif // ANALYSERTEMPORALITEM_H
