#include "disfluencyanalysertool.h"

QStringList DisfluencyAnalyserTool::m_simpleDisfluencyTags = QStringList()
        << "FIL" << "LEN" << "FST" << "WDP";
QStringList DisfluencyAnalyserTool::m_structuredDisfluecnyTags = QStringList()
        << "REP" << "DEL" << "SUB" << "INS" << "COM";

DisfluencyAnalyserTool::DisfluencyAnalyserTool(IntervalTier *tierTokens, QObject *parent) :
    QObject(parent), m_tierTokens(tierTokens)
{
}

Disfluency* DisfluencyAnalyserTool::disfluency(int index) const
{
    return m_disfluencies.value(index);
}

QList<Disfluency *> DisfluencyAnalyserTool::disfluencies() const
{
    return m_disfluencies;
}

bool DisfluencyAnalyserTool::readFromTier(IntervalTier *tierDisfluency, const QString &attribute)
{
    if (!tierDisfluency)
        return false;
    if (!(tierDisfluency->countItems() == m_tierTokens->countItems()))
        return false;

    int start = -1, end = -1, interruptionPoint = -1, editStart = -1, editEnd = -1;
    QString tag, previousTag, tagExt;
    bool inEdit = false;
    for (int i = 0; i < m_tierTokens->countItems(); i++) {
        QString disfluencyIntv;
        if (attribute.isEmpty())
            disfluencyIntv = tierDisfluency->interval(i)->text();
        else
            disfluencyIntv = tierDisfluency->interval(i)->attribute(attribute).toString();
        tag = disfluencyIntv.left(3);
        tagExt = disfluencyIntv.mid(3, -1);
        if (tag != previousTag) {
            if (start >= 0) {
                end = i - 1;
                m_disfluencies.append(new Disfluency(m_tierTokens, start, end, previousTag, interruptionPoint, editStart, editEnd, this));
                start = end = interruptionPoint = editStart = editEnd = -1;
            }
            if ((!tag.isEmpty()) && (!(tag == "SIL"))) {
                start = i;
                inEdit = false;
            }
        }
        if (tag.isEmpty() || tag == "SIL") {
            previousTag.clear();
            continue;
        }
        // Found the interruption point
        if (tagExt.startsWith("*")) {
            interruptionPoint = i;
        }
        // Entering an explicit editing term
        if (!inEdit && tagExt.startsWith(":edt")) {
            editStart = i;
            inEdit = true;
        }
        // No longer inside an explicit editing term
        if (inEdit & !tagExt.startsWith(":edt")) {
            editEnd = i - 1; // the editing term finishes on the previous token
            inEdit = false;
        }
        previousTag = tag;
    }
    return true;
}

void disfluencyToBacktrack(Disfluency *disf)
{
    QList<QPair<int, QString> > line1;
    QList<QPair<int, QString> > line2;
}



