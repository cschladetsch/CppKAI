#include "KAI/Network/ProxyBase.h"

KAI_NET_BEGIN

// Stub implementation to allow compilation
void Representative::Receive(NetHandle sender, BinaryStream &packet) {
    KAI_UNUSED_2(sender, packet);
}

void Representative::Receive(NetHandle sender, StringStream &packet) {
    KAI_UNUSED_2(sender, packet);
}

void Representative::Send(NetHandle recipient, const char *text) {
    KAI_UNUSED_2(recipient, text);
}

void Representative::Send(NetHandle recipient, BinaryPacket const &packet) {
    KAI_UNUSED_2(recipient, packet);
}

NetHandle GetNetHandle(Object const &t, Node const &node) {
    KAI_UNUSED_2(t, node);
    return NetHandle(0);
}

KAI_NET_END
