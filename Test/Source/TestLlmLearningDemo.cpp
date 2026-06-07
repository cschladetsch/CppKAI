#include <KAI/LLM/Session.h>
#include <gtest/gtest.h>
#include <string>
#include <string_view>
#include <optional>

using namespace kai;

// Simulate different LLM capability levels
std::string BaselineModelHandler(const LLM::Session&, std::string_view prompt, std::string*) {
    // Baseline model: fails translation
    return "Error: Cannot translate Rho snippet: " + std::string(prompt);
}

std::string TrainedModelHandler(const LLM::Session&, std::string_view prompt, std::string*) {
    // "Learned" model: successfully performs specific translation
    if (prompt == "x = 1 + 2;") {
        return "push 1, push 2, add, set x";
    }
    return "Unknown snippet";
}

TEST(TestLlmLearningDemo, DemonstratesPerformanceImprovement) {
    LLM::Session session("kai-rho-model");
    
    // Task: Rho to Pi translation
    std::string prompt = "x = 1 + 2;";
    std::string expected_pi = "push 1, push 2, add, set x";
    std::string error;

    // 1. Evaluate Baseline Performance
    session.SetPromptHandler(BaselineModelHandler);
    auto baseline_reply = session.Prompt(prompt, &error);
    ASSERT_TRUE(baseline_reply.has_value());
    EXPECT_NE(*baseline_reply, expected_pi) << "Baseline model should fail";

    // 2. Evaluate "Learned" Performance
    session.SetPromptHandler(TrainedModelHandler);
    auto trained_reply = session.Prompt(prompt, &error);
    ASSERT_TRUE(trained_reply.has_value());
    EXPECT_EQ(*trained_reply, expected_pi) << "Trained model should succeed";
}
