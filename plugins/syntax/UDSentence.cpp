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
}

void UDSentence::setHead(int id, int head, const QString &deprel)
{
}

void UDSentence::unlinkAllTokens()
{
}

// CoNLL-U defined comments
bool UDSentence::getNewDoc(QString &id) const
{
    return false;
}

void UDSentence::setNewDoc(bool newDoc, const QString &id)
{

}

bool UDSentence::getNewPar(QString &id) const
{
    return false;
}

void UDSentence::setNewPar(bool newPar, const QString &id)
{

}

bool UDSentence::getSentID(QString &id) const
{
    return false;
}

void UDSentence::setSentID(const QString &id)
{

}

bool UDSentence::getText(QString &text) const
{
    return false;
}

void UDSentence::setText(const QString &text)
{

}
