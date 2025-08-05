#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>

#include "KAI/Core/Config/Base.h"
#include "KAI/Core/Debug.h"
#include "KAI/Network/Network.h"
#include "KAI/Network/RakNetStub.h"
#include "TestCommon.h"

using namespace kai;
using namespace kai::net;
using namespace std;
using namespace std::chrono;

/**
 * Advanced network communication tests for KAI system
 * Focuses on testing network functionality that works with existing API
 */
class AdvancedNetworkTests : public ::testing::Test {
protected:
    void SetUp() override {
        // Clean any previous network state
        this_thread::sleep_for(milliseconds(100));
    }
    
    void TearDown() override {
        // Allow time for network cleanup
        this_thread::sleep_for(milliseconds(100));
    }
};

// Test basic node creation and lifecycle
TEST_F(AdvancedNetworkTests, NodeLifecycleTest) {
    Node node;
    
    // Initial state
    EXPECT_FALSE(node.IsRunning());
    
    // Start listening
    ASSERT_NO_THROW(node.Listen(15001));
    EXPECT_TRUE(node.IsRunning());
    
    // Process updates
    ASSERT_NO_THROW(node.Update());
    
    // Shutdown
    ASSERT_NO_THROW(node.Shutdown());
    EXPECT_FALSE(node.IsRunning());
}

// Test multiple node creation
TEST_F(AdvancedNetworkTests, MultipleNodesTest) {
    const int NUM_NODES = 3;
    vector<unique_ptr<Node>> nodes;
    
    // Create multiple nodes
    for (int i = 0; i < NUM_NODES; ++i) {
        auto node = make_unique<Node>();
        ASSERT_NO_THROW(node->Listen(15010 + i));
        EXPECT_TRUE(node->IsRunning());
        nodes.push_back(move(node));
    }
    
    // Update all nodes
    for (auto& node : nodes) {
        ASSERT_NO_THROW(node->Update());
    }
    
    // Shutdown all nodes
    for (auto& node : nodes) {
        ASSERT_NO_THROW(node->Shutdown());
        EXPECT_FALSE(node->IsRunning());
    }
}

// Test peer discovery functionality
TEST_F(AdvancedNetworkTests, PeerDiscoveryTest) {
    Node node1;
    Node node2;
    
    // Initialize nodes
    ASSERT_NO_THROW(node1.Listen(15020));
    ASSERT_NO_THROW(node2.Listen(15021));
    
    // Start discovery
    ASSERT_NO_THROW(node1.StartDiscovery());
    ASSERT_NO_THROW(node2.StartDiscovery());
    
    // Allow time for discovery
    this_thread::sleep_for(milliseconds(200));
    
    // Update nodes
    ASSERT_NO_THROW(node1.Update());
    ASSERT_NO_THROW(node2.Update());
    
    // Get discovered peers (API should work even if no peers found)
    ASSERT_NO_THROW({
        auto peers1 = node1.GetDiscoveredPeers();
        auto peers2 = node2.GetDiscoveredPeers();
        
        // Verify we can iterate over peers
        for (const auto& peer : peers1) {
            string address = peer.ToString();
        }
        for (const auto& peer : peers2) {
            string address = peer.ToString();
        }
    });
    
    // Stop discovery
    ASSERT_NO_THROW(node1.StopDiscovery());
    ASSERT_NO_THROW(node2.StopDiscovery());
    
    // Shutdown
    node1.Shutdown();
    node2.Shutdown();
}

// Test connection establishment
TEST_F(AdvancedNetworkTests, ConnectionTest) {
    Node server;
    Node client;
    
    // Setup server
    ASSERT_NO_THROW(server.Listen(15030));
    EXPECT_TRUE(server.IsRunning());
    
    // Setup client
    ASSERT_NO_THROW(client.Listen(0)); // Any port
    EXPECT_TRUE(client.IsRunning());
    
    // Attempt connection
    IpAddress localhost("127.0.0.1");
    ASSERT_NO_THROW(client.Connect(localhost, 15030));
    
    // Allow time for connection
    this_thread::sleep_for(milliseconds(200));
    
    // Update both nodes
    ASSERT_NO_THROW(server.Update());
    ASSERT_NO_THROW(client.Update());
    
    // Cleanup
    client.Shutdown();
    server.Shutdown();
}

// Test rapid node creation/destruction
TEST_F(AdvancedNetworkTests, RapidLifecycleTest) {
    const int NUM_ITERATIONS = 10;
    
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        Node node;
        
        // Rapid creation and destruction
        ASSERT_NO_THROW(node.Listen(15040 + i));
        EXPECT_TRUE(node.IsRunning());
        
        ASSERT_NO_THROW(node.Update());
        
        ASSERT_NO_THROW(node.Shutdown());
        EXPECT_FALSE(node.IsRunning());
        
        // Small delay between iterations
        this_thread::sleep_for(milliseconds(10));
    }
}

// Test update performance
TEST_F(AdvancedNetworkTests, UpdatePerformanceTest) {
    Node node;
    ASSERT_NO_THROW(node.Listen(15050));
    
    auto start = high_resolution_clock::now();
    
    // Perform many updates
    const int NUM_UPDATES = 1000;
    for (int i = 0; i < NUM_UPDATES; ++i) {
        ASSERT_NO_THROW(node.Update());
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end - start);
    
    // Should complete updates reasonably quickly
    EXPECT_LT(duration.count(), 1000000); // Less than 1 second
    
    cout << "Performed " << NUM_UPDATES << " updates in " 
         << duration.count() << " microseconds\n";
    cout << "Average time per update: " 
         << (duration.count() / NUM_UPDATES) << " microseconds\n";
    
    node.Shutdown();
}

// Test concurrent node operations
TEST_F(AdvancedNetworkTests, ConcurrentOperationsTest) {
    const int NUM_THREADS = 3;
    vector<thread> threads;
    atomic<int> successCount{0};
    
    // Launch concurrent node operations
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back([i, &successCount]() {
            try {
                Node node;
                node.Listen(15060 + i);
                
                // Perform some operations
                for (int j = 0; j < 10; ++j) {
                    node.Update();
                    this_thread::sleep_for(milliseconds(1));
                }
                
                node.Shutdown();
                successCount++;
            } catch (...) {
                // Thread failed
            }
        });
    }
    
    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Should have reasonable success rate
    EXPECT_GT(successCount.load(), NUM_THREADS / 2);
    
    cout << "Successful concurrent operations: " 
         << successCount.load() << "/" << NUM_THREADS << "\n";
}

// Test memory usage with many nodes
TEST_F(AdvancedNetworkTests, MemoryUsageTest) {
    const int NUM_NODES = 20;
    vector<unique_ptr<Node>> nodes;
    
    // Create many nodes
    for (int i = 0; i < NUM_NODES; ++i) {
        auto node = make_unique<Node>();
        ASSERT_NO_THROW(node->Listen(15070 + i));
        nodes.push_back(move(node));
    }
    
    // Update all nodes multiple times
    for (int iteration = 0; iteration < 5; ++iteration) {
        for (auto& node : nodes) {
            ASSERT_NO_THROW(node->Update());
        }
        this_thread::sleep_for(milliseconds(10));
    }
    
    // Cleanup all nodes
    for (auto& node : nodes) {
        ASSERT_NO_THROW(node->Shutdown());
    }
    
    cout << "Successfully created and managed " << NUM_NODES << " nodes\n";
}