// TauCenturyTests.cpp
//
// 50 additional Tau IDL tests: realistic service/domain definitions (chat,
// inventory, matchmaking, health, networking messages), broader primitive
// and container type coverage, inheritance/composition shapes, extended
// enum coverage, larger composed files, and malformed-input rejection.
// Positive-parse tests use Generate::GenerateProxy (the pattern established
// by TauComprehensiveTests.cpp: proxy.failed / proxy.error); malformed-input
// tests use a self-contained lex+parse-only helper (the pattern established
// by TauEdgeCaseTests.cpp: parser->error, not the disabled-logging-only
// `lex->Error` capital-E spelling that appears in some older, unbuilt files).

#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "KAI/Core/Registry.h"
#include "KAI/Language/Tau/Generate/GenerateProxy.h"
#include "KAI/Language/Tau/TauLexer.h"
#include "KAI/Language/Tau/TauParser.h"

using namespace kai;
using namespace kai::tau;
using namespace std;

namespace {

bool ParseTauContent(const string &content) {
    if (content.find_first_not_of(" \t\r\n") == string::npos) return false;
    try {
        Registry registry;
        auto lexer = std::make_shared<TauLexer>(content.c_str(), registry);
        if (!lexer->Process()) return false;

        auto parser = std::make_shared<TauParser>(registry);
        parser->SetStrictMode(true);
        if (!parser->Process(lexer, Structure::Module)) return false;
        if (!parser->error.empty()) return false;

        auto root = parser->GetRoot();
        return root && !root->GetChildren().empty();
    } catch (const std::exception &) {
        return false;
    }
}

}  // namespace

struct TauCenturyTests : ::testing::Test {
    void ExpectParses(const string &code) {
        string proxyOutput;
        Generate::GenerateProxy proxy(code.c_str(), proxyOutput);
        ASSERT_FALSE(proxy.failed) << proxy.error;
    }
};

// --------------------------------------------------------------------------
// Realistic service/domain definitions (12)
// --------------------------------------------------------------------------

TEST_F(TauCenturyTests, ChatServiceInterface) {
    ExpectParses(R"(
        namespace Chat {
            interface IChatService {
                Future<bool> SendMessage(int roomId, string text);
                Future<string[]> GetHistory(int roomId, int count);
            }
        }
    )");
}

TEST_F(TauCenturyTests, UserAccountStruct) {
    ExpectParses(R"(
        namespace Accounts {
            struct UserAccount {
                int id;
                string name;
                string email;
                bool active;
            }
        }
    )");
}

TEST_F(TauCenturyTests, InventorySystemNamespace) {
    ExpectParses(R"(
        namespace Inventory {
            struct Item {
                int id;
                string name;
                int quantity;
            }
            interface IInventory {
                Future<bool> AddItem(Item item);
                Future<bool> RemoveItem(int id);
                Future<int> GetCount();
            }
        }
    )");
}

TEST_F(TauCenturyTests, LeaderboardRankEnum) {
    ExpectParses(R"(
        namespace Leaderboard {
            enum Rank {
                Bronze,
                Silver,
                Gold,
                Platinum
            }
        }
    )");
}

TEST_F(TauCenturyTests, GameEventInterface) {
    ExpectParses(R"(
        namespace Game {
            interface IGameEvents {
                event PlayerJoined(int playerId);
                event PlayerLeft(int playerId);
            }
        }
    )");
}

TEST_F(TauCenturyTests, MatchmakingRequestStruct) {
    ExpectParses(R"(
        namespace Matchmaking {
            struct MatchRequest {
                int playerId;
                int skillLevel;
                string region;
            }
        }
    )");
}

TEST_F(TauCenturyTests, HealthSystemInterface) {
    ExpectParses(R"(
        namespace Combat {
            interface IHealth {
                Future<bool> TakeDamage(int amount);
                Future<bool> Heal(int amount);
                Future<int> GetHealth();
            }
        }
    )");
}

TEST_F(TauCenturyTests, WeaponTypeEnum) {
    ExpectParses(R"(
        namespace Combat {
            enum WeaponType {
                Sword,
                Bow,
                Staff,
                Dagger,
                Hammer
            }
        }
    )");
}

