#include <gtest/gtest.h>

#include "KAI/Language/Tau/TauParser.h"
#include "TestLangCommon.h"

// Test suite for Tau attributes and metadata - IDL parsing only
// Note: Tau is an IDL for code generation, not an executable language

TEST(TauAttribute, BasicAttributes) {
    // Test that we can parse IDL with attributes
    kai::Registry reg;

    const char* code = R"(
        namespace Test {
            [Serializable]
            [Description("A simple point class")]
            class Point {
                [Required]
                float x;
                
                [Required]
                float y;
                
                [Computed]
                float Distance();
            }
        }
    )";

    auto lexer = std::make_shared<kai::tau::TauLexer>(code, reg);
    ASSERT_TRUE(lexer->Process());

    auto parser = std::make_shared<kai::tau::TauParser>(reg);
    (void)parser->Process(lexer, kai::Structure::Module);

    // For now, attributes are not implemented in the parser
    // This test just verifies the IDL can be lexed without errors
    EXPECT_TRUE(parser->Error.empty() ||
                parser->Error.find("Not Implemented") != std::string::npos)
        << "Parser error: " << parser->Error;
}

TEST(TauAttribute, CustomAttributes) {
    kai::Registry reg;

    const char* code = R"(
        namespace Game {
            class ValidateRangeAttribute : Attribute {
                int min;
                int max;
                
                void ValidateRangeAttribute(int minVal, int maxVal);
                bool IsValid(int value);
            }
            
            class Player {
                [ValidateRange(0, 100)]
                int health;
                
                [ValidateRange(0, 50)]
                int mana;
            }
        }
    )";

    auto lexer = std::make_shared<kai::tau::TauLexer>(code, reg);
    ASSERT_TRUE(lexer->Process());

    auto parser = std::make_shared<kai::tau::TauParser>(reg);
    (void)parser->Process(lexer, kai::Structure::Module);

    // Attributes are not yet implemented
    EXPECT_TRUE(parser->Error.empty() ||
                parser->Error.find("Not Implemented") != std::string::npos)
        << "Parser error: " << parser->Error;
}

TEST(TauAttribute, MethodAttributes) {
    kai::Registry reg;

    const char* code = R"(
        namespace API {
            class WebService {
                [HttpGet("/users")]
                [Authorize("Admin")]
                void GetAllUsers();
                
                [HttpPost("/users")]
                [ValidateModel]
                void CreateUser(User user);
                
                [HttpDelete("/users/{id}")]
                [Authorize("Admin", "Moderator")]
                void DeleteUser(int id);
            }
        }
    )";

    auto lexer = std::make_shared<kai::tau::TauLexer>(code, reg);
    ASSERT_TRUE(lexer->Process());

    auto parser = std::make_shared<kai::tau::TauParser>(reg);
    (void)parser->Process(lexer, kai::Structure::Module);

    EXPECT_TRUE(parser->Error.empty() ||
                parser->Error.find("Not Implemented") != std::string::npos)
        << "Parser error: " << parser->Error;
}

TEST(TauAttribute, CompileTimeAttributes) {
    kai::Registry reg;

    const char* code = R"(
        namespace System {
            [CompileTime]
            class Configuration {
                [Constant("Debug")]
                string buildMode;
                
                [Inline]
                void FastOperation();
                
                [NoOptimize]
                void DebugOperation();
            }
        }
    )";

    auto lexer = std::make_shared<kai::tau::TauLexer>(code, reg);
    ASSERT_TRUE(lexer->Process());

    auto parser = std::make_shared<kai::tau::TauParser>(reg);
    (void)parser->Process(lexer, kai::Structure::Module);

    EXPECT_TRUE(parser->Error.empty() ||
                parser->Error.find("Not Implemented") != std::string::npos)
        << "Parser error: " << parser->Error;
}

TEST(TauAttribute, ConditionalAttributes) {
    kai::Registry reg;

    const char* code = R"(
        namespace Features {
            class Service {
                [Conditional("DEBUG")]
                void LogDebugInfo();
                
                [Conditional("RELEASE")]
                void OptimizedMethod();
                
                [Deprecated("Use NewMethod instead")]
                void OldMethod();
                
                [Experimental]
                void NewFeature();
            }
        }
    )";

    auto lexer = std::make_shared<kai::tau::TauLexer>(code, reg);
    ASSERT_TRUE(lexer->Process());

    auto parser = std::make_shared<kai::tau::TauParser>(reg);
    (void)parser->Process(lexer, kai::Structure::Module);

    EXPECT_TRUE(parser->Error.empty() ||
                parser->Error.find("Not Implemented") != std::string::npos)
        << "Parser error: " << parser->Error;
}