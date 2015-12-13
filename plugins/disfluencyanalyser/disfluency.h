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
               int interruptionPoint = -1, int editTermStart = -1, int editTermEnd = -1, QObject *parent = 0);

    int indexInterruptionPoint() const { return m_interruptionPoint; }
    int indexEditTermStart() const { return m_editTermStart; }
    int indexEditTermEnd() const { return m_editTermEnd; }

    RealTime timeStart() const;
    RealTime timeEnd() const;
    RealTime timeInterruptionPoint() const;
    RealTime timeEditTermStart() const;
    RealTime timeEditTermEnd() const;

    QString globalTag() const { return m_tag; }
    void setGlobalTag(const QString &tag) { m_tag = tag; }

    DisfluencyStructureTypes structureType() const;
    QList<Interval *> reparandumIntervals() const;
    QList<Interval *> editTermIntervals() const;
    QList<Interval *> repairIntervals(int startFrom = 0) const;
    QString reparandumText() const;
    QString editTermText() const;
    QString repairText() const;

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
    // Complex disfluency data
    QList<QPair<int, int> > m_structure;

    bool isValid();
};

#endif // DISFLUENCY_H
