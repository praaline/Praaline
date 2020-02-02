#ifndef DISFLUENCYPATTERNDETECTOR_H
#define DISFLUENCYPATTERNDETECTOR_H

#include <QPair>
#include <QString>
#include <QList>

#include "PraalineCore/Annotation/IntervalTier.h"
#include "PraalineCore/Annotation/AnnotationTierGroup.h"
#include "PraalineCore/Corpus/CorpusBookmark.h"
using namespace Praaline::Core;

struct DisfluencyPatternDetectorData;

class DisfluencyPatternDetector
{
public:
    class PatternInfoBase {
    public:
        virtual ~PatternInfoBase() {}
        QList<int> indices;
        QString text;
        virtual QString type() const = 0;
    };

    class RepetitionInfo : public PatternInfoBase {
    public:
        virtual ~RepetitionInfo() {}
        int window;
        int times;
        QString type() const { return "REP"; }
    };

    class InsertionInfo : public PatternInfoBase {
    public:
        virtual ~InsertionInfo() {}
        int lengthBefore;
        int lengthInsertion;
        int lengthAfter;
        QString type() const { return "INS"; }
    };

    class SubstitutionInfo : public PatternInfoBase {
    public:
        virtual ~SubstitutionInfo() {}
        int lengthBefore;
        int lengthOldTokens;
        int lengthNewTokens;
        int lengthAfter;
        QString type() const { return "SUB"; }
    };

    DisfluencyPatternDetector();
    virtual ~DisfluencyPatternDetector();

    void setTierInfoToken(const QString &levelID, const QString &attributeID);
    void setTierInfoPOS(const QString &levelID, const QString &attributeID);
    void setTierInfoDisfluecny(const QString &levelID, const QString &attributeID);
    void setTiers(AnnotationTierGroup *tiergroup);
    void setTiers(IntervalTier *tierToken, IntervalTier *tierPOS, IntervalTier *tierDisfluecny);

    QList<int> indicesWithoutSimpleDisfluencies(bool withDMlist = false, int from = 0, int to = -1);

    void revertToDisfluenciesLevel1();
    QList<RepetitionInfo> detectRepetitionPatterns();

    QList<InsertionInfo> detectInsertionPatterns();
    void codeInsertions(QList<InsertionInfo> &insertions);

    QList<SubstitutionInfo> detectSubstitutionPatterns();
    void codeSubstitutions(QList<SubstitutionInfo> &substitutions);

    QList<CorpusBookmark *> createBookmarks(const QString &corpusID, const QString &communicationID, const QString &annotationID,
                                                     QList<RepetitionInfo> &repetitions);
    QList<CorpusBookmark *> createBookmarks(const QString &corpusID, const QString &communicationID, const QString &annotationID,
                                                     QList<InsertionInfo> &insertions);
    QList<CorpusBookmark *> createBookmarks(const QString &corpusID, const QString &communicationID, const QString &annotationID,
                                                     QList<SubstitutionInfo> &substitutions);
private:
    DisfluencyPatternDetectorData *d;

    inline QString token(QList<int> &indices, int i) const;
    inline QString pos(QList<int> &indices, int i) const;
    inline QString disfluency(QList<int> &indices, int i) const;

    bool matchAll(QList<int> &indices, int start, int window);
    bool match(QList<int> &indices, int startA, int startB, int length);

    void codeRepetitions(QList<RepetitionInfo> &repetitions, bool withDM);

    CorpusBookmark *createBookmark(const QString &corpusID, const QString &communicationID, const QString &annotationID,
                                            const PatternInfoBase &pattern);
};

#endif // DISFLUENCYPATTERNDETECTOR_H
