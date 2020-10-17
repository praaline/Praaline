#include "UDSentence.h"

UDSentence::UDSentence()
{
}

UDSentence::~UDSentence()
{
}

// Basic sentence modifications
bool UDSentence::empty()
{
    return false;
}

void UDSentence::clear()
{
}

UDToken& UDSentence::addToken(const QString &form)
{
    // TODO: Implement this!
    Q_UNUSED(form)
    return m_tokens[0];
}

void UDSentence::setHead(int id, int head, const QString &deprel)
{
    Q_UNUSED(id)
    Q_UNUSED(head)
    Q_UNUSED(deprel)
}

void UDSentence::unlinkAllTokens()
{
}

// CoNLL-U defined comments
bool UDSentence::getNewDoc(QString &id) const
{
    Q_UNUSED(id)
    return false;
}

void UDSentence::setNewDoc(bool newDoc, const QString &id)
{
    Q_UNUSED(newDoc)
    Q_UNUSED(id)
}

bool UDSentence::getNewPar(QString &id) const
{
    Q_UNUSED(id)
    return false;
}

void UDSentence::setNewPar(bool newPar, const QString &id)
{
    Q_UNUSED(newPar)
    Q_UNUSED(id)
}

bool UDSentence::getSentID(QString &id) const
{
    Q_UNUSED(id)
    return false;
}

void UDSentence::setSentID(const QString &id)
{
    Q_UNUSED(id)
}

bool UDSentence::getText(QString &text) const
{
    Q_UNUSED(text)
    return false;
}

void UDSentence::setText(const QString &text)
{
    Q_UNUSED(text)
}
