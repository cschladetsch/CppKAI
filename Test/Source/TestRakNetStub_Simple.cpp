#include <KAI/Network/RakNetStub.h>
#include <gtest/gtest.h>

TEST(RakNetStubSimple, BasicTest) {
    // Just create and verify a few basic types
    RakNet::SystemAddress addr;
    RakNet::BitStream stream;

    EXPECT_NO_THROW({
        addr = RakNet::SystemAddress("127.0.0.1", 8080);
        stream.Write((unsigned char)42);
    });

    EXPECT_EQ(addr.ToString(), "127.0.0.1:8080");
}