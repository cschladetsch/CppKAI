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

// HACK to allow printing for tests. see
// http://stackoverflow.com/questions/16491675/how-to-send-custom-message-in-google-c-testing-framework
// namespace testing
//{
// namespace internal
// {
//  enum GTestColor {
//      COLOR_DEFAULT,
//      COLOR_RED,
//      COLOR_GREEN,
//      COLOR_YELLOW
//  };
//
//  extern void ColoredPrintf(GTestColor color, const char* fmt, ...);
// }
//}
//
// #define TEST_PRINTF(COLOR, ...)  do {
// testing::internal::ColoredPrintf(testing::internal::COLOR, "[   KAI    ] ");
// testing::internal::ColoredPrintf(testing::internal::COLOR, __VA_ARGS__); }
// while(0)
//

// Above fix no longer works.
// see
// https://stackoverflow.com/questions/16491675/how-to-send-custom-message-in-google-c-testing-framework
#define GTEST_COUT std::cerr << "[          ]"

// C++ stream interface
class TestCout : public std::stringstream {
   public:
    bool isError_;
    TestCout(bool e = false) : isError_(e) {}
    ~TestCout() {
        if (isError_)
            GTEST_COUT << " [ERROR] " << str().c_str();
        else
            GTEST_COUT << "[INFO] " << str().c_str();
    }
};

#define TEST_COUT TestCout()
#define TEST_CERR TestCout(true)
