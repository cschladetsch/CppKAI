#include "KAI/Network/AgentBase.h"
#include "KAI/Network/Node.h"

KAI_NET_BEGIN

AgentBase::AgentBase(Node &node)
    : Representative(node, node.AttachAgent(this)) {}

AgentBase::~AgentBase() { GetNode().DetachAgent(GetHandle(), this); }

KAI_NET_END
