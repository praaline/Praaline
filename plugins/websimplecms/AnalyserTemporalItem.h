#ifndef ANALYSERTEMPORALITEM_H
#define ANALYSERTEMPORALITEM_H

#include <QObject>
#include <QPointer>
#include <QTextStream>
#include "PraalineCore/Statistics/StatisticalMeasureDefinition.h"

struct AnalyserTemporalItemData;

namespace Praaline {
namespace Core {
class CorpusCommunication;
class IntervalTier;
}
}

class AnalyserTemporalItem : public QObject
{
    Q_OBJECT
public:
    explicit AnalyserTemporalItem(QObject *parent = nullptr);
    virtual ~AnalyserTemporalItem();

    static QStringList measureIDsForCommunication();
    static QStringList measureIDsForSpeaker();
    static QStringList vectorMeasureIDsForCommunication();
    static QStringList vectorMeasureIDsForSpeaker();
    static Praaline::Core::StatisticalMeasureDefinition measureDefinition(const QString &measureID);

    QString levelIDSyllables() const;
    void setLevelIDSyllables(const QString &levelID);
    QString levelIDTokens() const;
    void setLevelIDTokens(const QString &levelID);
    QStringList filledPauseTokens() const;
    void setFilledPauseTokens(const QStringList &filledPauseTokens);

    QStringList speakerIDs() const;
    double measureCom(const QString &measureID) const;
    double measureSpk(const QString &speakerID, const QString &measureID) const;
    QList<double> vectorMeasureCom(const QString &measureID) const;
    QList<double> vectorMeasureSpk(const QString &speakerID, const QString &measureID) const;

    QPointer<Praaline::Core::IntervalTier> timelineSyll() const;
    QPointer<Praaline::Core::IntervalTier> timelineSpeaker() const;

    void analyse(Praaline::Core::CorpusCommunication *com);

signals:

public slots:

private:
    AnalyserTemporalItemData *d;
};

#endif // ANALYSERTEMPORALITEM_H
