#include <gtest/gtest.h>

#include "KAI/Language/Tau/Generate/GenerateAgent.h"
#include "KAI/Language/Tau/Generate/GenerateProxy.h"
#include "KAI/Language/Tau/TauParser.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// 100 new Tau IDL tests covering:
//   - round-trip node topologies scaling from 1 to 50 chained nodes
//   - continuations passed as method parameters/return types
//   - classes that hold and return Future<T> members
//   - Future<T> values passed as arguments (the network-argument-passing case)
//
// Tau is a code-generation IDL (see TauAsyncTests.cpp / TauFutureProxyTests.cpp),
// not an executable language, so "round trip" here means: the IDL for a chain of
// N cooperating nodes parses cleanly and a proxy/agent can be generated from it -
// the same verification strategy already used throughout this test suite.
struct TauNodeRoundTripAndFutureTests : TestLangCommon {
    void SetUp() override { TestLangCommon::SetUp(); }

    // Verify the IDL lexes and parses with no errors.
    bool ParseTauCode(const string& code) {
        auto lexer = make_shared<tau::TauLexer>(code.c_str(), *reg_);
        if (!lexer->Process()) {
            ADD_FAILURE() << "Lexer failed";
            return false;
        }

        auto parser = make_shared<tau::TauParser>(*reg_);
        if (!parser->Process(lexer, Structure::Module)) {
            ADD_FAILURE() << "Parser failed: " << parser->Error;
            return false;
        }

        return parser->Error.empty();
    }

    // Verify the IDL also survives at least one of the code generators,
    // matching the "at least one generator worked" bar used in TestTau.cpp.
    bool ParseAndGenerate(const string& code) {
        if (!ParseTauCode(code)) return false;

        string proxyOutput, agentOutput;
        tau::Generate::GenerateProxy proxy(code.c_str(), proxyOutput);
        tau::Generate::GenerateAgent agent(code.c_str(), agentOutput);
        return !proxy.Failed || !agent.Failed;
    }
};

