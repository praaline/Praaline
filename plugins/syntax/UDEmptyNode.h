#ifndef UDEMPTYNODE_H
#define UDEMPTYNODE_H

#include <QString>
#include "UDToken.h"

class UDEmptyNode : public UDToken
{
public:
    UDEmptyNode(int ID = -1, int index = 0);
    virtual ~UDEmptyNode();

    int index() const;
    void setIndex(int index);

protected:
    int m_index;
};

#endif // UDEMPTYNODE_H
