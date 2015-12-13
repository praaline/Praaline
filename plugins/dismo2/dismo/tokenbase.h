#ifndef TOKENBASE_H
#define TOKENBASE_H

#include <QString>
#include <QSet>

class TokenBase
{
public:
    TokenBase();
    ~TokenBase();
    
protected:
    //QSet<DictionaryEntry> m_possiblePOStags;
    QString m_punctuationBefore;
    QString m_punctuationAfter;
    QString m_lemma;
    QString m_tagPOS;
    QString m_tagPOSext;
    double m_confidencePOS;
    QString m_matchtypePOS;
};

#endif // TOKENBASE_H
