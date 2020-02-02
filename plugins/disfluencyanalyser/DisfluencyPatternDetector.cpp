#include <QString>
#include <QPair>
#include <QList>
#include <QSet>
#include <QPointer>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QApplication>
#include <QDir>
#include "PraalineCore/Corpus/CorpusBookmark.h"

#include "DisfluencyPatternDetector.h"

struct DisfluencyPatternDetectorData {
    DisfluencyPatternDetectorData() :
        tierinfoToken(QPair<QString, QString>("tok_min", "")),
        tierinfoPOS(QPair<QString, QString>("tok_min", "pos_min")),
        tierinfoDisfluency(QPair<QString, QString>("tok_min", "disfluency")),
        tierToken(0), tierPOS(0), tierDisfluency(0)
    {}

    QPair<QString, QString> tierinfoToken;
    QPair<QString, QString> tierinfoPOS;
    QPair<QString, QString> tierinfoDisfluency;
    IntervalTier *tierToken;
    IntervalTier *tierPOS;
    IntervalTier *tierDisfluency;
    QStringList skipDiscourseMarkers;
};

DisfluencyPatternDetector::DisfluencyPatternDetector() :
    d(new DisfluencyPatternDetectorData)
{
    // Read file of discourse markers to skip in the pattern detection
    QString filenameSkip = QDir::homePath() + "/Praaline/plugins/dismo/dis/discoursemarkerstoskip.txt";
    QFile file(filenameSkip);
    if (!file.open( QIODevice::ReadOnly | QIODevice::Text )) {
        qDebug() << "Error: could not read file " << filenameSkip;
        return;
    }
    QTextStream stream(&file);
    do {
        QString line = stream.readLine().trimmed();
        if (line.startsWith("#")) continue;
        d->skipDiscourseMarkers.append(line);
    } while (!stream.atEnd());
    file.close();
}

DisfluencyPatternDetector::~DisfluencyPatternDetector()
{
    delete d;
}

void DisfluencyPatternDetector::setTierInfoToken(const QString &levelID, const QString &attributeID)
{
    d->tierinfoToken.first = levelID; d->tierinfoToken.second = attributeID;
}

void DisfluencyPatternDetector::setTierInfoPOS(const QString &levelID, const QString &attributeID)
{
    d->tierinfoPOS.first = levelID; d->tierinfoPOS.second = attributeID;
}

void DisfluencyPatternDetector::setTierInfoDisfluecny(const QString &levelID, const QString &attributeID)
{
    d->tierinfoDisfluency.first = levelID; d->tierinfoDisfluency.second = attributeID;
}

void DisfluencyPatternDetector::setTiers(AnnotationTierGroup *tiergroup)
{
    d->tierToken = tiergroup->getIntervalTierByName(d->tierinfoToken.first);
    d->tierPOS = tiergroup->getIntervalTierByName(d->tierinfoPOS.first);
    d->tierDisfluency = tiergroup->getIntervalTierByName(d->tierinfoDisfluency.first);
}

void DisfluencyPatternDetector::setTiers(IntervalTier *tierToken, IntervalTier *tierPOS, IntervalTier *tierDisfluecny)
{
    d->tierToken = tierToken; d->tierPOS = tierPOS; d->tierDisfluency = tierDisfluecny;
}

inline QString DisfluencyPatternDetector::token(QList<int> &indices, int i) const
{
    if (i < 0 || i >= indices.count()) return QString("###");
    if (d->tierinfoToken.second.isEmpty()) return d->tierToken->interval(indices.at(i))->text();
    else return d->tierToken->interval(indices.at(i))->attribute(d->tierinfoToken.second).toString();
}

inline QString DisfluencyPatternDetector::pos(QList<int> &indices, int i) const
{
    if (i < 0 || i >= indices.count()) return QString("###");
    if (d->tierinfoPOS.second.isEmpty()) return d->tierPOS->interval(indices.at(i))->text();
    else return d->tierPOS->interval(indices.at(i))->attribute(d->tierinfoPOS.second).toString();
}

