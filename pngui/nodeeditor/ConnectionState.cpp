#include "ConnectionState.h"

#include <iostream>

#include <QtCore/QPointF>

#include "FlowScene.h"
#include "Node.h"

ConnectionState::
~ConnectionState()
{
    resetLastHoveredNode();
}


void
ConnectionState::
interactWithNode(std::shared_ptr<Node> node)
{
    if (node)
    {
        _lastHoveredNode = node;
    }
    else
    {
        resetLastHoveredNode();
    }
}


void
ConnectionState::
setLastHoveredNode(std::shared_ptr<Node> node)
{
    _lastHoveredNode = node;
}


void
ConnectionState::
resetLastHoveredNode()
{
    auto node = _lastHoveredNode.lock();

    if (node)
        node->resetReactionToConnection();

    _lastHoveredNode.reset();
}
