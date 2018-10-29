#include <QString>
#include <QList>
#include <QPair>
#include <QMap>
#include "UDTokenBase.h"
#include "UDToken.h"

UDToken::UDToken(int id, QString form) :
    UDTokenBase(form), m_id(id)
{
}

UDToken::UDToken(const UDToken &token) :
    UDTokenBase(token.m_form, token.m_misc),
    m_id(token.m_id), m_lemma(token.m_lemma), m_upostag(token.m_upostag), m_xpostag(token.m_xpostag),
    m_feats(token.m_feats), m_head(token.m_head), m_deprel(token.m_deprel), m_deps(token.m_deps),
    m_children(token.m_children)
{
}

UDToken::~UDToken()
{
}

int UDToken::ID() const
{
    return m_id;
}

void UDToken::setID(int id)
{
    m_id = id;
}

QString UDToken::lemma() const
{
    return m_lemma;
}

void UDToken::setLemma(const QString &lemma)
{
    m_lemma = lemma;
}

QString UDToken::UPOSTag() const
{
    return m_upostag;
}

void UDToken::setUPOSTag(const QString &UPOSTag)
{
    m_upostag = UPOSTag;
}

QString UDToken::XPOSTag() const
{
    return m_xpostag;
}

void UDToken::setXPOSTag(const QString &XPOSTag)
{
    m_xpostag = XPOSTag;
}

QString UDToken::feat(const QString &key) const
{
    return m_feats.value(key);
}

void UDToken::setFeat(const QString &key, const QString &value)
{
    m_feats.insert(key, value);
}

QString UDToken::featsToString() const
{
    return attributesToString(m_feats);
}

int UDToken::head() const
{
    return m_head;
}

void UDToken::setHead(int head)
{
    m_head = head;
}

QString UDToken::depRel() const
{
    return m_deprel;
}

void UDToken::setDepRel(const QString &deprel)
{
    m_deprel = deprel;
}

QList<QPair<int, QString> > &UDToken::deps()
{
    return m_deps;
}

void UDToken::appendDep(int id, const QString &dep)
{
    m_deps << QPair<int, QString>(id, dep);
}

void UDToken::clearDeps()
{
    m_deps.clear();
}

QList<int> &UDToken::children()
{
    return m_children;
}