inline QString DisfluencyPatternDetector::disfluency(QList<int> &indices, int i) const
{
    if (i < 0 || i >= indices.count()) return QString("###");
    if (d->tierinfoDisfluency.second.isEmpty()) return d->tierDisfluency->interval(indices.at(i))->text();
    else return d->tierDisfluency->interval(indices.at(i))->attribute(d->tierinfoDisfluency.second).toString();
}

QList<int> DisfluencyPatternDetector::indicesWithoutSimpleDisfluencies(bool withDMlist, int from, int to)
{
    // Token skipping happens here
    QList<int> list;
    if (to < 0) to = d->tierDisfluency->count() - 1;
    for (int i = from; i <= to; ++i) {
        QString tok = (d->tierinfoToken.second.isEmpty()) ?
                    d->tierToken->interval(i)->text() :
                    d->tierToken->interval(i)->attribute(d->tierinfoToken.second).toString();
        QString pos = (d->tierinfoPOS.second.isEmpty()) ?
                    d->tierPOS->interval(i)->text() :
                    d->tierPOS->interval(i)->attribute(d->tierinfoPOS.second).toString();
        QString dis = (d->tierinfoDisfluency.second.isEmpty()) ?
                    d->tierDisfluency->interval(i)->text() :
                    d->tierDisfluency->interval(i)->attribute(d->tierinfoDisfluency.second).toString();
        // Skip (already annotated) simple disfluencies
        if (dis.contains("SIL") || dis.contains("FIL") || dis.contains("FST") || dis.contains("PARA")) continue;
        if (withDMlist && d->skipDiscourseMarkers.contains(tok)) {
            d->tierDisfluency->interval(i)->setAttribute("__DMskip", true);
            continue;
        }
        if (pos.startsWith("ITJ")) continue;
        if (withDMlist) qDebug() << tok << pos << dis;
        // otherwise, do not skip
        list << i;
    }
    return list;
}

bool DisfluencyPatternDetector::matchAll(QList<int> &indices, int start, int window)
{
    bool match = true;
    for (int i = 0; i < window; ++i) {
        QString tok1 = token(indices, start + i);
        QString tok2 = token(indices, start + i + window);
        qDebug() << tok1 << tok2;
        if (tok1 != tok2) {
            match = false;
            break;
        }
    }
    return match;
}

void DisfluencyPatternDetector::revertToDisfluenciesLevel1()
{
    for (int index = 0; index < d->tierDisfluency->count(); ++index) {
        QString dis = (d->tierinfoDisfluency.second.isEmpty()) ?
                    d->tierDisfluency->interval(index)->text() :
                    d->tierDisfluency->interval(index)->attribute(d->tierinfoDisfluency.second).toString();
        QString updated;
        if (dis.contains("SIL")) updated = "SIL";
        if (dis.contains("FIL")) updated = "FIL";
        if (dis.contains("FST")) updated = "FST";
        if (dis.contains("LEN")) updated = "LEN";
        if (dis.contains("WDP")) updated = "WDP";
        if (dis.contains("PARA")) updated = "PARA";
        if (d->tierinfoDisfluency.second.isEmpty())
            d->tierDisfluency->interval(index)->setText(updated);
        else
            d->tierDisfluency->interval(index)->setAttribute(d->tierinfoDisfluency.second, updated);
    }
}

