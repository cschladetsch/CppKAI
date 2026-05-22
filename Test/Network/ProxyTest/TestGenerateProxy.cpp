#include <KAI/Language/Tau/Generate/GenerateAgent.h>
#include <KAI/Language/Tau/Generate/GenerateProxy.h>
#include <gtest/gtest.h>

#include <fstream>
#include <iostream>
#include <string>

using namespace kai::tau::Generate;

// Write a minimal Tau interface file to disk for generation testing.
static void WriteTestTauFile(const std::string& path) {
    std::ofstream file(path);
    ASSERT_TRUE(file.is_open()) << "Failed to create: " << path;
    file << "namespace test {\n\n"
         << "class Calculator {\n"
         << "    int Add(int a, int b);\n"
         << "    int Subtract(int a, int b);\n"
         << "}\n\n"
         << "class DataService {\n"
         << "    void StoreData(string key, string value);\n"
         << "    string GetData(string key);\n"
         << "}\n\n"
         << "} // namespace test\n";
}

TEST(NetworkProxyTest, GeneratesValidProxyFromTauFile) {
    const std::string tauFile = "TestInterface.tau";
    const std::string outFile = "GeneratedProxy.h";

    WriteTestTauFile(tauFile);

    std::string proxyOut, agentOut, error;
    ASSERT_TRUE(
        GenerateProxy::GenerateFromFile(tauFile.c_str(), proxyOut, error))
        << "Proxy generation failed: " << error;
    ASSERT_TRUE(
        GenerateAgent::GenerateFromFile(tauFile.c_str(), agentOut, error))
        << "Agent generation failed: " << error;

    const std::string combined = proxyOut + agentOut;

    std::ofstream out(outFile);
    ASSERT_TRUE(out.is_open()) << "Failed to write: " << outFile;
    out << combined;
    out.close();

    EXPECT_NE(combined.find("CalculatorProxy"), std::string::npos);
    EXPECT_NE(combined.find("CalculatorAgent"), std::string::npos);
    EXPECT_NE(combined.find("DataServiceProxy"), std::string::npos);
    EXPECT_NE(combined.find("DataServiceAgent"), std::string::npos);
    EXPECT_NE(combined.find("SendWithResponseAsync"), std::string::npos);
    EXPECT_NE(combined.find("SendAsync"), std::string::npos);
    EXPECT_NE(combined.find("SendWithResponse"), std::string::npos);
    EXPECT_NE(combined.find("Handle_Add"), std::string::npos);
    EXPECT_NE(combined.find("AgentBase<Calculator>"), std::string::npos);

    std::remove(tauFile.c_str());
    std::remove(outFile.c_str());
}