TEST_F(TauCenturyTests, InventoryArrayReturnInterface) {
    ExpectParses(R"(
        namespace Inventory {
            struct Item {
                int id;
                string name;
            }
            interface IInventoryQuery {
                Future<Item[]> GetAllItems();
            }
        }
    )");
}

TEST_F(TauCenturyTests, ChatRoomsNestedNamespace) {
    ExpectParses(R"(
        namespace Chat {
            namespace Rooms {
                struct RoomInfo {
                    int id;
                    string topic;
                    int memberCount;
                }
            }
        }
    )");
}

TEST_F(TauCenturyTests, NetworkMessageStruct) {
    ExpectParses(R"(
        namespace Networking {
            struct NetworkMessage {
                byte[] payload;
                int64 timestamp;
                int senderId;
            }
        }
    )");
}

TEST_F(TauCenturyTests, PlayerProfileCombinedFeatures) {
    ExpectParses(R"(
        namespace Player {
            enum Status {
                Offline,
                Online,
                AwayFromKeyboard
            }
            struct Profile {
                int id;
                string displayName;
                Status status;
            }
            interface IProfileService {
                Future<Profile> GetProfile(int id);
                event StatusChanged(int id, Status newStatus);
            }
        }
    )");
}

// --------------------------------------------------------------------------
// Type-system coverage (10)
// --------------------------------------------------------------------------

TEST_F(TauCenturyTests, WideIntegerFields) {
    ExpectParses(R"(
        namespace Types {
            struct WideInts {
                int64 big;
                short small;
                int regular;
            }
        }
    )");
}

TEST_F(TauCenturyTests, UnsignedFields) {
    ExpectParses(R"(
        namespace Types {
            struct Unsigned {
                uint32 a;
                uint64 b;
                byte c;
            }
        }
    )");
}

TEST_F(TauCenturyTests, FloatingPointFields) {
    ExpectParses(R"(
        namespace Types {
            struct Floats {
                float single;
                double precise;
            }
        }
    )");
}

TEST_F(TauCenturyTests, BooleanArrayField) {
    ExpectParses(R"(
        namespace Types {
            struct Flags {
                bool[] values;
            }
        }
    )");
}

TEST_F(TauCenturyTests, StringArrayField) {
    ExpectParses(R"(
        namespace Types {
            struct Tags {
                string[] values;
            }
        }
    )");
}

TEST_F(TauCenturyTests, ArrayOfStructsField) {
    ExpectParses(R"(
        namespace Types {
            struct Item {
                int id;
            }
            struct Bag {
                Item[] items;
            }
        }
    )");
}

TEST_F(TauCenturyTests, MultipleReturnTypesInterface) {
    ExpectParses(R"(
        namespace Types {
            interface IMulti {
                Future<int> AsInt();
                Future<string> AsString();
                Future<bool> AsBool();
                Future<float> AsFloat();
            }
        }
    )");
}

TEST_F(TauCenturyTests, FutureOfPrimitive) {
    ExpectParses(R"(
        namespace Types {
            interface IFuturePrimitive {
                Future<int> GetValue();
            }
        }
    )");
}

TEST_F(TauCenturyTests, FutureOfCustomStruct) {
    ExpectParses(R"(
        namespace Types {
            struct Payload {
                int id;
                string data;
            }
            interface IFutureStruct {
                Future<Payload> GetPayload();
            }
        }
    )");
}

TEST_F(TauCenturyTests, FutureOfArray) {
    ExpectParses(R"(
        namespace Types {
            interface IFutureArray {
                Future<int[]> GetValues();
            }
        }
    )");
}

// --------------------------------------------------------------------------
// Inheritance and composition (6)
// --------------------------------------------------------------------------

TEST_F(TauCenturyTests, SingleInheritanceInterface) {
    ExpectParses(R"(
        namespace Inherit {
            interface IBase {
                Future<int> BaseMethod();
            }
            interface IDerived : IBase {
                Future<int> DerivedMethod();
            }
        }
    )");
}

