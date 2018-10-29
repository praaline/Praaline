#include "UDEmptyNode.h"

UDEmptyNode::UDEmptyNode(int ID, int index) :
    UDToken(ID), m_index(index)
{
}

UDEmptyNode::~UDEmptyNode()
{
}

int UDEmptyNode::index() const
{
    return m_index;
}

void UDEmptyNode::setIndex(int index)
{
    m_index = index;
}