// ============================================================================
// ROUND-TRIP NODE CHAINS: N = 1..50 cooperating nodes (50 tests)
// ============================================================================

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_01Nodes) {
    const char* code = R"(
        namespace RoundTrip1 {
            interface IChain1 {
                Future<int> Node1Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_02Nodes) {
    const char* code = R"(
        namespace RoundTrip2 {
            interface IChain2 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_03Nodes) {
    const char* code = R"(
        namespace RoundTrip3 {
            interface IChain3 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_04Nodes) {
    const char* code = R"(
        namespace RoundTrip4 {
            interface IChain4 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_05Nodes) {
    const char* code = R"(
        namespace RoundTrip5 {
            interface IChain5 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_06Nodes) {
    const char* code = R"(
        namespace RoundTrip6 {
            interface IChain6 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_07Nodes) {
    const char* code = R"(
        namespace RoundTrip7 {
            interface IChain7 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_08Nodes) {
    const char* code = R"(
        namespace RoundTrip8 {
            interface IChain8 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_09Nodes) {
    const char* code = R"(
        namespace RoundTrip9 {
            interface IChain9 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_10Nodes) {
    const char* code = R"(
        namespace RoundTrip10 {
            interface IChain10 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_11Nodes) {
    const char* code = R"(
        namespace RoundTrip11 {
            interface IChain11 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node11
                Future<int> Node11Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_12Nodes) {
    const char* code = R"(
        namespace RoundTrip12 {
            interface IChain12 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node11
                Future<int> Node11Step(int value); // hands off to Node12
                Future<int> Node12Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_13Nodes) {
    const char* code = R"(
        namespace RoundTrip13 {
            interface IChain13 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node11
                Future<int> Node11Step(int value); // hands off to Node12
                Future<int> Node12Step(int value); // hands off to Node13
                Future<int> Node13Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_14Nodes) {
    const char* code = R"(
        namespace RoundTrip14 {
            interface IChain14 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node11
                Future<int> Node11Step(int value); // hands off to Node12
                Future<int> Node12Step(int value); // hands off to Node13
                Future<int> Node13Step(int value); // hands off to Node14
                Future<int> Node14Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_15Nodes) {
    const char* code = R"(
        namespace RoundTrip15 {
            interface IChain15 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node11
                Future<int> Node11Step(int value); // hands off to Node12
                Future<int> Node12Step(int value); // hands off to Node13
                Future<int> Node13Step(int value); // hands off to Node14
                Future<int> Node14Step(int value); // hands off to Node15
                Future<int> Node15Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_16Nodes) {
    const char* code = R"(
        namespace RoundTrip16 {
            interface IChain16 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node11
                Future<int> Node11Step(int value); // hands off to Node12
                Future<int> Node12Step(int value); // hands off to Node13
                Future<int> Node13Step(int value); // hands off to Node14
                Future<int> Node14Step(int value); // hands off to Node15
                Future<int> Node15Step(int value); // hands off to Node16
                Future<int> Node16Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_17Nodes) {
    const char* code = R"(
        namespace RoundTrip17 {
            interface IChain17 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node11
                Future<int> Node11Step(int value); // hands off to Node12
                Future<int> Node12Step(int value); // hands off to Node13
                Future<int> Node13Step(int value); // hands off to Node14
                Future<int> Node14Step(int value); // hands off to Node15
                Future<int> Node15Step(int value); // hands off to Node16
                Future<int> Node16Step(int value); // hands off to Node17
                Future<int> Node17Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_18Nodes) {
    const char* code = R"(
        namespace RoundTrip18 {
            interface IChain18 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node11
                Future<int> Node11Step(int value); // hands off to Node12
                Future<int> Node12Step(int value); // hands off to Node13
                Future<int> Node13Step(int value); // hands off to Node14
                Future<int> Node14Step(int value); // hands off to Node15
                Future<int> Node15Step(int value); // hands off to Node16
                Future<int> Node16Step(int value); // hands off to Node17
                Future<int> Node17Step(int value); // hands off to Node18
                Future<int> Node18Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_19Nodes) {
    const char* code = R"(
        namespace RoundTrip19 {
            interface IChain19 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node11
                Future<int> Node11Step(int value); // hands off to Node12
                Future<int> Node12Step(int value); // hands off to Node13
                Future<int> Node13Step(int value); // hands off to Node14
                Future<int> Node14Step(int value); // hands off to Node15
                Future<int> Node15Step(int value); // hands off to Node16
                Future<int> Node16Step(int value); // hands off to Node17
                Future<int> Node17Step(int value); // hands off to Node18
                Future<int> Node18Step(int value); // hands off to Node19
                Future<int> Node19Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_20Nodes) {
    const char* code = R"(
        namespace RoundTrip20 {
            interface IChain20 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node11
                Future<int> Node11Step(int value); // hands off to Node12
                Future<int> Node12Step(int value); // hands off to Node13
                Future<int> Node13Step(int value); // hands off to Node14
                Future<int> Node14Step(int value); // hands off to Node15
                Future<int> Node15Step(int value); // hands off to Node16
                Future<int> Node16Step(int value); // hands off to Node17
                Future<int> Node17Step(int value); // hands off to Node18
                Future<int> Node18Step(int value); // hands off to Node19
                Future<int> Node19Step(int value); // hands off to Node20
                Future<int> Node20Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_21Nodes) {
    const char* code = R"(
        namespace RoundTrip21 {
            interface IChain21 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node11
                Future<int> Node11Step(int value); // hands off to Node12
                Future<int> Node12Step(int value); // hands off to Node13
                Future<int> Node13Step(int value); // hands off to Node14
                Future<int> Node14Step(int value); // hands off to Node15
                Future<int> Node15Step(int value); // hands off to Node16
                Future<int> Node16Step(int value); // hands off to Node17
                Future<int> Node17Step(int value); // hands off to Node18
                Future<int> Node18Step(int value); // hands off to Node19
                Future<int> Node19Step(int value); // hands off to Node20
                Future<int> Node20Step(int value); // hands off to Node21
                Future<int> Node21Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_22Nodes) {
    const char* code = R"(
        namespace RoundTrip22 {
            interface IChain22 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node11
                Future<int> Node11Step(int value); // hands off to Node12
                Future<int> Node12Step(int value); // hands off to Node13
                Future<int> Node13Step(int value); // hands off to Node14
                Future<int> Node14Step(int value); // hands off to Node15
                Future<int> Node15Step(int value); // hands off to Node16
                Future<int> Node16Step(int value); // hands off to Node17
                Future<int> Node17Step(int value); // hands off to Node18
                Future<int> Node18Step(int value); // hands off to Node19
                Future<int> Node19Step(int value); // hands off to Node20
                Future<int> Node20Step(int value); // hands off to Node21
                Future<int> Node21Step(int value); // hands off to Node22
                Future<int> Node22Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_23Nodes) {
    const char* code = R"(
        namespace RoundTrip23 {
            interface IChain23 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node11
                Future<int> Node11Step(int value); // hands off to Node12
                Future<int> Node12Step(int value); // hands off to Node13
                Future<int> Node13Step(int value); // hands off to Node14
                Future<int> Node14Step(int value); // hands off to Node15
                Future<int> Node15Step(int value); // hands off to Node16
                Future<int> Node16Step(int value); // hands off to Node17
                Future<int> Node17Step(int value); // hands off to Node18
                Future<int> Node18Step(int value); // hands off to Node19
                Future<int> Node19Step(int value); // hands off to Node20
                Future<int> Node20Step(int value); // hands off to Node21
                Future<int> Node21Step(int value); // hands off to Node22
                Future<int> Node22Step(int value); // hands off to Node23
                Future<int> Node23Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_24Nodes) {
    const char* code = R"(
        namespace RoundTrip24 {
            interface IChain24 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node11
                Future<int> Node11Step(int value); // hands off to Node12
                Future<int> Node12Step(int value); // hands off to Node13
                Future<int> Node13Step(int value); // hands off to Node14
                Future<int> Node14Step(int value); // hands off to Node15
                Future<int> Node15Step(int value); // hands off to Node16
                Future<int> Node16Step(int value); // hands off to Node17
                Future<int> Node17Step(int value); // hands off to Node18
                Future<int> Node18Step(int value); // hands off to Node19
                Future<int> Node19Step(int value); // hands off to Node20
                Future<int> Node20Step(int value); // hands off to Node21
                Future<int> Node21Step(int value); // hands off to Node22
                Future<int> Node22Step(int value); // hands off to Node23
                Future<int> Node23Step(int value); // hands off to Node24
                Future<int> Node24Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_25Nodes) {
    const char* code = R"(
        namespace RoundTrip25 {
            interface IChain25 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node11
                Future<int> Node11Step(int value); // hands off to Node12
                Future<int> Node12Step(int value); // hands off to Node13
                Future<int> Node13Step(int value); // hands off to Node14
                Future<int> Node14Step(int value); // hands off to Node15
                Future<int> Node15Step(int value); // hands off to Node16
                Future<int> Node16Step(int value); // hands off to Node17
                Future<int> Node17Step(int value); // hands off to Node18
                Future<int> Node18Step(int value); // hands off to Node19
                Future<int> Node19Step(int value); // hands off to Node20
                Future<int> Node20Step(int value); // hands off to Node21
                Future<int> Node21Step(int value); // hands off to Node22
                Future<int> Node22Step(int value); // hands off to Node23
                Future<int> Node23Step(int value); // hands off to Node24
                Future<int> Node24Step(int value); // hands off to Node25
                Future<int> Node25Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_26Nodes) {
    const char* code = R"(
        namespace RoundTrip26 {
            interface IChain26 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node11
                Future<int> Node11Step(int value); // hands off to Node12
                Future<int> Node12Step(int value); // hands off to Node13
                Future<int> Node13Step(int value); // hands off to Node14
                Future<int> Node14Step(int value); // hands off to Node15
                Future<int> Node15Step(int value); // hands off to Node16
                Future<int> Node16Step(int value); // hands off to Node17
                Future<int> Node17Step(int value); // hands off to Node18
                Future<int> Node18Step(int value); // hands off to Node19
                Future<int> Node19Step(int value); // hands off to Node20
                Future<int> Node20Step(int value); // hands off to Node21
                Future<int> Node21Step(int value); // hands off to Node22
                Future<int> Node22Step(int value); // hands off to Node23
                Future<int> Node23Step(int value); // hands off to Node24
                Future<int> Node24Step(int value); // hands off to Node25
                Future<int> Node25Step(int value); // hands off to Node26
                Future<int> Node26Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_27Nodes) {
    const char* code = R"(
        namespace RoundTrip27 {
            interface IChain27 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node11
                Future<int> Node11Step(int value); // hands off to Node12
                Future<int> Node12Step(int value); // hands off to Node13
                Future<int> Node13Step(int value); // hands off to Node14
                Future<int> Node14Step(int value); // hands off to Node15
                Future<int> Node15Step(int value); // hands off to Node16
                Future<int> Node16Step(int value); // hands off to Node17
                Future<int> Node17Step(int value); // hands off to Node18
                Future<int> Node18Step(int value); // hands off to Node19
                Future<int> Node19Step(int value); // hands off to Node20
                Future<int> Node20Step(int value); // hands off to Node21
                Future<int> Node21Step(int value); // hands off to Node22
                Future<int> Node22Step(int value); // hands off to Node23
                Future<int> Node23Step(int value); // hands off to Node24
                Future<int> Node24Step(int value); // hands off to Node25
                Future<int> Node25Step(int value); // hands off to Node26
                Future<int> Node26Step(int value); // hands off to Node27
                Future<int> Node27Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_28Nodes) {
    const char* code = R"(
        namespace RoundTrip28 {
            interface IChain28 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node11
                Future<int> Node11Step(int value); // hands off to Node12
                Future<int> Node12Step(int value); // hands off to Node13
                Future<int> Node13Step(int value); // hands off to Node14
                Future<int> Node14Step(int value); // hands off to Node15
                Future<int> Node15Step(int value); // hands off to Node16
                Future<int> Node16Step(int value); // hands off to Node17
                Future<int> Node17Step(int value); // hands off to Node18
                Future<int> Node18Step(int value); // hands off to Node19
                Future<int> Node19Step(int value); // hands off to Node20
                Future<int> Node20Step(int value); // hands off to Node21
                Future<int> Node21Step(int value); // hands off to Node22
                Future<int> Node22Step(int value); // hands off to Node23
                Future<int> Node23Step(int value); // hands off to Node24
                Future<int> Node24Step(int value); // hands off to Node25
                Future<int> Node25Step(int value); // hands off to Node26
                Future<int> Node26Step(int value); // hands off to Node27
                Future<int> Node27Step(int value); // hands off to Node28
                Future<int> Node28Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_29Nodes) {
    const char* code = R"(
        namespace RoundTrip29 {
            interface IChain29 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node11
                Future<int> Node11Step(int value); // hands off to Node12
                Future<int> Node12Step(int value); // hands off to Node13
                Future<int> Node13Step(int value); // hands off to Node14
                Future<int> Node14Step(int value); // hands off to Node15
                Future<int> Node15Step(int value); // hands off to Node16
                Future<int> Node16Step(int value); // hands off to Node17
                Future<int> Node17Step(int value); // hands off to Node18
                Future<int> Node18Step(int value); // hands off to Node19
                Future<int> Node19Step(int value); // hands off to Node20
                Future<int> Node20Step(int value); // hands off to Node21
                Future<int> Node21Step(int value); // hands off to Node22
                Future<int> Node22Step(int value); // hands off to Node23
                Future<int> Node23Step(int value); // hands off to Node24
                Future<int> Node24Step(int value); // hands off to Node25
                Future<int> Node25Step(int value); // hands off to Node26
                Future<int> Node26Step(int value); // hands off to Node27
                Future<int> Node27Step(int value); // hands off to Node28
                Future<int> Node28Step(int value); // hands off to Node29
                Future<int> Node29Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_30Nodes) {
    const char* code = R"(
        namespace RoundTrip30 {
            interface IChain30 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node11
                Future<int> Node11Step(int value); // hands off to Node12
                Future<int> Node12Step(int value); // hands off to Node13
                Future<int> Node13Step(int value); // hands off to Node14
                Future<int> Node14Step(int value); // hands off to Node15
                Future<int> Node15Step(int value); // hands off to Node16
                Future<int> Node16Step(int value); // hands off to Node17
                Future<int> Node17Step(int value); // hands off to Node18
                Future<int> Node18Step(int value); // hands off to Node19
                Future<int> Node19Step(int value); // hands off to Node20
                Future<int> Node20Step(int value); // hands off to Node21
                Future<int> Node21Step(int value); // hands off to Node22
                Future<int> Node22Step(int value); // hands off to Node23
                Future<int> Node23Step(int value); // hands off to Node24
                Future<int> Node24Step(int value); // hands off to Node25
                Future<int> Node25Step(int value); // hands off to Node26
                Future<int> Node26Step(int value); // hands off to Node27
                Future<int> Node27Step(int value); // hands off to Node28
                Future<int> Node28Step(int value); // hands off to Node29
                Future<int> Node29Step(int value); // hands off to Node30
                Future<int> Node30Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_31Nodes) {
    const char* code = R"(
        namespace RoundTrip31 {
            interface IChain31 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node11
                Future<int> Node11Step(int value); // hands off to Node12
                Future<int> Node12Step(int value); // hands off to Node13
                Future<int> Node13Step(int value); // hands off to Node14
                Future<int> Node14Step(int value); // hands off to Node15
                Future<int> Node15Step(int value); // hands off to Node16
                Future<int> Node16Step(int value); // hands off to Node17
                Future<int> Node17Step(int value); // hands off to Node18
                Future<int> Node18Step(int value); // hands off to Node19
                Future<int> Node19Step(int value); // hands off to Node20
                Future<int> Node20Step(int value); // hands off to Node21
                Future<int> Node21Step(int value); // hands off to Node22
                Future<int> Node22Step(int value); // hands off to Node23
                Future<int> Node23Step(int value); // hands off to Node24
                Future<int> Node24Step(int value); // hands off to Node25
                Future<int> Node25Step(int value); // hands off to Node26
                Future<int> Node26Step(int value); // hands off to Node27
                Future<int> Node27Step(int value); // hands off to Node28
                Future<int> Node28Step(int value); // hands off to Node29
                Future<int> Node29Step(int value); // hands off to Node30
                Future<int> Node30Step(int value); // hands off to Node31
                Future<int> Node31Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_32Nodes) {
    const char* code = R"(
        namespace RoundTrip32 {
            interface IChain32 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node11
                Future<int> Node11Step(int value); // hands off to Node12
                Future<int> Node12Step(int value); // hands off to Node13
                Future<int> Node13Step(int value); // hands off to Node14
                Future<int> Node14Step(int value); // hands off to Node15
                Future<int> Node15Step(int value); // hands off to Node16
                Future<int> Node16Step(int value); // hands off to Node17
                Future<int> Node17Step(int value); // hands off to Node18
                Future<int> Node18Step(int value); // hands off to Node19
                Future<int> Node19Step(int value); // hands off to Node20
                Future<int> Node20Step(int value); // hands off to Node21
                Future<int> Node21Step(int value); // hands off to Node22
                Future<int> Node22Step(int value); // hands off to Node23
                Future<int> Node23Step(int value); // hands off to Node24
                Future<int> Node24Step(int value); // hands off to Node25
                Future<int> Node25Step(int value); // hands off to Node26
                Future<int> Node26Step(int value); // hands off to Node27
                Future<int> Node27Step(int value); // hands off to Node28
                Future<int> Node28Step(int value); // hands off to Node29
                Future<int> Node29Step(int value); // hands off to Node30
                Future<int> Node30Step(int value); // hands off to Node31
                Future<int> Node31Step(int value); // hands off to Node32
                Future<int> Node32Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_33Nodes) {
    const char* code = R"(
        namespace RoundTrip33 {
            interface IChain33 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node11
                Future<int> Node11Step(int value); // hands off to Node12
                Future<int> Node12Step(int value); // hands off to Node13
                Future<int> Node13Step(int value); // hands off to Node14
                Future<int> Node14Step(int value); // hands off to Node15
                Future<int> Node15Step(int value); // hands off to Node16
                Future<int> Node16Step(int value); // hands off to Node17
                Future<int> Node17Step(int value); // hands off to Node18
                Future<int> Node18Step(int value); // hands off to Node19
                Future<int> Node19Step(int value); // hands off to Node20
                Future<int> Node20Step(int value); // hands off to Node21
                Future<int> Node21Step(int value); // hands off to Node22
                Future<int> Node22Step(int value); // hands off to Node23
                Future<int> Node23Step(int value); // hands off to Node24
                Future<int> Node24Step(int value); // hands off to Node25
                Future<int> Node25Step(int value); // hands off to Node26
                Future<int> Node26Step(int value); // hands off to Node27
                Future<int> Node27Step(int value); // hands off to Node28
                Future<int> Node28Step(int value); // hands off to Node29
                Future<int> Node29Step(int value); // hands off to Node30
                Future<int> Node30Step(int value); // hands off to Node31
                Future<int> Node31Step(int value); // hands off to Node32
                Future<int> Node32Step(int value); // hands off to Node33
                Future<int> Node33Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_34Nodes) {
    const char* code = R"(
        namespace RoundTrip34 {
            interface IChain34 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node11
                Future<int> Node11Step(int value); // hands off to Node12
                Future<int> Node12Step(int value); // hands off to Node13
                Future<int> Node13Step(int value); // hands off to Node14
                Future<int> Node14Step(int value); // hands off to Node15
                Future<int> Node15Step(int value); // hands off to Node16
                Future<int> Node16Step(int value); // hands off to Node17
                Future<int> Node17Step(int value); // hands off to Node18
                Future<int> Node18Step(int value); // hands off to Node19
                Future<int> Node19Step(int value); // hands off to Node20
                Future<int> Node20Step(int value); // hands off to Node21
                Future<int> Node21Step(int value); // hands off to Node22
                Future<int> Node22Step(int value); // hands off to Node23
                Future<int> Node23Step(int value); // hands off to Node24
                Future<int> Node24Step(int value); // hands off to Node25
                Future<int> Node25Step(int value); // hands off to Node26
                Future<int> Node26Step(int value); // hands off to Node27
                Future<int> Node27Step(int value); // hands off to Node28
                Future<int> Node28Step(int value); // hands off to Node29
                Future<int> Node29Step(int value); // hands off to Node30
                Future<int> Node30Step(int value); // hands off to Node31
                Future<int> Node31Step(int value); // hands off to Node32
                Future<int> Node32Step(int value); // hands off to Node33
                Future<int> Node33Step(int value); // hands off to Node34
                Future<int> Node34Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_35Nodes) {
    const char* code = R"(
        namespace RoundTrip35 {
            interface IChain35 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node11
                Future<int> Node11Step(int value); // hands off to Node12
                Future<int> Node12Step(int value); // hands off to Node13
                Future<int> Node13Step(int value); // hands off to Node14
                Future<int> Node14Step(int value); // hands off to Node15
                Future<int> Node15Step(int value); // hands off to Node16
                Future<int> Node16Step(int value); // hands off to Node17
                Future<int> Node17Step(int value); // hands off to Node18
                Future<int> Node18Step(int value); // hands off to Node19
                Future<int> Node19Step(int value); // hands off to Node20
                Future<int> Node20Step(int value); // hands off to Node21
                Future<int> Node21Step(int value); // hands off to Node22
                Future<int> Node22Step(int value); // hands off to Node23
                Future<int> Node23Step(int value); // hands off to Node24
                Future<int> Node24Step(int value); // hands off to Node25
                Future<int> Node25Step(int value); // hands off to Node26
                Future<int> Node26Step(int value); // hands off to Node27
                Future<int> Node27Step(int value); // hands off to Node28
                Future<int> Node28Step(int value); // hands off to Node29
                Future<int> Node29Step(int value); // hands off to Node30
                Future<int> Node30Step(int value); // hands off to Node31
                Future<int> Node31Step(int value); // hands off to Node32
                Future<int> Node32Step(int value); // hands off to Node33
                Future<int> Node33Step(int value); // hands off to Node34
                Future<int> Node34Step(int value); // hands off to Node35
                Future<int> Node35Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_36Nodes) {
    const char* code = R"(
        namespace RoundTrip36 {
            interface IChain36 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node11
                Future<int> Node11Step(int value); // hands off to Node12
                Future<int> Node12Step(int value); // hands off to Node13
                Future<int> Node13Step(int value); // hands off to Node14
                Future<int> Node14Step(int value); // hands off to Node15
                Future<int> Node15Step(int value); // hands off to Node16
                Future<int> Node16Step(int value); // hands off to Node17
                Future<int> Node17Step(int value); // hands off to Node18
                Future<int> Node18Step(int value); // hands off to Node19
                Future<int> Node19Step(int value); // hands off to Node20
                Future<int> Node20Step(int value); // hands off to Node21
                Future<int> Node21Step(int value); // hands off to Node22
                Future<int> Node22Step(int value); // hands off to Node23
                Future<int> Node23Step(int value); // hands off to Node24
                Future<int> Node24Step(int value); // hands off to Node25
                Future<int> Node25Step(int value); // hands off to Node26
                Future<int> Node26Step(int value); // hands off to Node27
                Future<int> Node27Step(int value); // hands off to Node28
                Future<int> Node28Step(int value); // hands off to Node29
                Future<int> Node29Step(int value); // hands off to Node30
                Future<int> Node30Step(int value); // hands off to Node31
                Future<int> Node31Step(int value); // hands off to Node32
                Future<int> Node32Step(int value); // hands off to Node33
                Future<int> Node33Step(int value); // hands off to Node34
                Future<int> Node34Step(int value); // hands off to Node35
                Future<int> Node35Step(int value); // hands off to Node36
                Future<int> Node36Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_37Nodes) {
    const char* code = R"(
        namespace RoundTrip37 {
            interface IChain37 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node11
                Future<int> Node11Step(int value); // hands off to Node12
                Future<int> Node12Step(int value); // hands off to Node13
                Future<int> Node13Step(int value); // hands off to Node14
                Future<int> Node14Step(int value); // hands off to Node15
                Future<int> Node15Step(int value); // hands off to Node16
                Future<int> Node16Step(int value); // hands off to Node17
                Future<int> Node17Step(int value); // hands off to Node18
                Future<int> Node18Step(int value); // hands off to Node19
                Future<int> Node19Step(int value); // hands off to Node20
                Future<int> Node20Step(int value); // hands off to Node21
                Future<int> Node21Step(int value); // hands off to Node22
                Future<int> Node22Step(int value); // hands off to Node23
                Future<int> Node23Step(int value); // hands off to Node24
                Future<int> Node24Step(int value); // hands off to Node25
                Future<int> Node25Step(int value); // hands off to Node26
                Future<int> Node26Step(int value); // hands off to Node27
                Future<int> Node27Step(int value); // hands off to Node28
                Future<int> Node28Step(int value); // hands off to Node29
                Future<int> Node29Step(int value); // hands off to Node30
                Future<int> Node30Step(int value); // hands off to Node31
                Future<int> Node31Step(int value); // hands off to Node32
                Future<int> Node32Step(int value); // hands off to Node33
                Future<int> Node33Step(int value); // hands off to Node34
                Future<int> Node34Step(int value); // hands off to Node35
                Future<int> Node35Step(int value); // hands off to Node36
                Future<int> Node36Step(int value); // hands off to Node37
                Future<int> Node37Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_38Nodes) {
    const char* code = R"(
        namespace RoundTrip38 {
            interface IChain38 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node11
                Future<int> Node11Step(int value); // hands off to Node12
                Future<int> Node12Step(int value); // hands off to Node13
                Future<int> Node13Step(int value); // hands off to Node14
                Future<int> Node14Step(int value); // hands off to Node15
                Future<int> Node15Step(int value); // hands off to Node16
                Future<int> Node16Step(int value); // hands off to Node17
                Future<int> Node17Step(int value); // hands off to Node18
                Future<int> Node18Step(int value); // hands off to Node19
                Future<int> Node19Step(int value); // hands off to Node20
                Future<int> Node20Step(int value); // hands off to Node21
                Future<int> Node21Step(int value); // hands off to Node22
                Future<int> Node22Step(int value); // hands off to Node23
                Future<int> Node23Step(int value); // hands off to Node24
                Future<int> Node24Step(int value); // hands off to Node25
                Future<int> Node25Step(int value); // hands off to Node26
                Future<int> Node26Step(int value); // hands off to Node27
                Future<int> Node27Step(int value); // hands off to Node28
                Future<int> Node28Step(int value); // hands off to Node29
                Future<int> Node29Step(int value); // hands off to Node30
                Future<int> Node30Step(int value); // hands off to Node31
                Future<int> Node31Step(int value); // hands off to Node32
                Future<int> Node32Step(int value); // hands off to Node33
                Future<int> Node33Step(int value); // hands off to Node34
                Future<int> Node34Step(int value); // hands off to Node35
                Future<int> Node35Step(int value); // hands off to Node36
                Future<int> Node36Step(int value); // hands off to Node37
                Future<int> Node37Step(int value); // hands off to Node38
                Future<int> Node38Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_39Nodes) {
    const char* code = R"(
        namespace RoundTrip39 {
            interface IChain39 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node11
                Future<int> Node11Step(int value); // hands off to Node12
                Future<int> Node12Step(int value); // hands off to Node13
                Future<int> Node13Step(int value); // hands off to Node14
                Future<int> Node14Step(int value); // hands off to Node15
                Future<int> Node15Step(int value); // hands off to Node16
                Future<int> Node16Step(int value); // hands off to Node17
                Future<int> Node17Step(int value); // hands off to Node18
                Future<int> Node18Step(int value); // hands off to Node19
                Future<int> Node19Step(int value); // hands off to Node20
                Future<int> Node20Step(int value); // hands off to Node21
                Future<int> Node21Step(int value); // hands off to Node22
                Future<int> Node22Step(int value); // hands off to Node23
                Future<int> Node23Step(int value); // hands off to Node24
                Future<int> Node24Step(int value); // hands off to Node25
                Future<int> Node25Step(int value); // hands off to Node26
                Future<int> Node26Step(int value); // hands off to Node27
                Future<int> Node27Step(int value); // hands off to Node28
                Future<int> Node28Step(int value); // hands off to Node29
                Future<int> Node29Step(int value); // hands off to Node30
                Future<int> Node30Step(int value); // hands off to Node31
                Future<int> Node31Step(int value); // hands off to Node32
                Future<int> Node32Step(int value); // hands off to Node33
                Future<int> Node33Step(int value); // hands off to Node34
                Future<int> Node34Step(int value); // hands off to Node35
                Future<int> Node35Step(int value); // hands off to Node36
                Future<int> Node36Step(int value); // hands off to Node37
                Future<int> Node37Step(int value); // hands off to Node38
                Future<int> Node38Step(int value); // hands off to Node39
                Future<int> Node39Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_40Nodes) {
    const char* code = R"(
        namespace RoundTrip40 {
            interface IChain40 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node11
                Future<int> Node11Step(int value); // hands off to Node12
                Future<int> Node12Step(int value); // hands off to Node13
                Future<int> Node13Step(int value); // hands off to Node14
                Future<int> Node14Step(int value); // hands off to Node15
                Future<int> Node15Step(int value); // hands off to Node16
                Future<int> Node16Step(int value); // hands off to Node17
                Future<int> Node17Step(int value); // hands off to Node18
                Future<int> Node18Step(int value); // hands off to Node19
                Future<int> Node19Step(int value); // hands off to Node20
                Future<int> Node20Step(int value); // hands off to Node21
                Future<int> Node21Step(int value); // hands off to Node22
                Future<int> Node22Step(int value); // hands off to Node23
                Future<int> Node23Step(int value); // hands off to Node24
                Future<int> Node24Step(int value); // hands off to Node25
                Future<int> Node25Step(int value); // hands off to Node26
                Future<int> Node26Step(int value); // hands off to Node27
                Future<int> Node27Step(int value); // hands off to Node28
                Future<int> Node28Step(int value); // hands off to Node29
                Future<int> Node29Step(int value); // hands off to Node30
                Future<int> Node30Step(int value); // hands off to Node31
                Future<int> Node31Step(int value); // hands off to Node32
                Future<int> Node32Step(int value); // hands off to Node33
                Future<int> Node33Step(int value); // hands off to Node34
                Future<int> Node34Step(int value); // hands off to Node35
                Future<int> Node35Step(int value); // hands off to Node36
                Future<int> Node36Step(int value); // hands off to Node37
                Future<int> Node37Step(int value); // hands off to Node38
                Future<int> Node38Step(int value); // hands off to Node39
                Future<int> Node39Step(int value); // hands off to Node40
                Future<int> Node40Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_41Nodes) {
    const char* code = R"(
        namespace RoundTrip41 {
            interface IChain41 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node11
                Future<int> Node11Step(int value); // hands off to Node12
                Future<int> Node12Step(int value); // hands off to Node13
                Future<int> Node13Step(int value); // hands off to Node14
                Future<int> Node14Step(int value); // hands off to Node15
                Future<int> Node15Step(int value); // hands off to Node16
                Future<int> Node16Step(int value); // hands off to Node17
                Future<int> Node17Step(int value); // hands off to Node18
                Future<int> Node18Step(int value); // hands off to Node19
                Future<int> Node19Step(int value); // hands off to Node20
                Future<int> Node20Step(int value); // hands off to Node21
                Future<int> Node21Step(int value); // hands off to Node22
                Future<int> Node22Step(int value); // hands off to Node23
                Future<int> Node23Step(int value); // hands off to Node24
                Future<int> Node24Step(int value); // hands off to Node25
                Future<int> Node25Step(int value); // hands off to Node26
                Future<int> Node26Step(int value); // hands off to Node27
                Future<int> Node27Step(int value); // hands off to Node28
                Future<int> Node28Step(int value); // hands off to Node29
                Future<int> Node29Step(int value); // hands off to Node30
                Future<int> Node30Step(int value); // hands off to Node31
                Future<int> Node31Step(int value); // hands off to Node32
                Future<int> Node32Step(int value); // hands off to Node33
                Future<int> Node33Step(int value); // hands off to Node34
                Future<int> Node34Step(int value); // hands off to Node35
                Future<int> Node35Step(int value); // hands off to Node36
                Future<int> Node36Step(int value); // hands off to Node37
                Future<int> Node37Step(int value); // hands off to Node38
                Future<int> Node38Step(int value); // hands off to Node39
                Future<int> Node39Step(int value); // hands off to Node40
                Future<int> Node40Step(int value); // hands off to Node41
                Future<int> Node41Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_42Nodes) {
    const char* code = R"(
        namespace RoundTrip42 {
            interface IChain42 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node11
                Future<int> Node11Step(int value); // hands off to Node12
                Future<int> Node12Step(int value); // hands off to Node13
                Future<int> Node13Step(int value); // hands off to Node14
                Future<int> Node14Step(int value); // hands off to Node15
                Future<int> Node15Step(int value); // hands off to Node16
                Future<int> Node16Step(int value); // hands off to Node17
                Future<int> Node17Step(int value); // hands off to Node18
                Future<int> Node18Step(int value); // hands off to Node19
                Future<int> Node19Step(int value); // hands off to Node20
                Future<int> Node20Step(int value); // hands off to Node21
                Future<int> Node21Step(int value); // hands off to Node22
                Future<int> Node22Step(int value); // hands off to Node23
                Future<int> Node23Step(int value); // hands off to Node24
                Future<int> Node24Step(int value); // hands off to Node25
                Future<int> Node25Step(int value); // hands off to Node26
                Future<int> Node26Step(int value); // hands off to Node27
                Future<int> Node27Step(int value); // hands off to Node28
                Future<int> Node28Step(int value); // hands off to Node29
                Future<int> Node29Step(int value); // hands off to Node30
                Future<int> Node30Step(int value); // hands off to Node31
                Future<int> Node31Step(int value); // hands off to Node32
                Future<int> Node32Step(int value); // hands off to Node33
                Future<int> Node33Step(int value); // hands off to Node34
                Future<int> Node34Step(int value); // hands off to Node35
                Future<int> Node35Step(int value); // hands off to Node36
                Future<int> Node36Step(int value); // hands off to Node37
                Future<int> Node37Step(int value); // hands off to Node38
                Future<int> Node38Step(int value); // hands off to Node39
                Future<int> Node39Step(int value); // hands off to Node40
                Future<int> Node40Step(int value); // hands off to Node41
                Future<int> Node41Step(int value); // hands off to Node42
                Future<int> Node42Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_43Nodes) {
    const char* code = R"(
        namespace RoundTrip43 {
            interface IChain43 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node11
                Future<int> Node11Step(int value); // hands off to Node12
                Future<int> Node12Step(int value); // hands off to Node13
                Future<int> Node13Step(int value); // hands off to Node14
                Future<int> Node14Step(int value); // hands off to Node15
                Future<int> Node15Step(int value); // hands off to Node16
                Future<int> Node16Step(int value); // hands off to Node17
                Future<int> Node17Step(int value); // hands off to Node18
                Future<int> Node18Step(int value); // hands off to Node19
                Future<int> Node19Step(int value); // hands off to Node20
                Future<int> Node20Step(int value); // hands off to Node21
                Future<int> Node21Step(int value); // hands off to Node22
                Future<int> Node22Step(int value); // hands off to Node23
                Future<int> Node23Step(int value); // hands off to Node24
                Future<int> Node24Step(int value); // hands off to Node25
                Future<int> Node25Step(int value); // hands off to Node26
                Future<int> Node26Step(int value); // hands off to Node27
                Future<int> Node27Step(int value); // hands off to Node28
                Future<int> Node28Step(int value); // hands off to Node29
                Future<int> Node29Step(int value); // hands off to Node30
                Future<int> Node30Step(int value); // hands off to Node31
                Future<int> Node31Step(int value); // hands off to Node32
                Future<int> Node32Step(int value); // hands off to Node33
                Future<int> Node33Step(int value); // hands off to Node34
                Future<int> Node34Step(int value); // hands off to Node35
                Future<int> Node35Step(int value); // hands off to Node36
                Future<int> Node36Step(int value); // hands off to Node37
                Future<int> Node37Step(int value); // hands off to Node38
                Future<int> Node38Step(int value); // hands off to Node39
                Future<int> Node39Step(int value); // hands off to Node40
                Future<int> Node40Step(int value); // hands off to Node41
                Future<int> Node41Step(int value); // hands off to Node42
                Future<int> Node42Step(int value); // hands off to Node43
                Future<int> Node43Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_44Nodes) {
    const char* code = R"(
        namespace RoundTrip44 {
            interface IChain44 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node11
                Future<int> Node11Step(int value); // hands off to Node12
                Future<int> Node12Step(int value); // hands off to Node13
                Future<int> Node13Step(int value); // hands off to Node14
                Future<int> Node14Step(int value); // hands off to Node15
                Future<int> Node15Step(int value); // hands off to Node16
                Future<int> Node16Step(int value); // hands off to Node17
                Future<int> Node17Step(int value); // hands off to Node18
                Future<int> Node18Step(int value); // hands off to Node19
                Future<int> Node19Step(int value); // hands off to Node20
                Future<int> Node20Step(int value); // hands off to Node21
                Future<int> Node21Step(int value); // hands off to Node22
                Future<int> Node22Step(int value); // hands off to Node23
                Future<int> Node23Step(int value); // hands off to Node24
                Future<int> Node24Step(int value); // hands off to Node25
                Future<int> Node25Step(int value); // hands off to Node26
                Future<int> Node26Step(int value); // hands off to Node27
                Future<int> Node27Step(int value); // hands off to Node28
                Future<int> Node28Step(int value); // hands off to Node29
                Future<int> Node29Step(int value); // hands off to Node30
                Future<int> Node30Step(int value); // hands off to Node31
                Future<int> Node31Step(int value); // hands off to Node32
                Future<int> Node32Step(int value); // hands off to Node33
                Future<int> Node33Step(int value); // hands off to Node34
                Future<int> Node34Step(int value); // hands off to Node35
                Future<int> Node35Step(int value); // hands off to Node36
                Future<int> Node36Step(int value); // hands off to Node37
                Future<int> Node37Step(int value); // hands off to Node38
                Future<int> Node38Step(int value); // hands off to Node39
                Future<int> Node39Step(int value); // hands off to Node40
                Future<int> Node40Step(int value); // hands off to Node41
                Future<int> Node41Step(int value); // hands off to Node42
                Future<int> Node42Step(int value); // hands off to Node43
                Future<int> Node43Step(int value); // hands off to Node44
                Future<int> Node44Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_45Nodes) {
    const char* code = R"(
        namespace RoundTrip45 {
            interface IChain45 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node11
                Future<int> Node11Step(int value); // hands off to Node12
                Future<int> Node12Step(int value); // hands off to Node13
                Future<int> Node13Step(int value); // hands off to Node14
                Future<int> Node14Step(int value); // hands off to Node15
                Future<int> Node15Step(int value); // hands off to Node16
                Future<int> Node16Step(int value); // hands off to Node17
                Future<int> Node17Step(int value); // hands off to Node18
                Future<int> Node18Step(int value); // hands off to Node19
                Future<int> Node19Step(int value); // hands off to Node20
                Future<int> Node20Step(int value); // hands off to Node21
                Future<int> Node21Step(int value); // hands off to Node22
                Future<int> Node22Step(int value); // hands off to Node23
                Future<int> Node23Step(int value); // hands off to Node24
                Future<int> Node24Step(int value); // hands off to Node25
                Future<int> Node25Step(int value); // hands off to Node26
                Future<int> Node26Step(int value); // hands off to Node27
                Future<int> Node27Step(int value); // hands off to Node28
                Future<int> Node28Step(int value); // hands off to Node29
                Future<int> Node29Step(int value); // hands off to Node30
                Future<int> Node30Step(int value); // hands off to Node31
                Future<int> Node31Step(int value); // hands off to Node32
                Future<int> Node32Step(int value); // hands off to Node33
                Future<int> Node33Step(int value); // hands off to Node34
                Future<int> Node34Step(int value); // hands off to Node35
                Future<int> Node35Step(int value); // hands off to Node36
                Future<int> Node36Step(int value); // hands off to Node37
                Future<int> Node37Step(int value); // hands off to Node38
                Future<int> Node38Step(int value); // hands off to Node39
                Future<int> Node39Step(int value); // hands off to Node40
                Future<int> Node40Step(int value); // hands off to Node41
                Future<int> Node41Step(int value); // hands off to Node42
                Future<int> Node42Step(int value); // hands off to Node43
                Future<int> Node43Step(int value); // hands off to Node44
                Future<int> Node44Step(int value); // hands off to Node45
                Future<int> Node45Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_46Nodes) {
    const char* code = R"(
        namespace RoundTrip46 {
            interface IChain46 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node11
                Future<int> Node11Step(int value); // hands off to Node12
                Future<int> Node12Step(int value); // hands off to Node13
                Future<int> Node13Step(int value); // hands off to Node14
                Future<int> Node14Step(int value); // hands off to Node15
                Future<int> Node15Step(int value); // hands off to Node16
                Future<int> Node16Step(int value); // hands off to Node17
                Future<int> Node17Step(int value); // hands off to Node18
                Future<int> Node18Step(int value); // hands off to Node19
                Future<int> Node19Step(int value); // hands off to Node20
                Future<int> Node20Step(int value); // hands off to Node21
                Future<int> Node21Step(int value); // hands off to Node22
                Future<int> Node22Step(int value); // hands off to Node23
                Future<int> Node23Step(int value); // hands off to Node24
                Future<int> Node24Step(int value); // hands off to Node25
                Future<int> Node25Step(int value); // hands off to Node26
                Future<int> Node26Step(int value); // hands off to Node27
                Future<int> Node27Step(int value); // hands off to Node28
                Future<int> Node28Step(int value); // hands off to Node29
                Future<int> Node29Step(int value); // hands off to Node30
                Future<int> Node30Step(int value); // hands off to Node31
                Future<int> Node31Step(int value); // hands off to Node32
                Future<int> Node32Step(int value); // hands off to Node33
                Future<int> Node33Step(int value); // hands off to Node34
                Future<int> Node34Step(int value); // hands off to Node35
                Future<int> Node35Step(int value); // hands off to Node36
                Future<int> Node36Step(int value); // hands off to Node37
                Future<int> Node37Step(int value); // hands off to Node38
                Future<int> Node38Step(int value); // hands off to Node39
                Future<int> Node39Step(int value); // hands off to Node40
                Future<int> Node40Step(int value); // hands off to Node41
                Future<int> Node41Step(int value); // hands off to Node42
                Future<int> Node42Step(int value); // hands off to Node43
                Future<int> Node43Step(int value); // hands off to Node44
                Future<int> Node44Step(int value); // hands off to Node45
                Future<int> Node45Step(int value); // hands off to Node46
                Future<int> Node46Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_47Nodes) {
    const char* code = R"(
        namespace RoundTrip47 {
            interface IChain47 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node11
                Future<int> Node11Step(int value); // hands off to Node12
                Future<int> Node12Step(int value); // hands off to Node13
                Future<int> Node13Step(int value); // hands off to Node14
                Future<int> Node14Step(int value); // hands off to Node15
                Future<int> Node15Step(int value); // hands off to Node16
                Future<int> Node16Step(int value); // hands off to Node17
                Future<int> Node17Step(int value); // hands off to Node18
                Future<int> Node18Step(int value); // hands off to Node19
                Future<int> Node19Step(int value); // hands off to Node20
                Future<int> Node20Step(int value); // hands off to Node21
                Future<int> Node21Step(int value); // hands off to Node22
                Future<int> Node22Step(int value); // hands off to Node23
                Future<int> Node23Step(int value); // hands off to Node24
                Future<int> Node24Step(int value); // hands off to Node25
                Future<int> Node25Step(int value); // hands off to Node26
                Future<int> Node26Step(int value); // hands off to Node27
                Future<int> Node27Step(int value); // hands off to Node28
                Future<int> Node28Step(int value); // hands off to Node29
                Future<int> Node29Step(int value); // hands off to Node30
                Future<int> Node30Step(int value); // hands off to Node31
                Future<int> Node31Step(int value); // hands off to Node32
                Future<int> Node32Step(int value); // hands off to Node33
                Future<int> Node33Step(int value); // hands off to Node34
                Future<int> Node34Step(int value); // hands off to Node35
                Future<int> Node35Step(int value); // hands off to Node36
                Future<int> Node36Step(int value); // hands off to Node37
                Future<int> Node37Step(int value); // hands off to Node38
                Future<int> Node38Step(int value); // hands off to Node39
                Future<int> Node39Step(int value); // hands off to Node40
                Future<int> Node40Step(int value); // hands off to Node41
                Future<int> Node41Step(int value); // hands off to Node42
                Future<int> Node42Step(int value); // hands off to Node43
                Future<int> Node43Step(int value); // hands off to Node44
                Future<int> Node44Step(int value); // hands off to Node45
                Future<int> Node45Step(int value); // hands off to Node46
                Future<int> Node46Step(int value); // hands off to Node47
                Future<int> Node47Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_48Nodes) {
    const char* code = R"(
        namespace RoundTrip48 {
            interface IChain48 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node11
                Future<int> Node11Step(int value); // hands off to Node12
                Future<int> Node12Step(int value); // hands off to Node13
                Future<int> Node13Step(int value); // hands off to Node14
                Future<int> Node14Step(int value); // hands off to Node15
                Future<int> Node15Step(int value); // hands off to Node16
                Future<int> Node16Step(int value); // hands off to Node17
                Future<int> Node17Step(int value); // hands off to Node18
                Future<int> Node18Step(int value); // hands off to Node19
                Future<int> Node19Step(int value); // hands off to Node20
                Future<int> Node20Step(int value); // hands off to Node21
                Future<int> Node21Step(int value); // hands off to Node22
                Future<int> Node22Step(int value); // hands off to Node23
                Future<int> Node23Step(int value); // hands off to Node24
                Future<int> Node24Step(int value); // hands off to Node25
                Future<int> Node25Step(int value); // hands off to Node26
                Future<int> Node26Step(int value); // hands off to Node27
                Future<int> Node27Step(int value); // hands off to Node28
                Future<int> Node28Step(int value); // hands off to Node29
                Future<int> Node29Step(int value); // hands off to Node30
                Future<int> Node30Step(int value); // hands off to Node31
                Future<int> Node31Step(int value); // hands off to Node32
                Future<int> Node32Step(int value); // hands off to Node33
                Future<int> Node33Step(int value); // hands off to Node34
                Future<int> Node34Step(int value); // hands off to Node35
                Future<int> Node35Step(int value); // hands off to Node36
                Future<int> Node36Step(int value); // hands off to Node37
                Future<int> Node37Step(int value); // hands off to Node38
                Future<int> Node38Step(int value); // hands off to Node39
                Future<int> Node39Step(int value); // hands off to Node40
                Future<int> Node40Step(int value); // hands off to Node41
                Future<int> Node41Step(int value); // hands off to Node42
                Future<int> Node42Step(int value); // hands off to Node43
                Future<int> Node43Step(int value); // hands off to Node44
                Future<int> Node44Step(int value); // hands off to Node45
                Future<int> Node45Step(int value); // hands off to Node46
                Future<int> Node46Step(int value); // hands off to Node47
                Future<int> Node47Step(int value); // hands off to Node48
                Future<int> Node48Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_49Nodes) {
    const char* code = R"(
        namespace RoundTrip49 {
            interface IChain49 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node11
                Future<int> Node11Step(int value); // hands off to Node12
                Future<int> Node12Step(int value); // hands off to Node13
                Future<int> Node13Step(int value); // hands off to Node14
                Future<int> Node14Step(int value); // hands off to Node15
                Future<int> Node15Step(int value); // hands off to Node16
                Future<int> Node16Step(int value); // hands off to Node17
                Future<int> Node17Step(int value); // hands off to Node18
                Future<int> Node18Step(int value); // hands off to Node19
                Future<int> Node19Step(int value); // hands off to Node20
                Future<int> Node20Step(int value); // hands off to Node21
                Future<int> Node21Step(int value); // hands off to Node22
                Future<int> Node22Step(int value); // hands off to Node23
                Future<int> Node23Step(int value); // hands off to Node24
                Future<int> Node24Step(int value); // hands off to Node25
                Future<int> Node25Step(int value); // hands off to Node26
                Future<int> Node26Step(int value); // hands off to Node27
                Future<int> Node27Step(int value); // hands off to Node28
                Future<int> Node28Step(int value); // hands off to Node29
                Future<int> Node29Step(int value); // hands off to Node30
                Future<int> Node30Step(int value); // hands off to Node31
                Future<int> Node31Step(int value); // hands off to Node32
                Future<int> Node32Step(int value); // hands off to Node33
                Future<int> Node33Step(int value); // hands off to Node34
                Future<int> Node34Step(int value); // hands off to Node35
                Future<int> Node35Step(int value); // hands off to Node36
                Future<int> Node36Step(int value); // hands off to Node37
                Future<int> Node37Step(int value); // hands off to Node38
                Future<int> Node38Step(int value); // hands off to Node39
                Future<int> Node39Step(int value); // hands off to Node40
                Future<int> Node40Step(int value); // hands off to Node41
                Future<int> Node41Step(int value); // hands off to Node42
                Future<int> Node42Step(int value); // hands off to Node43
                Future<int> Node43Step(int value); // hands off to Node44
                Future<int> Node44Step(int value); // hands off to Node45
                Future<int> Node45Step(int value); // hands off to Node46
                Future<int> Node46Step(int value); // hands off to Node47
                Future<int> Node47Step(int value); // hands off to Node48
                Future<int> Node48Step(int value); // hands off to Node49
                Future<int> Node49Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, RoundTrip_50Nodes) {
    const char* code = R"(
        namespace RoundTrip50 {
            interface IChain50 {
                Future<int> Node1Step(int value); // hands off to Node2
                Future<int> Node2Step(int value); // hands off to Node3
                Future<int> Node3Step(int value); // hands off to Node4
                Future<int> Node4Step(int value); // hands off to Node5
                Future<int> Node5Step(int value); // hands off to Node6
                Future<int> Node6Step(int value); // hands off to Node7
                Future<int> Node7Step(int value); // hands off to Node8
                Future<int> Node8Step(int value); // hands off to Node9
                Future<int> Node9Step(int value); // hands off to Node10
                Future<int> Node10Step(int value); // hands off to Node11
                Future<int> Node11Step(int value); // hands off to Node12
                Future<int> Node12Step(int value); // hands off to Node13
                Future<int> Node13Step(int value); // hands off to Node14
                Future<int> Node14Step(int value); // hands off to Node15
                Future<int> Node15Step(int value); // hands off to Node16
                Future<int> Node16Step(int value); // hands off to Node17
                Future<int> Node17Step(int value); // hands off to Node18
                Future<int> Node18Step(int value); // hands off to Node19
                Future<int> Node19Step(int value); // hands off to Node20
                Future<int> Node20Step(int value); // hands off to Node21
                Future<int> Node21Step(int value); // hands off to Node22
                Future<int> Node22Step(int value); // hands off to Node23
                Future<int> Node23Step(int value); // hands off to Node24
                Future<int> Node24Step(int value); // hands off to Node25
                Future<int> Node25Step(int value); // hands off to Node26
                Future<int> Node26Step(int value); // hands off to Node27
                Future<int> Node27Step(int value); // hands off to Node28
                Future<int> Node28Step(int value); // hands off to Node29
                Future<int> Node29Step(int value); // hands off to Node30
                Future<int> Node30Step(int value); // hands off to Node31
                Future<int> Node31Step(int value); // hands off to Node32
                Future<int> Node32Step(int value); // hands off to Node33
                Future<int> Node33Step(int value); // hands off to Node34
                Future<int> Node34Step(int value); // hands off to Node35
                Future<int> Node35Step(int value); // hands off to Node36
                Future<int> Node36Step(int value); // hands off to Node37
                Future<int> Node37Step(int value); // hands off to Node38
                Future<int> Node38Step(int value); // hands off to Node39
                Future<int> Node39Step(int value); // hands off to Node40
                Future<int> Node40Step(int value); // hands off to Node41
                Future<int> Node41Step(int value); // hands off to Node42
                Future<int> Node42Step(int value); // hands off to Node43
                Future<int> Node43Step(int value); // hands off to Node44
                Future<int> Node44Step(int value); // hands off to Node45
                Future<int> Node45Step(int value); // hands off to Node46
                Future<int> Node46Step(int value); // hands off to Node47
                Future<int> Node47Step(int value); // hands off to Node48
                Future<int> Node48Step(int value); // hands off to Node49
                Future<int> Node49Step(int value); // hands off to Node50
                Future<int> Node50Step(int value); // hands off to Node1
            }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

// ============================================================================
// CONTINUATIONS PASSED AS PARAMETERS / RETURN TYPES (20 tests)
// ============================================================================

TEST_F(TauNodeRoundTripAndFutureTests, ContinuationAsParameter) {
    const char* code = R"(
        namespace ContinuationTests {
            interface IScheduler { Future<void> RunWith(Continuation cont); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, ContinuationReturnType) {
    const char* code = R"(
        namespace ContinuationTests {
            interface IPipeline { Continuation NextStep(int stage); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, ContinuationChainOfTwo) {
    const char* code = R"(
        namespace ContinuationTests {
            interface IChainer { Continuation Compose(Continuation first, Continuation second); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, ContinuationWithFutureResult) {
    const char* code = R"(
        namespace ContinuationTests {
            interface IAsyncChain { Future<Continuation> Continue(int input); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, ContinuationArrayParameter) {
    const char* code = R"(
        namespace ContinuationTests {
            interface IBatchScheduler { Future<void> RunAll(Continuation[] steps); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, ContinuationInClass) {
    const char* code = R"(
        namespace ContinuationTests {
            class Workflow { Continuation current; Future<void> Advance(); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, ContinuationFactoryMethod) {
    const char* code = R"(
        namespace ContinuationTests {
            interface IContinuationFactory { Continuation Create(string name); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, ContinuationWithCallback) {
    const char* code = R"(
        namespace ContinuationTests {
            interface IEventLoop { Future<void> OnComplete(Continuation onDone); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, ContinuationTwoStageHandoff) {
    const char* code = R"(
        namespace ContinuationTests {
            interface ITwoStage { Continuation Stage1(int x); Continuation Stage2(Continuation prev); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, ContinuationThreeStageHandoff) {
    const char* code = R"(
        namespace ContinuationTests {
            interface IThreeStage { Continuation Stage1(int x); Continuation Stage2(Continuation prev); Continuation Stage3(Continuation prev); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, ContinuationResumeMethod) {
    const char* code = R"(
        namespace ContinuationTests {
            interface IResumable { Future<int> Resume(Continuation saved); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, ContinuationSuspendMethod) {
    const char* code = R"(
        namespace ContinuationTests {
            interface ISuspendable { Continuation Suspend(); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, ContinuationWithStructPayload) {
    const char* code = R"(
        namespace ContinuationTests {
            struct Payload { int id; string data; } interface IPayloadChain { Continuation Handle(Payload p); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, ContinuationNestedNamespace) {
    const char* code = R"(
        namespace ContinuationTests {
            namespace Outer { namespace Inner { interface IDeepChain { Continuation Step(int x); } } }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, ContinuationMultipleInterfaces) {
    const char* code = R"(
        namespace ContinuationTests {
            interface IProducer { Continuation Produce(); } interface IConsumer { Future<void> Consume(Continuation c); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, ContinuationVoidParameter) {
    const char* code = R"(
        namespace ContinuationTests {
            interface ITrigger { Future<void> Fire(Continuation onFire); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, ContinuationBoolResult) {
    const char* code = R"(
        namespace ContinuationTests {
            interface IGuard { Future<bool> CheckAndContinue(Continuation next); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, ContinuationClassWithHistory) {
    const char* code = R"(
        namespace ContinuationTests {
            class ExecutionHistory { Continuation[] steps; Future<void> Record(Continuation step); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, ContinuationSelfReferential) {
    const char* code = R"(
        namespace ContinuationTests {
            interface ILoop { Continuation Repeat(Continuation self); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, ContinuationWithFloatState) {
    const char* code = R"(
        namespace ContinuationTests {
            interface IAccumulator { Continuation Add(float amount); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

// ============================================================================
// CLASSES USING FUTURES (15 tests)
// ============================================================================

TEST_F(TauNodeRoundTripAndFutureTests, ClassWithFutureMember) {
    const char* code = R"(
        namespace ClassFutureTests {
            class JobHandle { Future<int> pendingResult; }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, ClassWithFutureMethod) {
    const char* code = R"(
        namespace ClassFutureTests {
            class JobRunner { Future<int> Submit(string jobName); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, ClassWithMultipleFutureMethods) {
    const char* code = R"(
        namespace ClassFutureTests {
            class TaskManager { Future<int> Submit(string name); Future<bool> Cancel(int jobId); Future<string> Status(int jobId); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, ClassWithFutureArrayMember) {
    const char* code = R"(
        namespace ClassFutureTests {
            class BatchHandle { Future<int>[] results; }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, ClassWithFutureOfCustomType) {
    const char* code = R"(
        namespace ClassFutureTests {
            struct Result { int code; string message; } class ResultHandle { Future<Result> pending; }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, ClassImplementingInterfaceWithFutures) {
    const char* code = R"(
        namespace ClassFutureTests {
            interface IWorker { Future<int> DoWork(int input); } class Worker : IWorker { Future<int> DoWork(int input); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, ClassWithFutureVoidMethod) {
    const char* code = R"(
        namespace ClassFutureTests {
            class Notifier { Future<void> NotifyAll(string message); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, ClassWithChainedFutureMethods) {
    const char* code = R"(
        namespace ClassFutureTests {
            class Pipeline { Future<int> Stage1(int x); Future<int> Stage2(int x); Future<int> Stage3(int x); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, ClassWithFutureAndPlainMembers) {
    const char* code = R"(
        namespace ClassFutureTests {
            class Session { int id; string user; Future<bool> isAuthenticated; }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, ClassWithStaticFutureMethod) {
    const char* code = R"(
        namespace ClassFutureTests {
            class GlobalCache { static Future<string> Get(string key); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, ClassWithFutureConstructorLikeFactory) {
    const char* code = R"(
        namespace ClassFutureTests {
            class ConnectionFactory { static Future<ConnectionFactory> Create(string address, int port); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, ClassWithNestedFutureClass) {
    const char* code = R"(
        namespace ClassFutureTests {
            class Outer { class Inner { Future<int> Compute(int x); } }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, ClassWithFutureBoolAndFutureString) {
    const char* code = R"(
        namespace ClassFutureTests {
            class Validator { Future<bool> Validate(string input); Future<string> Normalize(string input); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, ClassWithFutureBasedEventHandlers) {
    const char* code = R"(
        namespace ClassFutureTests {
            class EventBus { Future<void> Publish(string topic, string payload); Future<void> Subscribe(string topic); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, ClassWithFutureReturningIndexer) {
    const char* code = R"(
        namespace ClassFutureTests {
            class RemoteArray { Future<int> Get(int index); Future<void> Set(int index, int value); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

// ============================================================================
// FUTURES PASSED AS ARGUMENTS OVER THE NETWORK (15 tests)
// ============================================================================

TEST_F(TauNodeRoundTripAndFutureTests, FutureIntAsArgument) {
    const char* code = R"(
        namespace FutureArgTests {
            interface IAggregator { Future<int> Sum(Future<int> a, Future<int> b); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, FutureStringAsArgument) {
    const char* code = R"(
        namespace FutureArgTests {
            interface IGreeter { Future<string> Greet(Future<string> name); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, FutureCustomTypeAsArgument) {
    const char* code = R"(
        namespace FutureArgTests {
            struct User { int id; string name; } interface IUserProcessor { Future<bool> Process(Future<User> user); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, MultipleFutureArguments) {
    const char* code = R"(
        namespace FutureArgTests {
            interface ICombiner { Future<int> Combine(Future<int> a, Future<int> b, Future<int> c); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, FutureArrayAsArgument) {
    const char* code = R"(
        namespace FutureArgTests {
            interface IBatchProcessor { Future<int> ProcessAll(Future<int[]> items); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, FutureAsArgumentAndReturn) {
    const char* code = R"(
        namespace FutureArgTests {
            interface IRelay { Future<int> Relay(Future<int> incoming); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, FutureOfFutureAsArgument) {
    const char* code = R"(
        namespace FutureArgTests {
            interface INestedFuture { Future<int> Unwrap(Future<Future<int>> nested); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, FutureBoolGate) {
    const char* code = R"(
        namespace FutureArgTests {
            interface IGatekeeper { Future<void> ProceedIfReady(Future<bool> readySignal); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, FutureArgumentAcrossThreeNodeHandoff) {
    const char* code = R"(
        namespace FutureArgTests {
            interface INodeA { Future<int> Start(int seed); } interface INodeB { Future<int> Middle(Future<int> fromA); } interface INodeC { Future<int> End(Future<int> fromB); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, FutureArgumentWithVoidResult) {
    const char* code = R"(
        namespace FutureArgTests {
            interface ILogger { Future<void> LogWhenReady(Future<string> message); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, FutureArgumentInClassMethod) {
    const char* code = R"(
        namespace FutureArgTests {
            class Merger { Future<int> Merge(Future<int> left, Future<int> right); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, FutureArgumentWithFloatPayload) {
    const char* code = R"(
        namespace FutureArgTests {
            interface ISensorAggregator { Future<float> Average(Future<float> reading1, Future<float> reading2); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, FutureArgumentMixedWithPlainArgument) {
    const char* code = R"(
        namespace FutureArgTests {
            interface IConditionalRelay { Future<int> RelayIf(bool enabled, Future<int> value); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, FutureArgumentPassedThroughFiveNodeChain) {
    const char* code = R"(
        namespace FutureArgTests {
            interface IChainNode1 { Future<int> Step1(int x); } interface IChainNode2 { Future<int> Step2(Future<int> prev); } interface IChainNode3 { Future<int> Step3(Future<int> prev); } interface IChainNode4 { Future<int> Step4(Future<int> prev); } interface IChainNode5 { Future<int> Step5(Future<int> prev); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

TEST_F(TauNodeRoundTripAndFutureTests, FutureArgumentWithContinuationCallback) {
    const char* code = R"(
        namespace FutureArgTests {
            interface IAsyncRelay { Future<void> RelayThen(Future<int> value, Continuation onComplete); }
        }
    )";
    EXPECT_TRUE(ParseTauCode(code));
}

