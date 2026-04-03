#pragma once

#include <gtest/gtest.h>

#include <boost/filesystem.hpp>

#include "KAI/Core/BuiltinTypes.h"
#include "KAI/Core/Console.h"
#include "KAI/Core/Detail/Function.h"
#include "KAI/Core/Executor.h"
#include "KAI/Core/FunctionBase.h"
#include "KAI/Core/Object/Class.h"
#include "KAI/Core/Object/GetStorageBase.h"
#include "KAI/Core/StringStreamTraits.h"
#include "KAI/Language/Common/Language.h"
#include "rang.hpp"

KAI_BEGIN

extern boost::filesystem::path ScriptRoot;

std::string LoadScriptText(const char *filename);

// TODO: Rename to BaseTestClass once refactoring works with VSCode again :P
class TestCommon : public ::testing::Test {
   protected:
    Registry *reg_ = nullptr;
    Tree *tree_ = nullptr;
    Object root_;

    ~TestCommon() {}
    Registry &Reg() const;
    Tree &GetTree() const;
    Object Root() const;

    void SetUp() override;
    void TearDown() override;

    virtual void AddRequiredClasses() {}
};

KAI_END

// Above fix no longer works.
// see
// https://stackoverflow.com/questions/16491675/how-to-send-custom-message-in-google-c-testing-framework
/**
 * @brief Custom Google Test output stream with grey formatting
 *
 * This defines a custom output stream for Google Test that
 * uses grey text for the console metadata/prefix.
 */
#define GTEST_COUT \
    std::cerr << rang::fg::gray << "[          ]" << rang::fg::reset

// Forward declaration of color output function from Main.cpp
bool IsColorOutputEnabled();

/**
 * @brief Enhanced C++ stream interface for test output with color support
 *
 * This class provides:
 * - Color-coded INFO (green) and ERROR (red bold) messages
 * - Integration with the global color setting
 * - Support for the standard C++ stream operators
 *
 * Usage:
 *   TEST_COUT << "This is information"; // Green INFO message
 *   TEST_CERR << "This is an error";    // Red ERROR message
 */
class TestCout : public std::stringstream {
   public:
    bool isError_;
    TestCout(bool e = false) : isError_(e) {}
    ~TestCout() {
        if (IsColorOutputEnabled()) {
            // Use colored output
            if (isError_) {
                GTEST_COUT << " " << rang::fg::red << rang::style::bold
                           << "[ERROR]" << rang::style::reset << rang::fg::reset
                           << " " << str().c_str();
            } else {
                GTEST_COUT << " " << rang::fg::green << "[INFO]"
                           << rang::fg::reset << " " << str().c_str();
            }
        } else {
            // Use plain output
            if (isError_)
                GTEST_COUT << " [ERROR] " << str().c_str();
            else
                GTEST_COUT << " [INFO] " << str().c_str();
        }
    }
};

#define TEST_COUT TestCout()
#define TEST_CERR TestCout(true)
