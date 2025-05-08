#pragma once

#include <KAI/Console/Console.h>
#include <KAI/KAI.h>
#include <gtest/gtest.h>

#include "./TestCommon.h"

KAI_BEGIN

// Common for tests that work with the various
// languages in the system.
//
// TestLangCommon provides a fully-functional
// working test envionment with an interactive
// console and direct access to the data and
// context stacks for the Executor
class TestLangCommon : public TestCommon {
   public:
    TestLangCommon() = default;

   protected:
    void SetUp() override;
    void TearDown() override;

    void ExecScripts();
    void ExecScriptFile(const std::string &scriptName);

    // Get const ref to data at index on stack
    template <class T>
    const T &AtData(int index) {
        return Deref<T>(_data->At(index));
    }

    // get the _current continuation context
    Continuation const &GetContext() const {
        return ConstDeref<Continuation>(_context->At(0));
    }

    template <class T>
    void AssertResult(const char *text, T const &val) {
        _data->Clear();
        _console.Execute(text);
        ASSERT_EQ(AtData<T>(0), val);
    }

   protected:
    Console _console;
    Stack *_data;
    const Stack *_context;
    Executor *_exec;
};

KAI_END
