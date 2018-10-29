#ifndef UDSENTENCE_H
#define UDSENTENCE_H

#include <QString>
#include <QList>
#include <QMap>
#include "UDToken.h"
#include "UDTokenMultiWord.h"
#include "UDEmptyNode.h"

class UDSentence
{
public:
    UDSentence();
    virtual ~UDSentence();

    // Basic sentence modifications
    bool empty();
    void clear();
    UDToken& addToken(const QString &form = QString());
    void setHead(int id, int head, const QString &deprel);
    void unlinkAllTokens();

    // CoNLL-U defined comments
    bool getNewDoc(QString &id) const;
    void setNewDoc(bool newDoc, const QString &id = QString());
    bool getNewPar(QString &id) const;
    void setNewPar(bool newPar, const QString &id = QString());
    bool getSentID(QString &id) const;
    void setSentID(const QString &id);
    bool getText(QString &text) const;
    void setText(const QString &text);

protected:
    QList<UDToken> m_tokens;
    QList<UDTokenMultiWord> m_tokensMultiword;
    QList<UDEmptyNode> m_emptyNodes;
    QStringList m_comments;
    static const QString root_form;
};

#endif // UDSENTENCE_H
