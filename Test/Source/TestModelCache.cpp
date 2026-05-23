#include <KAI/LLM/ModelCache.h>

#include <filesystem>
#include <optional>

#include "TestCommon.h"

#if defined(_WIN32)
#include <cstdlib>
#else
#include <cstdlib>
#endif

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

TEST(TestModelCache, EnsureUsesConfiguredHome) {
    const auto temp_root =
        std::filesystem::temp_directory_path() / "kai-model-cache-test";
    std::error_code ec;
    std::filesystem::remove_all(temp_root, ec);

    ScopedEnvVar override_home("DEEPSEEK_MODEL_HOME", temp_root.string());

    EXPECT_EQ(LLM::ModelCache::ResolveHome(), temp_root.string());
    EXPECT_EQ(LLM::ModelCache::ResolvePath("llama-test"),
              (temp_root / "llama-test").string());

    std::string error;
    auto ensured = LLM::ModelCache::Ensure("llama-test", &error);
    ASSERT_TRUE(ensured.has_value()) << error;
    EXPECT_EQ(*ensured, (temp_root / "llama-test").string());
    EXPECT_TRUE(LLM::ModelCache::Exists("llama-test"));

    std::filesystem::remove_all(temp_root, ec);
}

TEST(TestModelCache, DefaultsToCacheHome) {
    const auto cache_root =
        std::filesystem::temp_directory_path() / "kai-model-cache-home";
    std::error_code ec;
    std::filesystem::remove_all(cache_root, ec);

    ScopedEnvVar unset_override("DEEPSEEK_MODEL_HOME", std::nullopt);
    ScopedEnvVar cache_home("XDG_CACHE_HOME", cache_root.string());

    EXPECT_EQ(LLM::ModelCache::ResolveHome(),
              (cache_root / "deepseek" / "models").string());

    std::filesystem::remove_all(cache_root, ec);
}
