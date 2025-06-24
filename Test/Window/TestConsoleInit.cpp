#include <KAI/Console.h>
#include <KAI/Language/Common/TranslatorFactory.h>
#include <KAI/Language/Pi/PiTranslator.h>
#include <KAI/Language/Rho/RhoTranslator.h>
#include <gtest/gtest.h>

#include <iostream>

using namespace kai;

// Register translators
REGISTER_TRANSLATOR(Language::Pi, PiTranslator);
REGISTER_TRANSLATOR(Language::Rho, RhoTranslator);

TEST(ConsoleInit, BasicCreation) {
    std::cout << "Creating Console..." << std::endl;

    try {
        Console console;
        std::cout << "Console created successfully" << std::endl;

        // Test that we can get components
        EXPECT_TRUE(console.GetExecutor().Exists());
        EXPECT_TRUE(&console.GetRegistry() != nullptr);
        EXPECT_TRUE(&console.GetTree() != nullptr);

        std::cout << "All components exist" << std::endl;
    } catch (const std::exception& e) {
        FAIL() << "Exception during console creation: " << e.what();
    }
}

TEST(ConsoleInit, SetLanguage) {
    try {
        Console console;

        // Test setting language
        console.SetLanguage(Language::Pi);
        EXPECT_EQ(console.GetLanguage(), Language::Pi);

        console.SetLanguage(Language::Rho);
        EXPECT_EQ(console.GetLanguage(), Language::Rho);

    } catch (const std::exception& e) {
        FAIL() << "Exception: " << e.what();
    }
}

TEST(ConsoleInit, SimpleExecution) {
    try {
        Console console;
        console.SetLanguage(Language::Pi);

        // Try a simple command
        console.Execute("1 2 +", Structure::Expression);

        // Check stack has result
        auto stack = console.GetExecutor()->GetDataStack();
        EXPECT_GT(stack->Size(), 0);

    } catch (const std::exception& e) {
        FAIL() << "Exception during execution: " << e.what();
    }
}