QList<DisfluencyPatternDetector::RepetitionInfo> DisfluencyPatternDetector::detectRepetitionPatterns()
{
    QList<RepetitionInfo> hitsAll;
    if (!d->tierDisfluency) return hitsAll;
    if (!d->tierToken) return hitsAll;

    QSet<int> indicesAlreadyAnnotated;

    QList<bool> withDMseq; withDMseq << false << true;
    foreach (bool withDM, withDMseq) {
        qDebug() << withDM;
        QList<RepetitionInfo> hits;
        QList<int> indices = indicesWithoutSimpleDisfluencies(withDM);
        for (int window = 8; window >= 1; --window) {
            for (int i = 0; i < indices.count(); ++i) {
                if (indicesAlreadyAnnotated.contains(indices.at(i))) continue;
                if (matchAll(indices, i, window)) {
                    // how many times was that repeated ?
                    int times = 2;
                    while (matchAll(indices, i + (times - 1) * window, window)) {
                        times++;
                    }
                    // simplify identical strings (a a a a)
                    bool identical = true;
                    for (int j = i; j < i + (times * window) - 1; ++j) {
                        if (token(indices, j) != token(indices, j + 1)) {
                            identical = false;
                            break;
                        }
                    }
                    RepetitionInfo rep;
                    for (int j = i; j < i + (times * window); ++j) {
                        rep.indices << indices.at(j);
                        indicesAlreadyAnnotated.insert(indices.at(j));
                    }
                    if (identical) {
                        rep.window = 1;
                        rep.times = times * window;
                    } else {
                        rep.window = window;
                        rep.times = times;
                    }
                    QString text;
                    for (int j = i; j < i + (times * window); ++j) {
                        text.append(token(indices, j)).append(" ");
                    }
                    rep.text = text;
                    hits << rep;

                    qDebug() << rep.window << "\t" << rep.times << "\t" << rep.text;
                }
            }
        }
        codeRepetitions(hits, withDM);
        hitsAll << hits;
    }
    return hitsAll;

}

void DisfluencyPatternDetector::codeRepetitions(QList<DisfluencyPatternDetector::RepetitionInfo> &repetitions, bool withDM)
{
    foreach (RepetitionInfo rep, repetitions) {
        int counterToken = 1; int counterTimes = 1;
        for (int index = rep.indices.first(); index <= rep.indices.last(); ++index) {
            QString tok = (d->tierinfoToken.second.isEmpty()) ?
                        d->tierToken->interval(index)->text() :
                        d->tierToken->interval(index)->attribute(d->tierinfoToken.second).toString();
            QString pos = (d->tierinfoPOS.second.isEmpty()) ?
                        d->tierPOS->interval(index)->text() :
                        d->tierPOS->interval(index)->attribute(d->tierinfoPOS.second).toString();
            QString dis = (d->tierinfoDisfluency.second.isEmpty()) ?
                        d->tierDisfluency->interval(index)->text() :
                        d->tierDisfluency->interval(index)->attribute(d->tierinfoDisfluency.second).toString();
            qDebug() << tok << pos << dis;
            if      (dis.contains("SIL"))   dis = "REP+SIL";
            else if (dis.contains("FIL"))   dis = "REP+FIL";
            else if (dis.contains("FST"))   dis = "REP+FST";
            else if (dis.contains("PARA"))  dis = "REP+PARA";
            else if (withDM && (d->skipDiscourseMarkers.contains(tok))) {
                dis = "REP+DM";
            }
            else if (pos.startsWith("ITJ")) {
                dis = "REP+ITJ";
            }
            else {
                if (counterTimes == rep.times) dis = "REP_";
                else {
                    if (rep.window > 1)
                        dis = QString("REP:%1").arg(counterToken);
                    else
                        dis = "REP";
                    if (counterToken == rep.window && counterTimes == rep.times - 1)
                        dis.append("*");
                }
                counterToken++;
                if (counterToken > rep.window) { counterToken = 1; counterTimes++; }
            }
            if (d->tierinfoDisfluency.second.isEmpty())
                d->tierDisfluency->interval(index)->setText(dis);
            else
                d->tierDisfluency->interval(index)->setAttribute(d->tierinfoDisfluency.second, dis);
        }
    }
}

bool DisfluencyPatternDetector::match(QList<int> &indices, int startA, int startB, int length)
{
    bool match = true;
    for (int i = 0; i < length; ++i) {
        if (token(indices, startA + i) != token(indices, startB + i)) {
            match = false;
            break;
        }
    }
    return match;
}

