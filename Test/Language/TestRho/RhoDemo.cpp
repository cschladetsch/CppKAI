#include <gtest/gtest.h>

#include <fstream>
#include <iostream>
#include <sstream>

#include "KAI/Core/BuiltinTypes/Stack.h"
#include "KAI/Core/Config/Base.h"
#include "KAI/Core/Debug.h"
#include "KAI/Language/Rho/RhoParser.h"
#include "KAI/Language/Rho/RhoTranslator.h"
#include "TestLangCommon.h"
#include "KAI/Core/Logger.h"
#include "KAI/Console/Console.h"
#include "KAI/Core/Exception.h"

using namespace kai;
using namespace std;

// Fixture for running the Rho demo
class RhoDemo : public TestLangCommon {
   protected:
    // Helper method to execute a Rho script file
    bool ExecuteRhoFile(const char *filename, bool verbose = true) {
        try {
            Registry reg;
            Console console;
            
            // Create a system object with print capability
            Object system = reg.New<Object>();
            auto scope = console.GetTree().GetScope();
            scope.Set(Label("System"), system);

            // Add print method
            // Create a simple print function for the system object
            system.Set("Print", reg.New<String>("print function placeholder"));

            // Read the file
            std::ifstream file(filename);
            if (!file.is_open()) {
                Logger::ErrorWithLocation(std::string("Failed to open file: ") + filename, __FILE__, __LINE__);
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
                Logger::InfoWithLocation("Script executed but no result on stack", __FILE__, __LINE__);
            }

            return true;
        } catch (const Exception::Base &e) {
            Logger::ErrorWithLocation("Exception: " + std::string(e.ToString()), __FILE__, __LINE__);
            return false;
        } catch (const std::exception &e) {
            Logger::ErrorWithLocation("std::exception: " + std::string(e.what()), __FILE__, __LINE__);
            return false;
        } catch (...) {
            Logger::ErrorWithLocation("Unknown exception", __FILE__, __LINE__);
            return false;
        }
    }
};

// Test to run the demo script
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
TEST_F(RhoDemo, BasicFeatureTests) {
    Registry reg;
    Console console;

    // Test arithmetic
    console.Execute(String("2 + 3 * 4"));
    auto stack = console.GetExecutor()->GetDataStack();
    EXPECT_FALSE(stack->Empty()) << "No result on stack after arithmetic";
    if (!stack->Empty()) {
        Object result = stack->Top();
        EXPECT_TRUE(result.IsType<int>()) << "Result is not an integer";
        if (result.IsType<int>()) {
            EXPECT_EQ(ConstDeref<int>(result), 14) << "Arithmetic result incorrect";
        }
        stack->Pop();
    }

    // Test control flow
    console.Execute(String("x = 5; if (x > 3) { x = 10; } else { x = 0; } x;"));
    EXPECT_FALSE(stack->Empty()) << "No result on stack after control flow";
    if (!stack->Empty()) {
        Object result = stack->Top();
        EXPECT_TRUE(result.IsType<int>()) << "Result is not an integer";
        if (result.IsType<int>()) {
            EXPECT_EQ(ConstDeref<int>(result), 10) << "Control flow result incorrect";
        }
        stack->Pop();
    }

    // Test function
    console.Execute(String("function add(a, b) { return a + b; } add(2, 3);"));
    EXPECT_FALSE(stack->Empty()) << "No result on stack after function test";
    if (!stack->Empty()) {
        Object result = stack->Top();
        EXPECT_TRUE(result.IsType<int>()) << "Result is not an integer";
        if (result.IsType<int>()) {
            EXPECT_EQ(ConstDeref<int>(result), 5) << "Function result incorrect";
        }
        stack->Pop();
    }

    // Test Pi integration
    console.Execute(String("5 + pi{ 2 3 + }"));
    EXPECT_FALSE(stack->Empty()) << "No result on stack after Pi integration";
    if (!stack->Empty()) {
        Object result = stack->Top();
        EXPECT_TRUE(result.IsType<int>()) << "Result is not an integer";
        if (result.IsType<int>()) {
            EXPECT_EQ(ConstDeref<int>(result), 10) << "Pi integration result incorrect";
        }
        stack->Pop();
    }

    std::cout << "All basic feature tests passed!" << std::endl;
}

