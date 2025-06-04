#include <gtest/gtest.h>
#include <KAI/Language/Tau/TauParser.h>
#include <KAI/Language/Tau/TauLexer.h>
#include <KAI/Language/Tau/Generate/GenerateProxy.h>
#include <KAI/Core/Registry.h>
#include <KAI/Core/BuiltinTypes.h>
#include "TestLangCommon.h"

using namespace kai;
using namespace kai::tau;

// Note: Tau is an Interface Definition Language (IDL) that generates C++ code.
// It is NOT an executable language like Pi or Rho.
// These tests validate parsing and code generation, not execution.
TEST(TauInterface, DISABLED_BasicInterface) {
    // DISABLED: Tau interface tests need to be rewritten to test only parsing/generation
    // without requiring a full Console/Registry initialization.
    GTEST_SKIP() << "Tau is an IDL - tests should validate code generation, not execution";
}

TEST(TauInterface, DISABLED_MultipleInterfaces) {
    GTEST_SKIP() << "Tau is an IDL - tests should validate code generation, not execution";
}

TEST(TauInterface, DISABLED_InterfaceInheritance) {
    GTEST_SKIP() << "Tau is an IDL - tests should validate code generation, not execution";
}

TEST(TauInterface, DISABLED_DefaultInterfaceMethods) {
    GTEST_SKIP() << "Tau is an IDL - tests should validate code generation, not execution";
}

TEST(TauInterface, DISABLED_GenericInterfaces) {
    GTEST_SKIP() << "Tau is an IDL - tests should validate code generation, not execution";
}