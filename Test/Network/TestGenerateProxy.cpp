#include <KAI/Network/Network.h>
#include <gtest/gtest.h>

#include <fstream>
#include <iostream>
#include <regex>
#include <string>

// Forward declaration for the function we'll test
namespace kai {
namespace net {
void GenerateNetworkProxy();
}
}  // namespace kai

// Helper function to create a test Tau interface file
void CreateTestInterfaceFile(const std::string& path) {
    std::ofstream file(path);
    ASSERT_TRUE(file.is_open()) << "Failed to create test interface file";

    file << "// TestInterface.tau - Test interface for proxy generation\n\n";
    file << "namespace test {\n\n";
    file << "class Calculator {\n";
    file << "    int Add(int a, int b);\n";
    file << "    int Subtract(int a, int b);\n";
    file << "    float Multiply(float a, float b);\n";
    file << "    float Divide(float a, float b);\n";
    file << "};\n\n";
    file << "class DataService {\n";
    file << "    void StoreData(string key, string value);\n";
    file << "    string GetData(string key);\n";
    file << "    bool HasData(string key);\n";
    file << "    void DeleteData(string key);\n";
    file << "};\n\n";
    file << "} // namespace test\n";

    file.close();
}

// Function to verify the generated proxy code
bool VerifyGeneratedCode(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file) {
        std::cerr << "Failed to open generated file: " << filePath << std::endl;
        return false;
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();

    // Define patterns to check for key components
    std::vector<std::regex> patterns = {
        // Check for Calculator proxy class
        std::regex("class CalculatorProxy"),
        // Check for Calculator agent class
        std::regex("class CalculatorAgent"),
        // Check for DataService proxy class
        std::regex("class DataServiceProxy"),
        // Check for DataService agent class
        std::regex("class DataServiceAgent"),
        // Check for Add method
        std::regex("int Add\\(const int& a, const int& b\\)"),
        // Check for GetData method
        std::regex("string GetData\\(const string& key\\)"),
        // Check for proxy method implementation
        std::regex("auto future = _node->SendWithResponse"),
        // Check for agent method implementation
        std::regex("void Handle_\\w+\\(RakNet::BitStream& bs, "
                   "RakNet::SystemAddress& sender\\)")};

    // Verify each pattern exists in the generated code
    for (const auto& pattern : patterns) {
        if (!std::regex_search(content, pattern)) {
            std::cerr << "Generated code verification failed: missing expected "
                         "pattern"
                      << std::endl;
            return false;
        }
    }

    return true;
}

// Test suite for GenerateNetworkProxy
TEST(NetworkProxyTest, GeneratesValidProxyFromTauFile) {
    // Create test files in the current directory
    std::string testFilePath = "TestInterface.tau";
    std::string outputFilePath = "GeneratedProxy.h";

    // Create the test Tau interface file
    CreateTestInterfaceFile(testFilePath);

    // Call the function being tested
    std::cout << "Calling GenerateNetworkProxy..." << std::endl;
    kai::net::GenerateNetworkProxy();

    // Verify the generated file exists and contains expected patterns
    ASSERT_TRUE(VerifyGeneratedCode(outputFilePath))
        << "Generated proxy code failed verification";
}

// Clean up test files after the test
class NetworkProxyEnvironment : public ::testing::Environment {
   public:
    ~NetworkProxyEnvironment() override {}

    void TearDown() override {
        // Clean up test files
        std::remove("TestInterface.tau");
        std::remove("GeneratedProxy.h");
    }
};

// Register the environment for cleanup
::testing::Environment* const env =
    ::testing::AddGlobalTestEnvironment(new NetworkProxyEnvironment);