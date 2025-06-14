#include <KAI/Network/ProxyDecl.h>

namespace KAI {
namespace Demo {
namespace Chat {
class IChatPeerProxy : public ProxyBase {
    using ProxyBase::StreamType;
    IChatPeerProxy(Node& node, NetHandle handle) : ProxyBase(node, handle) {}

    // Peer discovery and connection
    void AnnouncePresence(const PeerInfo& myInfo) {
        RakNet::BitStream args;
        args << myInfo;
        _node->Send("AnnouncePresence", args);
    }

    void RequestPeerInfo() { _node->Send("RequestPeerInfo"); }

    PeerInfo GetPeerInfo() {
        auto future = _node->SendWithResponse("GetPeerInfo");
        return future.get();
    }

    void PeerDisconnecting(const string& peerId) {
        RakNet::BitStream args;
        args << peerId;
        _node->Send("PeerDisconnecting", args);
    }

    // Message passing between peers
    void ReceiveMessage(const ChatMessage& message) {
        RakNet::BitStream args;
        args << message;
        _node->Send("ReceiveMessage", args);
    }

    void ReceivePrivateMessage(const ChatMessage& message) {
        RakNet::BitStream args;
        args << message;
        _node->Send("ReceivePrivateMessage", args);
    }

    void ForwardMessage(const ChatMessage& message,
                        const string[] & excludePeers) {
        RakNet::BitStream args;
        args << message;
        args << excludePeers;
        _node->Send("ForwardMessage", args);
    }

    // Channel coordination
    void JoinChannelRequest(const string& channelName,
                            const PeerInfo& peerInfo) {
        RakNet::BitStream args;
        args << channelName;
        args << peerInfo;
        _node->Send("JoinChannelRequest", args);
    }

    void LeaveChannelNotification(const string& channelName,
                                  const string& peerId) {
        RakNet::BitStream args;
        args << channelName;
        args << peerId;
        _node->Send("LeaveChannelNotification", args);
    }

    void ChannelStateUpdate(const ChannelState& state) {
        RakNet::BitStream args;
        args << state;
        _node->Send("ChannelStateUpdate", args);
    }

    void RequestChannelState(const string& channelName) {
        RakNet::BitStream args;
        args << channelName;
        _node->Send("RequestChannelState", args);
    }

    // Peer status updates
    void PeerStatusChanged(const string& peerId, const string& newStatus) {
        RakNet::BitStream args;
        args << peerId;
        args << newStatus;
        _node->Send("PeerStatusChanged", args);
    }

    void PeerJoinedChannel(const string& peerId, const string& channelName) {
        RakNet::BitStream args;
        args << peerId;
        args << channelName;
        _node->Send("PeerJoinedChannel", args);
    }

    void PeerLeftChannel(const string& peerId, const string& channelName) {
        RakNet::BitStream args;
        args << peerId;
        args << channelName;
        _node->Send("PeerLeftChannel", args);
    }

    // Distributed channel management
    void ProposeChannelCreation(const ChannelState& newChannel) {
        RakNet::BitStream args;
        args << newChannel;
        _node->Send("ProposeChannelCreation", args);
    }

    void VoteChannelCreation(const string& channelName, const bool& approve) {
        RakNet::BitStream args;
        args << channelName;
        args << approve;
        _node->Send("VoteChannelCreation", args);
    }

    void ChannelCreationResult(const string& channelName, const bool& created) {
        RakNet::BitStream args;
        args << channelName;
        args << created;
        _node->Send("ChannelCreationResult", args);
    }

    // Network maintenance
    void Heartbeat(const string& peerId, const int64& timestamp) {
        RakNet::BitStream args;
        args << peerId;
        args << timestamp;
        _node->Send("Heartbeat", args);
    }

    void RequestPeerList() { _node->Send("RequestPeerList"); }

    void SharePeerList(const PeerInfo[] & knownPeers) {
        RakNet::BitStream args;
        args << knownPeers;
        _node->Send("SharePeerList", args);
    }

    // Event registration methods
    void RegisterMessageReceivedHandler(
        std::function<void(ChatMessage)> handler) {
        RegisterEventHandler("MessageReceived", handler);
    }

    void UnregisterMessageReceivedHandler() {
        UnregisterEventHandler("MessageReceived");
    }

    void RegisterPrivateMessageReceivedHandler(
        std::function<void(ChatMessage)> handler) {
        RegisterEventHandler("PrivateMessageReceived", handler);
    }

    void UnregisterPrivateMessageReceivedHandler() {
        UnregisterEventHandler("PrivateMessageReceived");
    }

    void RegisterPeerJoinedHandler(std::function<void(PeerInfo)> handler) {
        RegisterEventHandler("PeerJoined", handler);
    }

    void UnregisterPeerJoinedHandler() { UnregisterEventHandler("PeerJoined"); }

    void RegisterPeerLeftHandler(std::function<void(string)> handler) {
        RegisterEventHandler("PeerLeft", handler);
    }

