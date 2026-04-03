#pragma once

#include <gtest/gtest.h>

#include "KAI/Console/Console.h"
#include "KAI/Core/BuiltinTypes/Stack.h"
#include "KAI/Core/Config/Base.h"
#include "KAI/Core/Debug.h"
#include "KAI/Core/Logger.h"
#include "TestLangCommon.h"
#include "TestRhoUtils.h"

// Base class for Rho tests that provides utilities for script execution
struct RhoTestBase : kai::TestLangCommon {
   protected:
    template <class T>
    void RunScriptAndExpect(const std::string &scriptFile, T expected,
                            bool verbose = false) {
        if (verbose) {
            KAI_LOG_INFO(std::string("Testing script: ") + scriptFile);
        }

        try {
            kai::Console console;
            console.SetLanguage(kai::Language::Rho);

            // Load and execute script
            std::string script = kai::test::LoadRhoScript(scriptFile);
            console.Execute(script.c_str(), kai::Structure::Program);

            // Get the result from the data stack after execution
            auto executor = console.GetExecutor();
            auto dataStack = executor->GetDataStack();

            if (dataStack->Empty()) {
                FAIL() << "No result on stack after script execution";
                return;
            }

            auto val = dataStack->Top();
            if (!val.IsType<T>()) {
                std::string expectedTypeName = typeid(T).name();
                std::string actualTypeName =
                    val.GetClass()
                        ? std::string(
                              val.GetClass()->GetName().ToString().c_str())
                        : "unknown";
                KAI_LOG_ERROR("Type mismatch. Expected: " + expectedTypeName +
                              ", Got: " + actualTypeName);
                FAIL() << "Type mismatch. Expected: " << expectedTypeName
                       << ", Got: " << actualTypeName;
                return;
            }

            T actual = kai::ConstDeref<T>(val);
            if (verbose) {
                // Convert result to string for logging (handle different types)
                std::string resultStr;
                if constexpr (std::is_same_v<T, kai::String>) {
                    resultStr = actual.StdString();
                } else if constexpr (std::is_arithmetic_v<T>) {
                    resultStr = std::to_string(actual);
                } else {
                    resultStr = "(complex type)";
                }
                KAI_LOG_INFO("Result: " + resultStr);
            }
            ASSERT_EQ(expected, actual)
                << "Result doesn't match expected value";
        } catch (const kai::Exception::Base &e) {
            KAI_LOG_ERROR("Exception: " + std::string(e.ToString()));
            FAIL() << "Exception: " << e.ToString();
        } catch (const std::exception &e) {
            KAI_LOG_ERROR("std::exception: " + std::string(e.what()));
            FAIL() << "std::exception: " << e.what();
        } catch (...) {
            KAI_LOG_ERROR("Unknown exception");
            FAIL() << "Unknown exception";
        }
    }

    template <class T>
    void RunCodeAndExpect(const char *code, T expected, bool verbose = false) {
        if (verbose) {
            KAI_LOG_INFO(std::string("Testing code: ") + code);
        }

        try {
            kai::Console console;
            console.SetLanguage(kai::Language::Rho);

            // Execute code directly
            console.Execute(code, kai::Structure::Program);

            // Get the result from the data stack after execution
            auto executor = console.GetExecutor();
            auto dataStack = executor->GetDataStack();

            if (dataStack->Empty()) {
                FAIL() << "No result on stack after code execution";
                return;
            }

            auto val = dataStack->Top();
            if (!val.IsType<T>()) {
                std::string expectedTypeName = typeid(T).name();
                std::string actualTypeName =
                    val.GetClass()
                        ? std::string(
                              val.GetClass()->GetName().ToString().c_str())
                        : "unknown";
                FAIL() << "Type mismatch. Expected: " << expectedTypeName
                       << ", Got: " << actualTypeName;
                return;
            }

            T actual = kai::ConstDeref<T>(val);
            ASSERT_EQ(expected, actual)
                << "Result doesn't match expected value";
        } catch (const kai::Exception::Base &e) {
            FAIL() << "Exception: " << e.ToString();
        } catch (const std::exception &e) {
            FAIL() << "std::exception: " << e.what();
        } catch (...) {
            FAIL() << "Unknown exception";
        }
    }
};