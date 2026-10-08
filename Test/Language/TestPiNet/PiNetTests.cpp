// PiNet: a continuation may only be sent if every name it uses is bound
// inside it ('x #, or a function's parameters), or is a system name every
// node has (/Bin, /Sys, /Types). Values from the sender come in on the stack.

#include <gtest/gtest.h>

#include <algorithm>

#include "KAI/Core/BinaryStream.h"
#include "KAI/Core/FunctionBase.h"
#include "KAI/Executor/BinBase.h"
#include "KAI/Language/PiNet/PiNet.h"
#include "KAI/Language/Sigma/SigmaTranslator.h"
#include "TestLangCommon.h"

using namespace kai;

struct PiNetTests : TestLangCommon {
    // Run Pi and return what it left on top of the stack.
    Object Pi(const char *code) {
        console_.SetLanguage(Language::Pi);
        data_->Clear();
        console_.Execute(code, Structure::Program);
        EXPECT_FALSE(data_->Empty()) << code;
        return data_->Empty() ? Object() : data_->Top();
    }

    PiNet::Report Check(const char *code, bool withTree = true) {
        return PiNet::Check(Pi(code), withTree ? &console_.GetTree() : nullptr);
    }

    static bool Has(const std::vector<std::string> &names, const std::string &name) {
        return std::find(names.begin(), names.end(), name) != names.end();
    }

    // Call the console's /Bin/send function as the executor does, and
    // return the message it fails with, or "" if it doesn't throw.
    std::string SendError(Object payload) {
        Object send = console_.GetTree().GetRoot().Get(Label("Bin")).Get(Label("send"));
        Stack &stack = *data_;
        stack.Clear();
        stack.Push(payload);
        stack.Push(reg_->New<int>(0));  // peer 0
        try {
            Deref<BasePointer<FunctionBase>>(send)->Invoke(*reg_, stack);
        } catch (const exception::Base &e) {
            return e.ToString();
        } catch (const std::exception &e) {
            return e.what();
        }
        return "";
    }

    void UsePiNetForSend() {
        console_.AddSendCheck([this](Object payload) { PiNet::Require(payload, &console_.GetTree()); });
    }
};

// --- What passes -----------------------------------------------------------------

TEST_F(PiNetTests, ClosedArithmetic) { EXPECT_TRUE(Check("{ 1 2 + }").Ok()); }

TEST_F(PiNetTests, InputsComeOnTheStack) { EXPECT_TRUE(Check("{ dup * 1 + }").Ok()); }

TEST_F(PiNetTests, NamesBoundInThePayload) { EXPECT_TRUE(Check("{ 'a # a a * }").Ok()); }

TEST_F(PiNetTests, NestedBlockSeesTheEnclosingBinding) {
    EXPECT_TRUE(Check("{ 'n # { n 1 + } & }").Ok());
}

TEST_F(PiNetTests, QuotedNameAsDataIsNotAUse) { EXPECT_TRUE(Check("{ 'peer }").Ok()); }

TEST_F(PiNetTests, SystemNamesNeedATree) {
    // `send` lives in /Bin on every node.
    EXPECT_TRUE(Check("{ 1 0 send }").Ok());
    auto without = Check("{ 1 0 send }", /*withTree*/ false);
    EXPECT_TRUE(Has(without.unbound, "send"));
}

TEST_F(PiNetTests, PlainDataPasses) {
    EXPECT_TRUE(PiNet::Check(Pi("42"), &console_.GetTree()).Ok());
    EXPECT_TRUE(PiNet::Check(Pi("\"text\""), &console_.GetTree()).Ok());
}

// --- What fails ------------------------------------------------------------------

TEST_F(PiNetTests, UnboundName) {
    auto r = Check("{ a + }");
    EXPECT_FALSE(r.Ok());
    EXPECT_EQ(r.unbound, std::vector<std::string>{"a"});
}

TEST_F(PiNetTests, EachUnboundNameOnce) {
    auto r = Check("{ a b + a * }");
    EXPECT_EQ(r.unbound, (std::vector<std::string>{"a", "b"}));
}

TEST_F(PiNetTests, InnerBindingIsNotVisibleOutside) {
    auto r = Check("{ { 'n # } & n }");
    EXPECT_TRUE(Has(r.unbound, "n"));
}

