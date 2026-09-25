// Parameterized coverage for Tau IDL -> Proxy/Agent codegen across many
// interface shapes (property counts, namespace nesting). Pure parsing and
// code generation — no sockets, no runtime instantiation of the generated
// code — so this is safe, fast coverage of GenerateProxy/GenerateAgent.

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "KAI/Language/Tau/Generate/GenerateAgent.h"
#include "KAI/Language/Tau/Generate/GenerateProxy.h"

using namespace kai;

namespace {

struct TauCase {
    std::string tau;
    std::string interfaceName;
    std::string label;
};

std::string BuildTauSource(const std::string &ns, const std::string &iface,
                           int propertyCount) {
    std::string src = "namespace " + ns + " {\n    interface " + iface +
                      " {\n";
    static const char *names[] = {"Alpha", "Beta", "Gamma", "Delta", "Epsilon"};
    for (int i = 0; i < propertyCount; ++i) {
        src += "        int ";
        src += names[i % 5];
        src += std::to_string(i);
        src += ";\n";
    }
    src += "    }\n}\n";
    return src;
}

std::vector<TauCase> BuildCases() {
    std::vector<TauCase> cases;
    for (int i = 0; i < 55; ++i) {
        std::string ns = "GenNs" + std::to_string(i);
        std::string iface = "IGenerated" + std::to_string(i);
        int propertyCount = 1 + (i % 4);  // 1..4 properties, cycling
        cases.push_back({BuildTauSource(ns, iface, propertyCount), iface,
                         "Case" + std::to_string(i)});
    }
    return cases;
}

}  // namespace

class TauCodeGenParamTest : public ::testing::TestWithParam<TauCase> {};

TEST_P(TauCodeGenParamTest, ProxyGenerationSucceedsAndContainsInterfaceName) {
    const TauCase &tc = GetParam();
    std::string proxyOut;
    tau::Generate::GenerateProxy proxyGen(tc.tau.c_str(), proxyOut);
    ASSERT_FALSE(proxyGen.failed)
        << "Proxy generation failed for " << tc.label << ": " << proxyGen.error;
    EXPECT_NE(proxyOut.find(tc.interfaceName + "Proxy"), std::string::npos)
        << "Generated proxy for " << tc.label
        << " should contain " << tc.interfaceName << "Proxy:\n"
        << proxyOut;
}

TEST_P(TauCodeGenParamTest, AgentGenerationSucceedsAndContainsInterfaceName) {
    const TauCase &tc = GetParam();
    std::string agentOut;
    tau::Generate::GenerateAgent agentGen(tc.tau.c_str(), agentOut);
    ASSERT_FALSE(agentGen.failed)
        << "Agent generation failed for " << tc.label << ": " << agentGen.error;
    EXPECT_NE(agentOut.find(tc.interfaceName + "Agent"), std::string::npos)
        << "Generated agent for " << tc.label
        << " should contain " << tc.interfaceName << "Agent:\n"
        << agentOut;
}

INSTANTIATE_TEST_SUITE_P(
    GeneratedInterfaces, TauCodeGenParamTest, ::testing::ValuesIn(BuildCases()),
    [](const ::testing::TestParamInfo<TauCase> &info) {
        return info.param.label;
    });

// A few explicit edge cases beyond the generated sweep.

TEST(TauCodeGenEdgeCases, SinglePropertyInterfaceGeneratesCleanly) {
    static const std::string kTau = R"(
        namespace EdgeNs {
            interface IEdgeSingle {
                int OnlyValue;
            }
        }
    )";
    std::string proxyOut;
    tau::Generate::GenerateProxy proxyGen(kTau.c_str(), proxyOut);
    ASSERT_FALSE(proxyGen.failed) << proxyGen.error;
    EXPECT_NE(proxyOut.find("IEdgeSingleProxy"), std::string::npos);
}

TEST(TauCodeGenEdgeCases, MultiplePropertiesInterfaceGeneratesCleanly) {
    static const std::string kTau = R"(
        namespace EdgeNs {
            interface IEdgeMulti {
                int First;
                int Second;
                int Third;
                int Fourth;
                int Fifth;
            }
        }
    )";
    std::string agentOut;
    tau::Generate::GenerateAgent agentGen(kTau.c_str(), agentOut);
    ASSERT_FALSE(agentGen.failed) << agentGen.error;
    EXPECT_NE(agentOut.find("IEdgeMultiAgent"), std::string::npos);
}
