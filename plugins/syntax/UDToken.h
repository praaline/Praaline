#ifndef UDTOKEN_H
#define UDTOKEN_H

#include <QString>
#include <QList>
#include <QPair>
#include "UDTokenBase.h"

class UDToken : public UDTokenBase
{
public:
    UDToken(int id = -1, QString form = QString());
    UDToken(const UDToken &token);
    virtual ~UDToken();

    int ID() const;
    void setID(int id);
    QString lemma() const;
    void setLemma(const QString &lemma);
    QString UPOSTag() const;
    void setUPOSTag(const QString &UPOSTag);
    QString XPOSTag() const;
    void setXPOSTag(const QString &XPOSTag);
    QString feat(const QString &key) const;
    void setFeat(const QString &key, const QString &value);
    QString featsToString() const;
    int head() const;
    void setHead(int head);
    QString depRel() const;
    void setDepRel(const QString &deprel);
    QList<QPair<int, QString> > &deps();
    void appendDep(int id, const QString &dep);
    void clearDeps();
    QList<int> &children();

protected:
    // m_form and m_misc are inherited from UDTokenBase
    int m_id;                           // 0 is root, >0 is sentence word, <0 is undefined
    QString m_lemma;                    // lemma
    QString m_upostag;                  // universal part-of-speech tag
    QString m_xpostag;                  // language-specific part-of-speech tag
    QMap<QString, QString> m_feats;     // list of morphological features
    int m_head;                         // head, 0 is root, <0 is undefined
    QString m_deprel;                   // dependency relation to the head
    QList<QPair<int, QString> > m_deps; // secondary dependencies
    QList<int> m_children;
};

#endif // UDTOKEN_H