TEST_F(PiNetTests, SendersVariablesAreExternal) {
    // x resolves on this node, which is exactly why it must not travel.
    console_.SetLanguage(Language::Pi);
    console_.Execute("42 'x #", Structure::Program);
    auto r = Check("{ x 1 + }");
    EXPECT_TRUE(Has(r.unbound, "x"));
}

TEST_F(PiNetTests, RetrieveIsAUse) { EXPECT_TRUE(Has(Check("{ 'a @ }").unbound, "a")); }

TEST_F(PiNetTests, OperationsThatReachIntoTheNode) {
    auto r = Check("{ self }");
    EXPECT_FALSE(r.Ok());
    EXPECT_EQ(r.environment.size(), 1u);
}

TEST_F(PiNetTests, AbsolutePathIntoTheSendersTree) {
    // Pi has no absolute-path literal (`/Home/x` is division), so build the
    // continuation directly, as a translator emitting a Pathname would.
    auto code = [this](const char *path) {
        Pointer<Array> items = reg_->New<Array>();
        items->Append(reg_->New(Pathname(path)));
        Pointer<Continuation> cont = reg_->New<Continuation>();
        cont->SetCode(items);
        return PiNet::Check(cont, &console_.GetTree());
    };
    auto home = code("/Home/x");
    EXPECT_TRUE(Has(home.environment, "/Home/x")) << home.ToString();
    EXPECT_TRUE(code("/Bin/send").Ok());
}

// --- Payloads ----------------------------------------------------------------------

TEST_F(PiNetTests, ArraysAreCheckedElementByElement) {
    auto r = Check("{ 1 } { a } 2 toarray");
    EXPECT_TRUE(Has(r.unbound, "a"));
}

TEST_F(PiNetTests, FrozenPayloadsAreThawedToCheck) {
    Object frozen = Pi("{ a + } freeze");
    ASSERT_TRUE(frozen.IsType<BinaryStream>());
    EXPECT_TRUE(Has(PiNet::Check(frozen, &console_.GetTree()).unbound, "a"));
    // The check rewinds the stream, so it still thaws to the continuation.
    EXPECT_TRUE(Bin::Thaw(frozen).IsType<Continuation>());
}

TEST_F(PiNetTests, RequireNamesTheProblem) {
    try {
        PiNet::Require(Pi("{ a + }"), &console_.GetTree());
        FAIL() << "Require should throw";
    } catch (const exception::Base &e) {
        const std::string text = e.ToString();
        EXPECT_NE(text.find("PiNet: not transportable"), std::string::npos) << text;
        EXPECT_NE(text.find("'a'"), std::string::npos) << text;
    }
}

// --- send ---------------------------------------------------------------------------

TEST_F(PiNetTests, SendRefusesBeforeTheNetworkIsChecked) {
    UsePiNetForSend();
    const std::string error = SendError(Pi("{ a + }"));
    EXPECT_NE(error.find("PiNet"), std::string::npos) << error;
}

TEST_F(PiNetTests, ClosedPayloadGetsAsFarAsTheNetwork) {
    UsePiNetForSend();
    const std::string error = SendError(Pi("{ 1 2 + }"));
    EXPECT_EQ(error.find("PiNet"), std::string::npos) << error;
    EXPECT_NE(error.find("Network not enabled"), std::string::npos) << error;
}

// --- Higher-level languages -----------------------------------------------------------

TEST_F(PiNetTests, FunctionParametersAreBound) {
    // A Rho or Sigma function's parameters are its continuation's arguments.
    SigmaTranslator sigma(*reg_);
    ASSERT_TRUE(sigma.Compile("fun twice(x: int) -> int\n    return x * 2\nfun addY(x: int) -> int\n    return x + y\ny = 1\n"))
        << sigma.GetErrors().front();
    console_.SetLanguage(Language::Rho);
    console_.Execute(sigma.GetRho().c_str(), Structure::Program);
    auto exec = console_.GetExecutor();
    EXPECT_TRUE(PiNet::Check(exec->Resolve(Label("twice")), &console_.GetTree()).Ok());
    // addY reads the global y, which stays behind on the sender.
    EXPECT_TRUE(Has(PiNet::Check(exec->Resolve(Label("addY")), &console_.GetTree()).unbound, "y"));
}
