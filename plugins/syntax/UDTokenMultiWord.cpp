#include "UDTokenMultiWord.h"

UDTokenMultiWord::UDTokenMultiWord(int IDFirst, int IDLast, const QString &form, const QString &misc) :
    UDTokenBase (form, misc), m_IDFirst(IDFirst), m_IDLast(IDLast)
{
}

UDTokenMultiWord::~UDTokenMultiWord()
{
}

int UDTokenMultiWord::IDFirst() const
{
    return m_IDFirst;
}

void UDTokenMultiWord::setIDFirst(int IDFirst)
{
    m_IDFirst = IDFirst;
}

int UDTokenMultiWord::IDLast() const
{
    return m_IDLast;
}

void UDTokenMultiWord::setIDLast(int IDLast)
{
    m_IDLast = IDLast;
}
