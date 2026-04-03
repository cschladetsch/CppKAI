#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <fstream>
#include <future>
#include <memory>
#include <string>
#include <thread>

// For process handling
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// For JSON parsing
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace pt = boost::property_tree;
using namespace std::chrono_literals;

// Helper function to execute a command and capture output
std::string ExecuteCommand(const std::string& command) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"),
                                                  pclose);

    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result;
}

class CalculationTest : public ::testing::Test {
   protected:
    std::string projectRoot;
    std::string buildDir;
    pid_t serverPid = -1;

    void SetUp() override {
        // Find the project root directory
        char buffer[PATH_MAX];
        if (getcwd(buffer, sizeof(buffer)) != nullptr) {
            projectRoot = buffer;

            // This assumes the test is run from the build directory
            // If run from elsewhere, adjust as needed
            if (projectRoot.find("/build") != std::string::npos) {
                projectRoot = projectRoot.substr(0, projectRoot.find("/build"));
            }

            buildDir = projectRoot + "/build";
        } else {
            throw std::runtime_error("Failed to get current directory");
        }

        // Create test config files
        CreateConfigFiles();

        // Build the necessary executables
        BuildExecutables();

        // Start the server
        StartServer();
    }

    void TearDown() override {
        // Stop the server if running
        if (serverPid > 0) {
            kill(serverPid, SIGTERM);
            int status;
            waitpid(serverPid, &status, 0);
        }

        // Clean up config files
        remove((projectRoot + "/test_server_config.json").c_str());
        remove((projectRoot + "/test_client_config.json").c_str());
    }

    void CreateConfigFiles() {
        // Create server config
        pt::ptree serverConfig;
        serverConfig.put("port", 14596);
        serverConfig.put("maxClients", 4);
        serverConfig.put("enableCalculation", true);
        serverConfig.put("echoMode", true);

        pt::write_json(projectRoot + "/test_server_config.json", serverConfig);

        // Create client config
        pt::ptree clientConfig;
        clientConfig.put("serverIp", "127.0.0.1");
        clientConfig.put("serverPort", 14596);
        clientConfig.put("autoCalculate", true);
        clientConfig.put("calculationExpression", "1+2");
        clientConfig.put("waitForResult", true);

        pt::write_json(projectRoot + "/test_client_config.json", clientConfig);
    }

    void BuildExecutables() {
        // Build the configurable server and client
        std::string buildCommand = "cd " + buildDir +
                                   " && cmake --build . --target "
                                   "ConfigurableServer ConfigurableClient";

        int buildResult = system(buildCommand.c_str());
        if (buildResult != 0) {
            throw std::runtime_error("Failed to build executables");
        }

        // Verify the executables exist
        if (!std::ifstream(buildDir + "/Bin/ConfigurableServer").good() ||
            !std::ifstream(buildDir + "/Bin/ConfigurableClient").good()) {
            throw std::runtime_error("Executables not found after build");
        }
    }

    void StartServer() {
        serverPid = fork();
        if (serverPid == 0) {
            // Child process - server
            // Redirect stdout/stderr to a file
            freopen("/tmp/kai_test_server.log", "w", stdout);
            freopen("/tmp/kai_test_server.log", "w", stderr);

            // Execute the server
            std::string serverPath = buildDir + "/Bin/ConfigurableServer";
            std::string configPath = projectRoot + "/test_server_config.json";
            execl(serverPath.c_str(), serverPath.c_str(), configPath.c_str(),
                  nullptr);

            // If execl returns, there was an error
            perror("execl failed for server");
            exit(1);
        } else if (serverPid < 0) {
            throw std::runtime_error("Failed to fork server process");
        }

        // Give the server time to start
        std::this_thread::sleep_for(2s);
    }

    // Helper method to run a client and get its exit code
    int RunClient(const std::string& expression) {
        // Update client config with the specified expression
        pt::ptree clientConfig;
        clientConfig.put("serverIp", "127.0.0.1");
        clientConfig.put("serverPort", 14596);
        clientConfig.put("autoCalculate", true);
        clientConfig.put("calculationExpression", expression);
        clientConfig.put("waitForResult", true);

        pt::write_json(projectRoot + "/test_client_config.json", clientConfig);

        // Fork a process to run the client
        pid_t clientPid = fork();
        if (clientPid == 0) {
            // Child process - client
            // Redirect stdout/stderr to a file
            freopen("/tmp/kai_test_client.log", "w", stdout);
            freopen("/tmp/kai_test_client.log", "w", stderr);

            // Execute the client
            std::string clientPath = buildDir + "/Bin/ConfigurableClient";
            std::string configPath = projectRoot + "/test_client_config.json";
            execl(clientPath.c_str(), clientPath.c_str(), configPath.c_str(),
                  nullptr);

            // If execl returns, there was an error
            perror("execl failed for client");
            exit(1);
        } else if (clientPid < 0) {
            throw std::runtime_error("Failed to fork client process");
        }

        // Wait for the client to complete
        int status;
        waitpid(clientPid, &status, 0);

        // Return the client exit code
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }

        return -1;
    }
};

// Test case: 1+2 should calculate to 3
TEST_F(CalculationTest, BasicAddition) {
    // Run client with expression "1+2"
    int exitCode = RunClient("1+2");

    // A successful calculation (result 3) should return exit code 0
    ASSERT_EQ(exitCode, 0)
        << "Client did not return success for calculation 1+2=3";

    // Print the client log for debugging
    std::ifstream clientLog("/tmp/kai_test_client.log");
    std::string line;
    std::cout << "Client log:" << std::endl;
    while (std::getline(clientLog, line)) {
        std::cout << line << std::endl;
    }
}

// Test case: Invalid expression should not return success
TEST_F(CalculationTest, InvalidExpression) {
    // Run client with an invalid expression "1*2"
    // Our simple calculator doesn't support multiplication
    int exitCode = RunClient("1*2");

    // An invalid calculation should not return exit code 0
    ASSERT_NE(exitCode, 0)
        << "Client incorrectly returned success for unsupported calculation";
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}