QList<DisfluencyPatternDetector::InsertionInfo> DisfluencyPatternDetector::detectInsertionPatterns()
{
    QList<InsertionInfo> hits;
    if (!d->tierDisfluency) return hits;
    if (!d->tierToken) return hits;

    QList<int> indices = indicesWithoutSimpleDisfluencies();
    QSet<int> indicesAlreadyAnnotated;

    // Insertion pattern
    // x1..b   y1..a   x1..b   ins1...n   y1.....a, b=before=3, a=after=3, n=insertion=3
    // 1 2 3   4 5 6   7 8 9   10 11 12   13 14 15
    // i       i+b     i+b+a   i+2b+a     i+2b+a+n  i+2b+n+2a
    // p0      p1      p2      p3         p4        p5
    //
    // i = position
    for (int i = 0; i < indices.count(); ++i) {
        for (int lengthBefore = 4; lengthBefore >= 1; --lengthBefore) {
            for (int lengthAfter = 4; lengthAfter >=1; --lengthAfter) {
                for (int lengthInsertion = 4; lengthInsertion >= 1; --lengthInsertion) {
                    // Already annotated?
                    if (indicesAlreadyAnnotated.contains(indices.at(i))) continue;
                    // Calculate points
                    int p0 = i;
                    int p1 = p0 + lengthBefore;
                    int p2 = p1 + lengthAfter;
                    int p3 = p2 + lengthBefore;
                    int p4 = p3 + lengthInsertion;
                    int p5 = p4 + lengthAfter;
                    // Try to fit pattern
                    if (match(indices, p0, p2, lengthBefore) &&
                        match(indices, p1, p4, lengthAfter))
                    {
                        // Extra check: this is necessary to avoid long repetitions (e.g. a b c a b c a b c)
                        // that otherwise fit the description of an insertion
                        bool notLongRep = true;
                        for (int window = 2; window <= 4; ++window) {
                            if (matchAll(indices, i, window)) notLongRep = false;
                        }
                        if (!notLongRep) continue;

                        // OK, we have an insertion
                        InsertionInfo ins;
                        ins.lengthBefore = lengthBefore;
                        ins.lengthInsertion = lengthInsertion;
                        ins.lengthAfter = lengthAfter;
                        QString text;
                        for (int iter = p0; iter < p5; ++iter) {
                            ins.indices << indices.at(iter);
                            indicesAlreadyAnnotated.insert(indices.at(iter));
                            text.append(token(indices, iter)).append(" ");
                        }
                        ins.text = text;
                        hits << ins;
                        qDebug() << ins.lengthBefore << "\t" << ins.lengthInsertion << "\t" << ins.lengthAfter << "\t" << ins.text;
                    }
                }
            }
        }
    }
    return hits;
}

void DisfluencyPatternDetector::codeInsertions(QList<DisfluencyPatternDetector::InsertionInfo> &insertions)
{

}

