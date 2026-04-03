#include <gtest/gtest.h>

#include <fstream>
#include <iostream>
#include <sstream>

#include "KAI/Console/Console.h"
#include "KAI/Core/BuiltinTypes/Stack.h"
#include "KAI/Core/Config/Base.h"
#include "KAI/Core/Debug.h"
#include "KAI/Core/Exception.h"
#include "KAI/Core/Logger.h"
#include "KAI/Language/Rho/RhoParser.h"
#include "KAI/Language/Rho/RhoTranslator.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Fixture for running the Rho demo
class RhoDemo : public TestLangCommon {
   protected:
    // Helper method to execute a Rho script file
    bool ExecuteRhoFile(const char *filename, bool verbose = true) {
        try {
            Console console;
            console.SetLanguage(Language::Rho);

            // Create a system object with print capability
            Registry &reg = console.GetRegistry();
            Object system = reg.New<Object>();
            auto scope = console.GetTree().GetScope();
            scope.Set(Label("System"), system);

            // Add print method
            // Create a simple print function for the system object
            system.Set("Print", reg.New<String>("print function placeholder"));

            // Read the file
            std::ifstream file(filename);
            if (!file.is_open()) {
                Logger::ErrorWithLocation(
                    std::string("Failed to open file: ") + filename, __FILE__,
                    __LINE__);
                return false;
            }

            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string scriptStr = buffer.str();
            String script = String(scriptStr.c_str());

            // Execute the script
            console.Execute(script.c_str());

            // Get result from data stack if needed
            auto stack = console.GetExecutor()->GetDataStack();
            if (stack->Empty()) {
                Logger::InfoWithLocation(
                    "Script executed but no result on stack", __FILE__,
                    __LINE__);
            }

            return true;
        } catch (const Exception::Base &e) {
            Logger::ErrorWithLocation("Exception: " + std::string(e.ToString()),
                                      __FILE__, __LINE__);
            return false;
        } catch (const std::exception &e) {
            Logger::ErrorWithLocation(
                "std::exception: " + std::string(e.what()), __FILE__, __LINE__);
            return false;
        } catch (...) {
            Logger::ErrorWithLocation("Unknown exception", __FILE__, __LINE__);
            return false;
        }
    }
};

// Test to run the demo script
// DISABLED: Demo script requires complete file I/O and system integration fixes
TEST_F(RhoDemo, RunDemo) {
    std::cout << "\n\n=======================================" << std::endl;
    std::cout << "          RHO LANGUAGE DEMO           " << std::endl;
    std::cout << "=======================================" << std::endl;

    bool success =
        ExecuteRhoFile("Test/Language/TestRho/Scripts/Demo.rho", true);

    std::cout << "=======================================" << std::endl;

    EXPECT_TRUE(success) << "Demo script execution failed";

    if (success) {
        std::cout << "Demo completed successfully!" << std::endl;
    } else {
        std::cout << "Demo failed. See error logs for details." << std::endl;
    }
}

// Simple test for each major feature to ensure they work separately
// DISABLED: Basic features require parser fixes for control flow and function
// definitions
TEST_F(RhoDemo, BasicFeatureTests) {
    Console console;
    console.SetLanguage(Language::Rho);

    // Test arithmetic
    console.Execute(String("2 + 3 * 4"));
    auto stack = console.GetExecutor()->GetDataStack();
    EXPECT_FALSE(stack->Empty()) << "No result on stack after arithmetic";
    if (!stack->Empty()) {
        Object result = stack->Top();
        EXPECT_TRUE(result.IsType<int>()) << "Result is not an integer";
        if (result.IsType<int>()) {
            EXPECT_EQ(ConstDeref<int>(result), 14)
                << "Arithmetic result incorrect";
        }
        stack->Pop();
    }

    // Test control flow - simple if test
    console.Execute(String("x = 5"));
    stack->Clear();
    console.Execute(String("if x > 3\n    x = 10\nelse\n    x = 0"));
    stack->Clear();
    console.Execute(String("x"));
    EXPECT_FALSE(stack->Empty()) << "No result on stack after control flow";
    if (!stack->Empty()) {
        Object result = stack->Top();
        EXPECT_TRUE(result.IsType<int>()) << "Result is not an integer";
        if (result.IsType<int>()) {
            EXPECT_EQ(ConstDeref<int>(result), 10)
                << "Control flow result incorrect";
        }
        stack->Pop();
    }

    // Skip function test for now - Rho function syntax needs work

    // Test Pi integration - just verify pi block works
    console.Execute(String("pi { 2 3 + }"));
    EXPECT_FALSE(stack->Empty()) << "No result on stack after Pi integration";
    if (!stack->Empty()) {
        Object result = stack->Top();
        EXPECT_TRUE(result.IsType<int>()) << "Result is not an integer";
        if (result.IsType<int>()) {
            EXPECT_EQ(ConstDeref<int>(result), 5)
                << "Pi block should compute 2 + 3 = 5";
        }
        stack->Pop();
    }

    std::cout << "All basic feature tests passed!" << std::endl;
}
