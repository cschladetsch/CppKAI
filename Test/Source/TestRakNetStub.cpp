#include <KAI/Network/RakNetStub.h>
#include <gtest/gtest.h>

#include <string>

// Test BitStream functionality
TEST(RakNetStubTest, BitStreamWorks) {
    // Create a BitStream
    RakNet::BitStream bs;

    // Test writing basic data types
    unsigned int testInt = 42;
    bs.Write(testInt);

    const char* testString = "Hello RakNet";
    size_t stringLen = strlen(testString) + 1;  // Include null terminator
    bs.Write(testString, stringLen);

    // Verify size
    EXPECT_EQ(bs.GetNumberOfBytesUsed(), sizeof(unsigned int) + stringLen);

    // Test reading data back
    bs.IgnoreBytes(0);  // Reset read position

    unsigned int readInt = 0;
    bs.Read(readInt);
    EXPECT_EQ(readInt, testInt);

    char readString[100] = {0};
    bs.Read(readString, stringLen);
    EXPECT_STREQ(readString, testString);
}

// Test SystemAddress functionality
TEST(RakNetStubTest, SystemAddressWorks) {
    // Create SystemAddress objects
    RakNet::SystemAddress addr1("127.0.0.1", 12345);
    RakNet::SystemAddress addr2("192.168.1.1", 54321);
    RakNet::SystemAddress addr3("127.0.0.1", 12345);

    // Test ToString
    EXPECT_EQ(addr1.ToString(), "127.0.0.1:12345");

    // Test equality
    EXPECT_EQ(addr1, addr3);
    EXPECT_NE(addr1, addr2);

    // Test FromString
    RakNet::SystemAddress addr4;
    addr4.FromString("10.0.0.1:8888");
    EXPECT_EQ(addr4.ip, "10.0.0.1");
    EXPECT_EQ(addr4.port, 8888);
}

// Test RakPeerInterface functionality
TEST(RakNetStubTest, RakPeerInterfaceWorks) {
    // Get instance
    RakNet::RakPeerInterface* peer = RakNet::RakPeerInterface::GetInstance();
    ASSERT_NE(peer, nullptr);

    // Test basic operations (these are stubs, so they should always succeed)
    RakNet::SocketDescriptor sd(12345, nullptr);
    RakNet::StartupResult result = peer->Startup(32, &sd, 1);
    EXPECT_EQ(result, RakNet::RAKNET_STARTED);

    peer->SetMaximumIncomingConnections(16);

    // Test Connect
    RakNet::ConnectionAttemptResult connectResult =
        peer->Connect("127.0.0.1", 54321, nullptr, 0);
    EXPECT_EQ(connectResult, RakNet::CONNECTION_ATTEMPT_STARTED);

    // Test Receive (should return nullptr in stub)
    RakNet::Packet* packet = peer->Receive();
    EXPECT_EQ(packet, nullptr);

    // Clean up
    peer->Shutdown(0);
    RakNet::RakPeerInterface::DestroyInstance(peer);
}

// Test time functionality
TEST(RakNetStubTest, TimeHandlingWorks) {
    RakNet::TimeMS time = RakNet::GetTimeMS();
    EXPECT_GT(time, 0);
}

// Test packet creation and handling
TEST(RakNetStubTest, PacketHandlingWorks) {
    // Create a packet manually
    RakNet::Packet packet;

    // Test SystemAddress
    packet.systemAddress = RakNet::SystemAddress("192.168.1.1", 12345);
    EXPECT_EQ(packet.systemAddress.ToString(), "192.168.1.1:12345");

    // Create and set data
    const char* testData = "Test packet data";
    size_t dataLen = strlen(testData) + 1;

    packet.data = new unsigned char[dataLen];
    memcpy(packet.data, testData, dataLen);
    packet.length = dataLen;

    // Verify data can be read
    EXPECT_STREQ((const char*)packet.data, testData);

    // Cleanup is handled by packet destructor
}