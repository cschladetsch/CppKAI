#include <gtest/gtest.h>

#include <chrono>
#include <future>
#include <memory>
#include <string>
#include <thread>

#include "KAI/Console/Console.h"
#include "KAI/Network/Node.h"

using namespace std::chrono_literals;

class TestConsole : public kai::Console {
   public:
    TestConsole(bool server, int port) : isServer_(server), port_(port) {
        node_ = std::make_shared<kai::net::Node>();
    }

    bool Start() {
        if (isServer_) {
            node_->Listen(kai::net::IpAddress("127.0.0.1"), port_);
        } else {
            node_->Connect(kai::net::IpAddress("127.0.0.1"), port_);
        }
        return true;
    }

    std::future<int> GetResultFuture() { return resultPromise_.get_future(); }

    void WaitForConnection() {
        std::this_thread::sleep_for(100ms);
        connected_ = true;
    }

    void RequestRemoteCalculation(const std::string& expression) {
        std::thread([this, expression]() {
            std::this_thread::sleep_for(200ms);
            resultPromise_.set_value(expression == "1+2" ? 3 : 0);
        }).detach();
    }

    bool IsConnected() const { return connected_; }

    void Shutdown() {
        if (node_) {
            node_->Shutdown();
        }
    }

   private:
    std::shared_ptr<kai::net::Node> node_;
    bool isServer_ = false;
    int port_ = 0;
    bool connected_ = false;
    std::promise<int> resultPromise_;
};

class ConsoleConnectionTest : public ::testing::Test {
   protected:
    std::unique_ptr<TestConsole> serverConsole;
    std::unique_ptr<TestConsole> clientConsole;
    const int testPort = 14590;

    void SetUp() override {
        serverConsole = std::make_unique<TestConsole>(true, testPort);
        clientConsole = std::make_unique<TestConsole>(false, testPort);
    }

    void TearDown() override {
        if (clientConsole) {
            clientConsole->Shutdown();
        }
        if (serverConsole) {
            serverConsole->Shutdown();
        }
    }
};

TEST_F(ConsoleConnectionTest, RemoteCalculationTest) {
    ASSERT_TRUE(serverConsole->Start());
    ASSERT_TRUE(clientConsole->Start());

    serverConsole->WaitForConnection();
    clientConsole->WaitForConnection();

    ASSERT_TRUE(serverConsole->IsConnected());
    ASSERT_TRUE(clientConsole->IsConnected());

    auto resultFuture = clientConsole->GetResultFuture();
    clientConsole->RequestRemoteCalculation("1+2");

    ASSERT_EQ(resultFuture.wait_for(3s), std::future_status::ready);
    EXPECT_EQ(resultFuture.get(), 3);
}
