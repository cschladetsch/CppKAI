#include <KAI/Network/RakNetStub.h>
#include <gtest/gtest.h>

#include <string>

// Test basic network functionality only
TEST(RakNetSimple, BasicStubTest) {
    // Create a simple BitStream
    RakNet::BitStream bs;

    // Test writing and reading
    unsigned int testVal = 42;
    bs.Write(testVal);

    // Verify size
    EXPECT_GT(bs.GetNumberOfBytesUsed(), 0);

    // Test reading
    bs.IgnoreBytes(0);
    unsigned int readVal = 0;
    bs.Read(readVal);
    EXPECT_EQ(readVal, testVal);

    // Test SystemAddress
    RakNet::SystemAddress addr("127.0.0.1", 8888);
    EXPECT_EQ(addr.ToString(), "127.0.0.1:8888");

    // Test RakPeerInterface factory
    RakNet::RakPeerInterface* peer = RakNet::RakPeerInterface::GetInstance();
    ASSERT_NE(peer, nullptr);

    // Test stub implementation doesn't crash
    ASSERT_NO_THROW(peer->Startup(32, nullptr, 0));
    ASSERT_NO_THROW(peer->Shutdown(0));

    // Success if we get here without crashing
    SUCCEED();
}