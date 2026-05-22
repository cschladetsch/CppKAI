#include "TestCommon.h"

#include <KAI/LLM/Session.h>

#include <filesystem>
#include <optional>

using namespace kai;
using namespace std;

namespace {

class ScopedEnvVar {
   public:
    ScopedEnvVar(const char* key, std::optional<std::string> value)
        : key_(key) {
        const char* current = std::getenv(key_.c_str());
        if (current) {
            previous_ = current;
        }
        if (value) {
            Set(*value);
        } else {
            Unset();
        }
    }

    ~ScopedEnvVar() {
        if (previous_) {
            Set(*previous_);
        } else {
            Unset();
        }
    }

   private:
    void Set(const std::string& value) {
#if defined(_WIN32)
        _putenv_s(key_.c_str(), value.c_str());
#else
        setenv(key_.c_str(), value.c_str(), 1);
#endif
    }

    void Unset() {
#if defined(_WIN32)
        _putenv_s(key_.c_str(), "");
#else
        unsetenv(key_.c_str());
#endif
    }

    std::string key_;
    std::optional<std::string> previous_;
};

}  // namespace

TEST(TestLLMSession, LoadResolvesCachePath) {
    const auto cache_root =
        std::filesystem::temp_directory_path() / "kai-llm-session";
    std::error_code ec;
    std::filesystem::remove_all(cache_root, ec);

    ScopedEnvVar cache_home("XDG_CACHE_HOME", cache_root.string());
    ScopedEnvVar unset_override("DEEPSEEK_MODEL_HOME", std::nullopt);

    LLM::Session session;
    std::string error;
    ASSERT_TRUE(session.Load("llama-test", &error)) << error;
    EXPECT_TRUE(session.IsLoaded());
    EXPECT_EQ(session.ModelName(), "llama-test");
    EXPECT_EQ(session.ModelPath(),
              (cache_root / "deepseek" / "models" / "llama-test").string());

    std::filesystem::remove_all(cache_root, ec);
}

TEST(TestLLMSession, PromptUsesInjectedHandler) {
    const auto cache_root =
        std::filesystem::temp_directory_path() / "kai-llm-session-prompt";
    std::error_code ec;
    std::filesystem::remove_all(cache_root, ec);

    ScopedEnvVar cache_home("XDG_CACHE_HOME", cache_root.string());
    ScopedEnvVar unset_override("DEEPSEEK_MODEL_HOME", std::nullopt);

    LLM::Session session;
    std::string error;
    ASSERT_TRUE(session.Load("llama-test", &error)) << error;

    session.SetPromptHandler(
        [](const LLM::Session& s, std::string_view prompt, std::string*) {
            return std::string(s.ModelName() + ": " + std::string(prompt));
        });

    auto reply = session.Prompt("hello", &error);
    ASSERT_TRUE(reply.has_value()) << error;
    EXPECT_EQ(*reply, "llama-test: hello");

    std::filesystem::remove_all(cache_root, ec);
}
