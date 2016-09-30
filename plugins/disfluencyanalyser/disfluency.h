#ifndef DISFLUENCY_H
#define DISFLUENCY_H

#include <QObject>
#include <QString>
#include "annotation/intervaltier.h"
#include "annotation/interval.h"
#include "tokensequence.h"

class Disfluency : public TokenSequence
{
    Q_OBJECT
public:
    enum DisfluencyStructureTypes {
        Simple,
        Structured,
        Complex,
        InvalidData
    };

    Disfluency(IntervalTier *tierTokens, int start, int end, const QString &globalTag,
               int interruptionPoint = -1, int reparansStart = -1,
               int editTermStart = -1, int editTermEnd = -1, QObject *parent = 0);

    int indexInterruptionPoint() const { return m_interruptionPoint; }
    int indexReparansStart() const { return m_reparansStart; }
    int indexEditTermStart() const { return m_editTermStart; }
    int indexEditTermEnd() const { return m_editTermEnd; }    

    RealTime timeStart() const;
    RealTime timeEnd() const;
    RealTime timeInterruptionPoint() const;
    RealTime timeReparansStart() const;
    RealTime timeEditTermStart() const;
    RealTime timeEditTermEnd() const;
    RealTime timeReparandumStart() const { return timeStart(); }
    RealTime timeReparandumEnd() const { return timeInterruptionPoint(); }
    RealTime timeInterregnumStart() const { return timeInterruptionPoint(); }
    RealTime timeInterregnumEnd() const { return timeReparansStart(); }
    RealTime timeReparansEnd() const { return timeEnd(); }

    QString globalTag() const { return m_tag; }
    void setGlobalTag(const QString &tag) { m_tag = tag; }

    DisfluencyStructureTypes structureType() const;
    QList<Interval *> reparandumIntervals() const;
    QList<Interval *> interregnumIntervals() const;
    QList<Interval *> editTermIntervals() const;
    QList<Interval *> reparansIntervals(int startFrom = 0) const;
    QString reparandumText() const;
    QString interregnumText() const;
    QString editTermText() const;
    QString reparansText() const;

    QString formatted(const QString &leftS = "(", const QString &rightS = ")",
                      const QString &intpt = "* ",
                      const QString &leftE = " <", const QString &rightE = "> ");

    QList<QList<Interval *> > backtracking() const;
    QString backtrackingFormatted() const;

signals:

public slots:

private:
    // Structured disfluency data
    int m_interruptionPoint;
    int m_editTermStart;
    int m_editTermEnd;
    int m_reparansStart;
    // Complex disfluency data
    QList<QPair<int, int> > m_structure;

    bool isValid();
};

#endif // DISFLUENCY_H