TEST_F(TauCenturyTests, DiamondInheritanceInterfaces) {
    ExpectParses(R"(
        namespace Inherit {
            interface IBase {
                Future<int> BaseMethod();
            }
            interface IA : IBase {
                Future<int> AMethod();
            }
            interface IB : IBase {
                Future<int> BMethod();
            }
            interface IC : IA, IB {
                Future<int> CMethod();
            }
        }
    )");
}

TEST_F(TauCenturyTests, ThreeLevelInheritanceChain) {
    ExpectParses(R"(
        namespace Inherit {
            interface ILevel1 {
                Future<int> Method1();
            }
            interface ILevel2 : ILevel1 {
                Future<int> Method2();
            }
            interface ILevel3 : ILevel2 {
                Future<int> Method3();
            }
        }
    )");
}

TEST_F(TauCenturyTests, InterfaceWithMultipleEventsAndMethods) {
    ExpectParses(R"(
        namespace Inherit {
            interface IRich {
                Future<int> DoWork();
                event Started();
                event Finished(int result);
            }
        }
    )");
}

TEST_F(TauCenturyTests, StructComposedOfMultipleStructs) {
    ExpectParses(R"(
        namespace Inherit {
            struct Point { float x; float y; }
            struct Size { float w; float h; }
            struct Velocity { float dx; float dy; }
            struct Body {
                Point position;
                Size bounds;
                Velocity velocity;
            }
        }
    )");
}

TEST_F(TauCenturyTests, NamespaceWithInterfaceImplementingMultipleBases) {
    ExpectParses(R"(
        namespace Inherit {
            interface IReadable {
                Future<string> Read();
            }
            interface IWritable {
                Future<bool> Write(string data);
            }
            interface IReadWrite : IReadable, IWritable {
                Future<bool> Flush();
            }
        }
    )");
}

// --------------------------------------------------------------------------
// Extended enum coverage (5)
// --------------------------------------------------------------------------

TEST_F(TauCenturyTests, EnumWithAllExplicitValues) {
    ExpectParses(R"(
        namespace Enums {
            enum Code {
                Ok = 0,
                Warning = 1,
                Error = 2,
                Fatal = 3
            }
        }
    )");
}

TEST_F(TauCenturyTests, EnumUsedAsFieldAndParam) {
    ExpectParses(R"(
        namespace Enums {
            enum Level { Low, Medium, High }
            struct Config {
                Level level;
            }
            interface ILevelService {
                Future<bool> SetLevel(Level level);
            }
        }
    )");
}

TEST_F(TauCenturyTests, EnumWithNegativeAndPositive) {
    ExpectParses(R"(
        namespace Enums {
            enum Offset {
                Negative = -5,
                Zero = 0,
                Positive = 5
            }
        }
    )");
}

TEST_F(TauCenturyTests, MultipleEnumsCrossReferenced) {
    ExpectParses(R"(
        namespace Enums {
            enum Suit { Hearts, Diamonds, Clubs, Spades }
            enum Rank { Two, Three, Four, Five, Six, Seven, Eight, Nine, Ten, Jack, Queen, King, Ace }
            struct Card {
                Suit suit;
                Rank rank;
            }
        }
    )");
}

TEST_F(TauCenturyTests, ManyEnumsInOneNamespace) {
    ExpectParses(R"(
        namespace Enums {
            enum A { A1, A2 }
            enum B { B1, B2 }
            enum C { C1, C2 }
            enum D { D1, D2 }
        }
    )");
}

// --------------------------------------------------------------------------
// Larger composed files (5)
// --------------------------------------------------------------------------

TEST_F(TauCenturyTests, FullServiceDefinitionFile) {
    ExpectParses(R"(
        namespace FullService {
            enum Status { Pending, Active, Closed }

            struct Ticket {
                int id;
                string title;
                Status status;
            }

            interface ITicketService {
                Future<int> CreateTicket(string title);
                Future<bool> CloseTicket(int id);
                Future<Ticket[]> ListTickets();
                event TicketCreated(int id);
                event TicketClosed(int id);
            }
        }
    )");
}

TEST_F(TauCenturyTests, TwoNamespacesTwoInterfaces) {
    ExpectParses(R"(
        namespace Alpha {
            interface IAlpha {
                Future<int> DoAlpha();
            }
        }
        namespace Beta {
            interface IBeta {
                Future<int> DoBeta();
            }
        }
    )");
}

