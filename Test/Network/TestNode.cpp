#include "TestCommon.h"
#include <gtest/gtest.h>
#include <KAI/Network/Network.h>
#include <KAI/Network/Serialization.h>
#include <KAI/Network/RakNetStub.h>
#include <KAI/Core/BuiltinTypes/All.h>
#include <KAI/Core/Registry.h>
#include <thread>
#include <chrono>

// Use the common test namespace
using namespace kai;
using namespace kai::net;

// Basic network creation test
TEST(Network, CreateNode)
{
    // Create a network node
    Node node;
    
    // Verify initial state
    ASSERT_FALSE(node.IsRunning());
    
    // Initialize on a random port
    ASSERT_NO_THROW(node.Listen(0));
    
    // Verify node is running
    ASSERT_TRUE(node.IsRunning());
    
    // Clean up
    node.Shutdown();
    ASSERT_FALSE(node.IsRunning());
}

// Test peer discovery
TEST(Network, PeerDiscovery)
{
    // This test is disabled for automated testing since it requires network access
    if (getenv("KAI_NETWORK_TEST_DISCOVERY") == nullptr)
    {
        GTEST_SKIP() << "Skipping peer discovery test. Set KAI_NETWORK_TEST_DISCOVERY=1 to enable.";
    }
    
    // Create two nodes
    Node node1;
    Node node2;
    
    // Initialize both nodes on different ports
    node1.Listen(14589);
    node2.Listen(14590);
    
    // Start peer discovery
    node1.StartDiscovery();
    node2.StartDiscovery();
    
    // Give some time for discovery
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    // Process messages
    node1.Update();
    node2.Update();
    
    // Get discovered peers (may be empty if no nodes found on local network)
    auto peers1 = node1.GetDiscoveredPeers();
    auto peers2 = node2.GetDiscoveredPeers();
    
    // Stop discovery
    node1.StopDiscovery();
    node2.StopDiscovery();
    
    // Clean up
    node1.Shutdown();
    node2.Shutdown();
}

// Test connection between two nodes
TEST(Network, Connect)
{
    // This test is disabled for automated testing since it requires network access
    if (getenv("KAI_NETWORK_TEST_CONNECT") == nullptr)
    {
        GTEST_SKIP() << "Skipping connection test. Set KAI_NETWORK_TEST_CONNECT=1 to enable.";
    }
    
    // Create two nodes
    Node server;
    Node client;
    
    // Initialize server node on a specific port
    server.Listen(14591);
    
    // Initialize client node
    client.Listen(0);  // Any available port
    
    // Connect client to server
    IpAddress localhost("127.0.0.1");
    client.Connect(localhost, 14591);
    
    // Give some time for connection
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // Process messages
    server.Update();
    client.Update();
    
    // Check connection status
    ASSERT_EQ(server.GetConnectionCount(), 1);
    ASSERT_EQ(client.GetConnectionCount(), 1);
    
    // Disconnect
    client.Disconnect();
    
    // Clean up
    server.Shutdown();
    client.Shutdown();
}

// Test object serialization
TEST(Network, Serialization)
{
    // Skip this test for now since it requires more integration
    GTEST_SKIP() << "Skipping serialization test until core types are fully integrated";
    
    // This is a stub for future implementation
    // Create a BitStream
    RakNet::BitStream bitStream;
    
    // Verify operations can be performed on the BitStream
    unsigned int testValue = 42;
    bitStream.Write(testValue);
    
    // Verify data was written
    ASSERT_GT(bitStream.GetNumberOfBytesUsed(), 0);
    
    // Reset read position and read back
    bitStream.IgnoreBytes(0);  // Reset to beginning for reading
    
    unsigned int readValue = 0;
    bitStream.Read(readValue);
    
    // Verify we can read what we wrote
    ASSERT_EQ(readValue, testValue);
}

// Test with more complex object
TEST(Network, ComplexSerialization) 
{
    // Skip this test for now since it requires more integration
    GTEST_SKIP() << "Skipping complex serialization test until core types are fully integrated";
    
    // This test is a skeleton for future implementation
    // when the serialization system is fully integrated
}

// End of tests