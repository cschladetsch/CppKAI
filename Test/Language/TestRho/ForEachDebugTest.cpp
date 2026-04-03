#include <gtest/gtest.h>

#include <iostream>

#include "TestLangCommon.h"

using namespace kai;

struct ForEachDebugTest : TestLangCommon {};

TEST_F(ForEachDebugTest, SimpleForEach) {
    console_.SetLanguage(Language::Rho);

    // Very simple foreach test
    const char* code = R"(
foreach x in [1, 2, 3]
    print(x)
)";

    std::cout << "Executing foreach test code:\n" << code << std::endl;

    try {
        console_.Execute(code, Structure::Program);

        if (data_->Empty()) {
            std::cout << "Stack is empty after execution" << std::endl;
        } else {
            std::cout << "Stack has " << data_->Size() << " items" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
}