    void UnregisterPeerLeftHandler() { UnregisterEventHandler("PeerLeft"); }

    void RegisterPeerStatusUpdatedHandler(
        std::function<void(string, string)> handler) {
        RegisterEventHandler("PeerStatusUpdated", handler);
    }

    void UnregisterPeerStatusUpdatedHandler() {
        UnregisterEventHandler("PeerStatusUpdated");
    }

    void RegisterChannelMembershipChangedHandler(
        std::function<void(string, string[])> handler) {
        RegisterEventHandler("ChannelMembershipChanged", handler);
    }

    void UnregisterChannelMembershipChangedHandler() {
        UnregisterEventHandler("ChannelMembershipChanged");
    }

    void RegisterChannelCreatedHandler(
        std::function<void(ChannelState)> handler) {
        RegisterEventHandler("ChannelCreated", handler);
    }

    void UnregisterChannelCreatedHandler() {
        UnregisterEventHandler("ChannelCreated");
    }
};

class IChatManagerProxy : public ProxyBase {
    using ProxyBase::StreamType;
    IChatManagerProxy(Node& node, NetHandle handle) : ProxyBase(node, handle) {}

    // Local peer management
    void Initialize(const string& username, const string& displayName) {
        RakNet::BitStream args;
        args << username;
        args << displayName;
        _node->Send("Initialize", args);
    }

    void SetStatus(const string& status) {
        RakNet::BitStream args;
        args << status;
        _node->Send("SetStatus", args);
    }

    PeerInfo GetMyInfo() {
        auto future = _node->SendWithResponse("GetMyInfo");
        return future.get();
    }

    // Channel operations
    void CreateChannel(const string& name, const string& description) {
        RakNet::BitStream args;
        args << name;
        args << description;
        _node->Send("CreateChannel", args);
    }

    void JoinChannel(const string& channelName) {
        RakNet::BitStream args;
        args << channelName;
        _node->Send("JoinChannel", args);
    }

    void LeaveChannel(const string& channelName) {
        RakNet::BitStream args;
        args << channelName;
        _node->Send("LeaveChannel", args);
    }

    string[] GetMyChannels() {
        auto future = _node->SendWithResponse("GetMyChannels");
        return future.get();
    }

    // Messaging
    void SendMessage(const string& content, const string& channel) {
        RakNet::BitStream args;
        args << content;
        args << channel;
        _node->Send("SendMessage", args);
    }

    void SendPrivateMessage(const string& content, const string& targetPeerId) {
        RakNet::BitStream args;
        args << content;
        args << targetPeerId;
        _node->Send("SendPrivateMessage", args);
    }

    ChatMessage[] GetChannelMessages(const string& channel) {
        RakNet::BitStream args;
        args << channel;
        auto future = _node->SendWithResponse("GetChannelMessages", args);
        return future.get();
    }

    ChatMessage[] GetPrivateMessages(const string& peerId) {
        RakNet::BitStream args;
        args << peerId;
        auto future = _node->SendWithResponse("GetPrivateMessages", args);
        return future.get();
    }

    // Peer discovery
    PeerInfo[] GetKnownPeers() {
        auto future = _node->SendWithResponse("GetKnownPeers");
        return future.get();
    }

    PeerInfo[] GetPeersInChannel(const string& channel) {
        RakNet::BitStream args;
        args << channel;
        auto future = _node->SendWithResponse("GetPeersInChannel", args);
        return future.get();
    }

    void RefreshPeerList() { _node->Send("RefreshPeerList"); }

    // Network operations
    void StartNetworking() { _node->Send("StartNetworking"); }

    void StopNetworking() { _node->Send("StopNetworking"); }

    bool IsNetworkActive() {
        auto future = _node->SendWithResponse("IsNetworkActive");
        return future.get();
    }

    // Event registration methods
    void RegisterNetworkStatusChangedHandler(
        std::function<void(bool)> handler) {
        RegisterEventHandler("NetworkStatusChanged", handler);
    }

    void UnregisterNetworkStatusChangedHandler() {
        UnregisterEventHandler("NetworkStatusChanged");
    }

    void RegisterMessageAddedHandler(std::function<void(ChatMessage)> handler) {
        RegisterEventHandler("MessageAdded", handler);
    }

    void UnregisterMessageAddedHandler() {
        UnregisterEventHandler("MessageAdded");
    }

    void RegisterChannelListUpdatedHandler(
        std::function<void(string[])> handler) {
        RegisterEventHandler("ChannelListUpdated", handler);
    }

    void UnregisterChannelListUpdatedHandler() {
        UnregisterEventHandler("ChannelListUpdated");
    }

    void RegisterPeerListUpdatedHandler(
        std::function<void(PeerInfo[])> handler) {
        RegisterEventHandler("PeerListUpdated", handler);
    }

    void UnregisterPeerListUpdatedHandler() {
        UnregisterEventHandler("PeerListUpdated");
    }
};
}  // namespace Chat
}  // namespace Demo
}  // namespace KAI