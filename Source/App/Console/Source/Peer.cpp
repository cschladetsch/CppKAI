#if 0
#include "KAI/Network/Peer.h"

#include <iostream>

#include "KAI/ClassBuilder.h"

using namespace std;

KAI_BEGIN

Peer::Peer()
    : nextPeerId_(1)
{
    peer_ = RakNet::RakPeerInterface::GetInstance();
    
    peer_->SetTimeoutTime(3000, RakNet::UNASSIGNED_SYSTEM_ADDRESS);
    peer_->DisableSecurity();
}

bool Peer::Start(int listenPort, PacketReceiveCallback r0, ReceiveCallback r1)
{
    // attempt both IPV4 and V6
    socketDescriptors_[0].port = listenPort;
    socketDescriptors_[0].socketFamily = AF_INET;
    socketDescriptors_[1].port = listenPort;
    socketDescriptors_[1].socketFamily = AF_INET6;
    // TODO: try IP6 first. this only attempts IPV4
    // confusingly, the last parameter to Startup
    // is 1-based: so this listens on IP4. changing
    // the 3rd parameter to 2 attempts ip6...
    int startResult = peer_->Startup(MaxConnections, socketDescriptors_, 1);

    bool started = startResult == RakNet::RAKNET_STARTED;
    
    if (!started)
        return false;

    peer_->SetMaximumIncomingConnections(MaxConnections);
    peer_->SetOccasionalPing(true);
    peer_->SetUnreliableTimeout(1000);

    peer_->GetSockets(sockets_);

    return true;
}

bool Peer::Connect(const char *ip, int port)
{
    RakNet::PublicKey *publicKey = 0;
    const char *password = "";
    return peer_->Connect(ip, port, password, strlen(password)) == RakNet::CONNECTION_ATTEMPT_STARTED;
}

void Peer::Step()
{
    for (RakNet::Packet *p = peer_->Receive(); p; peer_->DeallocatePacket(p), p = peer_->Receive())
    {
        PacketType type = GetPacketIdentifier(p);
        switch (type)
        {
            case ID_NEW_INCOMING_CONNECTION:
                NewConnection(p);
                break;

            case ID_DISCONNECTION_NOTIFICATION:
                // Connection lost normally
                cout << "ID_DISCONNECTION_NOTIFICATION from " << AddressFromPacket(p) << endl;
                break;

            case ID_INCOMPATIBLE_PROTOCOL_VERSION:
                cout << "ID_INCOMPATIBLE_PROTOCOL_VERSION\n";
                break;

            case ID_CONNECTED_PING:
            case ID_UNCONNECTED_PING:
                cout << "Ping from " << AddressFromPacket(p) << endl;
                break;

            case ID_CONNECTION_LOST:
                // Couldn't deliver a reliable packet - i.e. the other system was abnormally terminated
                cout << "ID_CONNECTION_LOST from " << AddressFromPacket(p) << endl;
                break;            

            case GetRemoteListenPort:
                cout << "Got request for remote listen port from " << AddressFromPacket(p);
                break;

            //default:
            //    // just assume it's message data
            //    cout << p->data << endl;
        }
    }
}

// make a dual connection with the sender of packet
void Peer::HandShake(RakNet::Packet *p)
{
    // TODO:
    // Connect(p->systemAddress.ipAddress)
    // RequestPort(...)
    // DisconnectOriginal(...)
    // ReconnectAsRealPeer(...)

    //peers_[++nextPeerId_] = p->systemAddress;

    CompleteConnnection(p);
}

void Peer::NewConnection(RakNet::Packet *p)
{
    printf("Connection from %s with GUID %s\n", AddressFromPacket(p), p->guid.ToString());

    HandShake(p);
}

void Peer::CompleteConnnection(RakNet::Packet *p)
{
    printf("Remote internal IDs:\n");
    for (int index=0; index < MAXIMUM_NUMBER_OF_INTERNAL_IDS; index++)
    {
        RakNet::SystemAddress internalId = peer_->GetInternalID(p->systemAddress, index);
        if (internalId != RakNet::UNASSIGNED_SYSTEM_ADDRESS)
        {
            printf("%i. %s\n", index+1, internalId.ToString(true));
        }
    }
}

int Peer::SendText2(Pointer<String> str)
{
    return SendText(str->c_str());
}

int Peer::SendBlob(Pointer<BinaryPacket> packet)
{
    KAI_NOT_IMPLEMENTED();
}

int Peer::SendText(const char *text)
{
    int messageId = peer_->Send(text, strlen(text) + 1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
    if (messageId == 0)
    {
        cerr << "Failed to send " << text << endl;
        return 0;
    }

    return messageId;
}

void Peer::Shutdown()
{
    peer_->Shutdown(300);
    RakNet::RakPeerInterface::DestroyInstance(peer_);
}

const char *Peer::AddressFromPacket(RakNet::Packet *p)
{
    return p->systemAddress.ToString(true);
}

unsigned char Peer::GetPacketIdentifier(RakNet::Packet *p)
{
    if (p == nullptr)
        return 255;

    // skip over timestamp to payload
    // TODO: not ignore timestamp?
    if (static_cast<unsigned char>(p->data[0]) == ID_TIMESTAMP)
        return static_cast<unsigned char>(p->data[sizeof(RakNet::MessageID) + sizeof(RakNet::Time)]);
    else
        return static_cast<unsigned char>(p->data[0]);
}

void Peer::Register(Registry &reg)
{
    ClassBuilder<Peer>(reg, "Peer")
        .Methods
            ("SendText", &Peer::SendText2)
            ("SendBlob", &Peer::SendBlob)
        ;
}

StringStream &operator<<(StringStream &str, Peer const &peer)
{
    return str << "Peer is connected: " << peer.connected;
}

StringStream &operator<<(StringStream &str, RakNet::Packet const &packet)
{
    return str << "Packet from " << packet.guid.ToString() << ", length " << static_cast<int>(packet.length);
}
    
KAI_END

#endif
