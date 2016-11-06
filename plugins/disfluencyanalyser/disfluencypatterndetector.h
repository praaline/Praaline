#ifndef DISFLUENCYPATTERNDETECTOR_H
#define DISFLUENCYPATTERNDETECTOR_H

#include <QPair>
#include <QString>
#include <QList>

#include "pncore/annotation/IntervalTier.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/corpus/CorpusBookmark.h"
using namespace Praaline::Core;

class DisfluencyPatternDetector
{
public:
    class PatternInfoBase {
    public:
        QList<int> indices;
        QString text;
        virtual QString type() const = 0;
    };

    class RepetitionInfo : public PatternInfoBase {
    public:
        int window;
        int times;
        QString type() const { return "REP"; }
    };

    class InsertionInfo : public PatternInfoBase {
    public:
        int lengthBefore;
        int lengthInsertion;
        int lengthAfter;
        QString type() const { return "INS"; }
    };

    class SubstitutionInfo : public PatternInfoBase {
    public:
        int lengthBefore;
        int lengthOldTokens;
        int lengthNewTokens;
        int lengthAfter;
        QString type() const { return "SUB"; }
    };

    DisfluencyPatternDetector();
    ~DisfluencyPatternDetector();

    void setTierInfoToken(const QString &levelID, const QString &attributeID);
    void setTierInfoPOS(const QString &levelID, const QString &attributeID);
    void setTierInfoDisfluecny(const QString &levelID, const QString &attributeID);
    void setTiers(AnnotationTierGroup *tiergroup);
    void setTiers(IntervalTier *tierToken, IntervalTier *tierPOS, IntervalTier *tierDisfluecny);

    QList<int> indicesWithoutSimpleDisfluencies(int from = 0, int to = -1);

    QList<RepetitionInfo> detectRepetitionPatterns();
    void codeRepetitions(QList<RepetitionInfo> &repetitions);

    QList<InsertionInfo> detectInsertionPatterns();
    void codeInsertions(QList<InsertionInfo> &insertions);

    QList<SubstitutionInfo> detectSubstitutionPatterns();
    void codeSubstitutions(QList<SubstitutionInfo> &substitutions);

    QList<QPointer<CorpusBookmark> > createBookmarks(const QString &corpusID, const QString &communicationID, const QString &annotationID,
                                                     QList<RepetitionInfo> &repetitions);
    QList<QPointer<CorpusBookmark> > createBookmarks(const QString &corpusID, const QString &communicationID, const QString &annotationID,
                                                     QList<InsertionInfo> &insertions);
    QList<QPointer<CorpusBookmark> > createBookmarks(const QString &corpusID, const QString &communicationID, const QString &annotationID,
                                                     QList<SubstitutionInfo> &substitutions);
private:
    QPair<QString, QString> m_tierinfoToken;
    QPair<QString, QString> m_tierinfoPOS;
    QPair<QString, QString> m_tierinfoDisfluency;
    IntervalTier *m_tierToken;
    IntervalTier *m_tierPOS;
    IntervalTier *m_tierDisfluency;

    inline QString token(QList<int> &indices, int i) const;
    inline QString pos(QList<int> &indices, int i) const;
    inline QString disfluency(QList<int> &indices, int i) const;

    bool matchAll(QList<int> &indices, int start, int window);
    bool match(QList<int> &indices, int startA, int startB, int length);

    QPointer<CorpusBookmark> createBookmark(const QString &corpusID, const QString &communicationID, const QString &annotationID,
                                            const PatternInfoBase &pattern);
};

#endif // DISFLUENCYPATTERNDETECTOR_H
