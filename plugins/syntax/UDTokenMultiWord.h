#ifndef UDTOKENMULTIWORD_H
#define UDTOKENMULTIWORD_H

#include "UDTokenBase.h"

class UDTokenMultiWord : public UDTokenBase
{
public:
    UDTokenMultiWord(int IDFirst, int IDLast, const QString &form, const QString &misc);
    virtual ~UDTokenMultiWord();

    int IDFirst() const;
    void setIDFirst(int IDFirst);
    int IDLast() const;
    void setIDLast(int IDLast);

protected:
    int m_IDFirst;
    int m_IDLast;
};

#endif // UDTOKENMULTIWORD_H