QList<DisfluencyPatternDetector::SubstitutionInfo> DisfluencyPatternDetector::detectSubstitutionPatterns()
{
    QList<SubstitutionInfo> hits;
    if (!d->tierDisfluency) return hits;
    if (!d->tierToken) return hits;

    QList<int> indices = indicesWithoutSimpleDisfluencies();
    QSet<int> indicesAlreadyAnnotated;

    // Substitution pattern
    // x1..b   sx1.m   y1..a   x1.....b   sy1....n   y1.....a, b=before=3, a=after=3, m=substituted, n=substitution
    // 1 2 3   4 5 6   7 8 9   10 11 12   13 14 15   16 17 18
    // i       i+b     i+b+m   i+b+m+a    i+2b+m+a   i+2b+m+n+a  i+2b+m+n+2a
    // p0      p1      p2      p3         p4         p5          p6
    // where sa1...m != sb1...n
    for (int i = 0; i < indices.count(); ++i) {
        for (int lengthBefore = 3; lengthBefore >= 1; --lengthBefore) {
            for (int lengthAfter = 3; lengthAfter >=1; --lengthAfter) {
                for (int lengthOldTokens = 3; lengthOldTokens >= 1; --lengthOldTokens) {
                    for (int lengthNewTokens = 3; lengthNewTokens >= 1; --lengthNewTokens) {
                        // Already annotated?
                        if (indicesAlreadyAnnotated.contains(indices.at(i))) continue;
                        // Calculate points
                        int p0 = i;
                        int p1 = p0 + lengthBefore;
                        int p2 = p1 + lengthOldTokens;
                        int p3 = p2 + lengthAfter;
                        int p4 = p3 + lengthBefore;
                        int p5 = p4 + lengthNewTokens;
                        int p6 = p5 + lengthAfter;
                        // Try to fit pattern
                        if (match(indices, p0, p3, lengthBefore) &&
                            match(indices, p2, p5, lengthAfter) &&
                            (!match(indices, p1, p4, qMax(lengthOldTokens, lengthNewTokens))))
                        {
                            // OK, we have a substitution
                            SubstitutionInfo sub;
                            sub.lengthBefore = lengthBefore;
                            sub.lengthOldTokens = lengthOldTokens;
                            sub.lengthNewTokens = lengthNewTokens;
                            sub.lengthAfter = lengthAfter;
                            QString text;
                            for (int iter = p0; iter < p6; ++iter) {
                                sub.indices << indices.at(iter);
                                indicesAlreadyAnnotated.insert(indices.at(iter));
                                text.append(token(indices, iter)).append(" ");
                            }
                            sub.text = text;
                            hits << sub;
                            qDebug() << sub.lengthBefore << "\t" << sub.lengthOldTokens << "\t" << sub.lengthNewTokens
                                     << "\t" << sub.lengthAfter << "\t" << sub.text;
                        }
                    }
                }
            }
        }
    }
    return hits;
}

void DisfluencyPatternDetector::codeSubstitutions(QList<SubstitutionInfo> &substitutions)
{

}


CorpusBookmark *DisfluencyPatternDetector::createBookmark(const QString &corpusID, const QString &communicationID, const QString &annotationID,
                                                                   const DisfluencyPatternDetector::PatternInfoBase &pattern)
{
    if (!d->tierToken) return 0;
    if (pattern.indices.isEmpty()) return 0;
    int i = pattern.indices.first();
    if (i < 0 || i >= d->tierToken->count()) return 0;
    RealTime t = d->tierToken->interval(i)->tMin();
    return new CorpusBookmark(corpusID, communicationID, annotationID, t, pattern.type(), pattern.text);
}

QList<CorpusBookmark *>
DisfluencyPatternDetector::createBookmarks(const QString &corpusID, const QString &communicationID, const QString &annotationID,
                                           QList<DisfluencyPatternDetector::RepetitionInfo> &repetitions)
{
    QList<CorpusBookmark *> bookmarks;
    if (!d->tierToken) return bookmarks;
    foreach (RepetitionInfo rep, repetitions) {
        bookmarks << createBookmark(corpusID, communicationID, annotationID, rep);
    }
    return bookmarks;
}

QList<CorpusBookmark *>
DisfluencyPatternDetector::createBookmarks(const QString &corpusID, const QString &communicationID, const QString &annotationID,
                                           QList<DisfluencyPatternDetector::InsertionInfo> &insertions)
{
    QList<CorpusBookmark *> bookmarks;
    if (!d->tierToken) return bookmarks;
    foreach (InsertionInfo ins, insertions) {
        bookmarks << createBookmark(corpusID, communicationID, annotationID, ins);
    }
    return bookmarks;
}

QList<CorpusBookmark *>
DisfluencyPatternDetector::createBookmarks(const QString &corpusID, const QString &communicationID, const QString &annotationID,
                                           QList<DisfluencyPatternDetector::SubstitutionInfo> &substitutions)
{
    QList<CorpusBookmark *> bookmarks;
    if (!d->tierToken) return bookmarks;
    foreach (SubstitutionInfo sub, substitutions) {
        bookmarks << createBookmark(corpusID, communicationID, annotationID, sub);
    }
    return bookmarks;
}
