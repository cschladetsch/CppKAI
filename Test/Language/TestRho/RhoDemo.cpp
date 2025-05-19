#include <fstream>
#include <sstream>
#include <iostream>

#include "KAI/Core/Config/Base.h"
#include "KAI/Core/Debug.h"
#include <gtest/gtest.h>
#include "KAI/Language/Rho/RhoParser.h"
#include "KAI/Language/Rho/RhoTranslator.h"
#include "KAI/Core/BuiltinTypes/Stack.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Fixture for running the Rho demo
class RhoDemo : public TestLangCommon
{
protected:
    // Helper method to execute a Rho script file
    bool ExecuteRhoFile(const char *filename, bool verbose = true)
    {
        try
        {
            Registry reg;
            Console console(reg);
            console.SetScope(reg.GetGlobalScope());
            
            // Create a system object with print capability
            Object system = reg.New("Object");
            reg.GetGlobalScope().Set("System", system);
            
            // Add print method
            Function print = reg.NewFunction([&console, verbose](String const &text) {
                if (verbose)
                {
                    std::cout << text << std::endl;
                }
                return Value();
            });
            system.Set("Print", print);
            
            // Read the file
            std::ifstream file(filename);
            if (!file.is_open())
            {
                KAI_LOG_ERROR(std::string("Failed to open file: ") + filename);
                return false;
            }
            
            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string script = buffer.str();
            
            // Execute the script
            auto result = console.Execute(script);
            
            if (result.Failed)
            {
                KAI_LOG_ERROR("Execution failed: " + result.Error);
                return false;
            }
            
            return true;
        }
        catch (const Exception &e)
        {
            KAI_LOG_ERROR("Exception: " + std::string(e.ToString()));
            return false;
        }
        catch (const std::exception &e)
        {
            KAI_LOG_ERROR("std::exception: " + std::string(e.what()));
            return false;
        }
        catch (...)
        {
            KAI_LOG_ERROR("Unknown exception");
            return false;
        }
    }
};

// Test to run the demo script
TEST_F(RhoDemo, RunDemo)
{
    std::cout << "\n\n=======================================" << std::endl;
    std::cout << "          RHO LANGUAGE DEMO           " << std::endl;
    std::cout << "=======================================" << std::endl;
    
    bool success = ExecuteRhoFile("Test/Language/TestRho/Scripts/Demo.rho", true);
    
    std::cout << "=======================================" << std::endl;
    
    EXPECT_TRUE(success) << "Demo script execution failed";
    
    if (success)
    {
        std::cout << "Demo completed successfully!" << std::endl;
    }
    else
    {
        std::cout << "Demo failed. See error logs for details." << std::endl;
    }
}

// Simple test for each major feature to ensure they work separately
TEST_F(RhoDemo, BasicFeatureTests)
{
    Registry reg;
    Console console(reg);
    console.SetScope(reg.GetGlobalScope());
    
    // Test arithmetic
    auto result1 = console.Execute("2 + 3 * 4");
    EXPECT_FALSE(result1.Failed) << "Basic arithmetic failed: " << result1.Error;
    if (!result1.Failed)
    {
        EXPECT_EQ(kai_cast<int>(result1.Value), 14) << "Arithmetic result incorrect";
    }
    
    // Test control flow
    auto result2 = console.Execute("x = 5; if (x > 3) { x = 10; } else { x = 0; } x;");
    EXPECT_FALSE(result2.Failed) << "Control flow failed: " << result2.Error;
    if (!result2.Failed)
    {
        EXPECT_EQ(kai_cast<int>(result2.Value), 10) << "Control flow result incorrect";
    }
    
    // Test function
    auto result3 = console.Execute("function add(a, b) { return a + b; } add(2, 3);");
    EXPECT_FALSE(result3.Failed) << "Function test failed: " << result3.Error;
    if (!result3.Failed)
    {
        EXPECT_EQ(kai_cast<int>(result3.Value), 5) << "Function result incorrect";
    }
    
    // Test Pi integration
    auto result4 = console.Execute("5 + pi{ 2 3 + }");
    EXPECT_FALSE(result4.Failed) << "Pi integration failed: " << result4.Error;
    if (!result4.Failed)
    {
        EXPECT_EQ(kai_cast<int>(result4.Value), 10) << "Pi integration result incorrect";
    }
    
    std::cout << "All basic feature tests passed!" << std::endl;
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}