#ifndef WORDALIGN_H
#define WORDALIGN_H

#include <QString>
#include <QList>

struct WordAlignData;

class WordAlign
{
public:
    class AlignmentItem {
    public:
        QString ref;
        QString hyp;
        QString op;
    };

    WordAlign();
    ~WordAlign();

    void align(QList<QString> wordsReference, QList<QString> wordsHypothesis);

    QList<AlignmentItem> alignment() const;
    QString alignmentText() const;
    QString alignmentTextOriginal(const QString &format) const;
    QString alignmentTextModified(const QString &format) const;

    double WER() const;
    int insertionsCount() const;
    int deletionsCount() const;
    int substitutionsCount() const;
    int matchesCount() const;
    int wordsReferenceCount() const;
    int wordsHypothesisCount() const;

private:
    void initialise();
    void backtrace();

    WordAlignData *d;
};

#endif // WORDALIGN_H
