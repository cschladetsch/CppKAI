#include <gtest/gtest.h>
#include "TestLangCommon.h"
#include "KAI/Console.h"
#include "KAI/Language/Rho/RhoLexer.h"
#include "KAI/Language/Rho/RhoParser.h"

using namespace kai;

TEST(DebugParseTree, NestedCallTree) {
    Console console;
    console.SetLanguage(Language::Rho);
    
    // Parse outer(inner()) and print the tree
    std::string code = "outer(inner())";
    
    RhoLexer lexer(code.c_str(), console.GetRegistry());
    ASSERT_TRUE(lexer.Process());
    
    RhoParser parser(console.GetRegistry());
    
    // Create a shared_ptr from the lexer
    auto lexerPtr = std::make_shared<RhoLexer>(lexer);
    parser.Process(lexerPtr, Structure::Expression);
    
    std::cout << "\n=== Parse tree for: " << code << " ===\n";
    std::cout << parser.PrintTree() << std::endl;
    
    // Also check what gets translated
    std::cout << "\n=== Now executing the same code ===\n";
    
    // Define the functions first
    console.Execute(R"(
fun inner()
    return 5
    
fun outer(x)
    return x + 1
)", Structure::Program);
    
    // Execute and see result
    console.GetExecutor()->GetDataStack()->Clear();
    console.Execute(code, Structure::Expression);
    
    auto stack = console.GetExecutor()->GetDataStack();
    if (!stack->Empty()) {
        std::cout << "Result: " << stack->Top().ToString() << std::endl;
    }
}