TEST_F(TauCenturyTests, DeeplyNestedFourLevelsWithFullContent) {
    ExpectParses(R"(
        namespace L1 {
            namespace L2 {
                namespace L3 {
                    namespace L4 {
                        struct Leaf {
                            int value;
                        }
                        interface ILeaf {
                            Future<Leaf> GetLeaf();
                        }
                    }
                }
            }
        }
    )");
}

TEST_F(TauCenturyTests, MultipleStructsReferencingEachOther) {
    ExpectParses(R"(
        namespace Refs {
            struct Address {
                string city;
                string country;
            }
            struct Company {
                string name;
                Address headquarters;
            }
            struct Employee {
                string name;
                Company employer;
                Address homeAddress;
            }
        }
    )");
}

TEST_F(TauCenturyTests, LargeInterfaceManyMethods) {
    ExpectParses(R"(
        namespace Large {
            interface IBig {
                Future<int> MethodOne();
                Future<int> MethodTwo();
                Future<int> MethodThree();
                Future<int> MethodFour();
                Future<int> MethodFive();
                Future<int> MethodSix();
                Future<int> MethodSeven();
                Future<int> MethodEight();
                Future<int> MethodNine();
                Future<int> MethodTen();
            }
        }
    )");
}

// --------------------------------------------------------------------------
// Comments and whitespace (2)
// --------------------------------------------------------------------------

TEST_F(TauCenturyTests, LineCommentsInterspersed) {
    ExpectParses(R"(
        // Top-level comment
        namespace Commented {
            // A struct with a comment above it
            struct Data {
                int id; // inline comment on a field
                string name;
            }
        }
    )");
}

TEST_F(TauCenturyTests, BlockCommentsInterspersed) {
    ExpectParses(R"(
        /* Top-level block comment */
        namespace Commented {
            struct /* struct name follows */ Data {
                int id;
                /* a field */
                string name;
            }
        }
    )");
}

// --------------------------------------------------------------------------
// Malformed input rejection (10)
// --------------------------------------------------------------------------

TEST_F(TauCenturyTests, RejectsEmptyContent) {
    EXPECT_FALSE(ParseTauContent(""));
}

TEST_F(TauCenturyTests, RejectsWhitespaceOnly) {
    EXPECT_FALSE(ParseTauContent("   \n\t  "));
}

TEST_F(TauCenturyTests, RejectsMismatchedBraceCount) {
    EXPECT_FALSE(ParseTauContent(R"(
        namespace Broken {
            struct Data {
                int id;
        }
    )"));
}

TEST_F(TauCenturyTests, RejectsMissingNamespaceName) {
    EXPECT_FALSE(ParseTauContent(R"(
        namespace {
            struct Data { int id; }
        }
    )"));
}

TEST_F(TauCenturyTests, RejectsUnclosedArrayBracket) {
    EXPECT_FALSE(ParseTauContent(R"(
        namespace Broken {
            struct Data {
                int[ numbers;
            }
        }
    )"));
}

TEST_F(TauCenturyTests, RejectsStructFieldMissingType) {
    EXPECT_FALSE(ParseTauContent(R"(
        namespace Broken {
            struct Data {
                value;
            }
        }
    )"));
}

TEST_F(TauCenturyTests, RejectsInterfaceMissingClosingBrace) {
    EXPECT_FALSE(ParseTauContent(R"(
        namespace Broken {
            interface IBroken {
                Future<int> Method();
        }
    )"));
}

TEST_F(TauCenturyTests, RejectsNumberAsStructName) {
    EXPECT_FALSE(ParseTauContent(R"(
        namespace Broken {
            struct 123Bad {
                int id;
            }
        }
    )"));
}

TEST_F(TauCenturyTests, RejectsDoubleCommaInParams) {
    EXPECT_FALSE(ParseTauContent(R"(
        namespace Broken {
            interface IBroken {
                Future<int> Method(int a,, int b);
            }
        }
    )"));
}

TEST_F(TauCenturyTests, RejectsMethodMissingSemicolon) {
    EXPECT_FALSE(ParseTauContent(R"(
        namespace Broken {
            interface IBroken {
                Future<int> Method()
            }
        }
    )"));
}
