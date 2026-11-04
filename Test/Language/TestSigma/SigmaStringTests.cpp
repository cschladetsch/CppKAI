// Sigma string literals. Rho has no escape sequences, so a literal holds a
// double quote or an apostrophe but not both; Sigma picks the quote character
// that is free and rejects text that needs the pair.

#include <gtest/gtest.h>

#include "KAI/Language/Sigma/SigmaTranslator.h"
#include "TestLangCommon.h"

using namespace kai;

struct SigmaStringTests : TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(Language::Sigma);
    }

    std::string Errors(const std::string &code) {
        SigmaTranslator sigma(*reg_);
        sigma.Compile(code.c_str());
        std::string all;
        for (auto const &e : sigma.GetErrors()) all += e + "\n";
        return all;
    }

    std::string Rho(const std::string &code) {
        SigmaTranslator sigma(*reg_);
        EXPECT_TRUE(sigma.Compile(code.c_str())) << Errors(code);
        return sigma.GetRho();
    }

    template <class T>
    T Run(const std::string &code) {
        data_->Clear();
        console_.Execute(code.c_str(), Structure::Program);
        EXPECT_FALSE(data_->Empty()) << "no result for:\n" << code << "\n--- rho:\n" << Rho(code);
        if (data_->Empty()) return T();
        Object top = data_->Top();
        EXPECT_TRUE(top.IsType<T>()) << "wrong result type for:\n" << code;
        return top.IsType<T>() ? ConstDeref<T>(top) : T();
    }
};

TEST_F(SigmaStringTests, DoubleQuotesInsideSingleQuotes) {
    // 's', 'a', 'y', ' ', '"', 'h', 'i', '"'
    EXPECT_EQ(Run<int>("s = 'say \"hi\"'\ns.size()"), 8);
}

TEST_F(SigmaStringTests, DoubleQuotedTextKeepsItsValue) {
    EXPECT_EQ(Run<String>("s = 'say \"hi\"'\ns"), String("say \"hi\""));
}

TEST_F(SigmaStringTests, EscapedApostropheInsideSingleQuotes) {
    EXPECT_EQ(Run<String>("s = 'it\\'s'\ns"), String("it's"));
}

TEST_F(SigmaStringTests, TheOtherQuoteIsUsedInGeneratedRho) {
    EXPECT_EQ(Rho("s = 'say \"hi\"'"), "s = 'say \"hi\"'\n");
    EXPECT_EQ(Rho("s = 'plain'"), "s = \"plain\"\n");
    EXPECT_EQ(Rho("s = \"it's\""), "s = \"it's\"\n");
}

TEST_F(SigmaStringTests, TextNeedingBothQuotesIsRejected) {
    const std::string errors = Errors("s = 'a\\'b\"c'");
    EXPECT_NE(errors.find("a string cannot contain both"), std::string::npos) << errors;
}
