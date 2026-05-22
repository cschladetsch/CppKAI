#include <KAI/Core/Base.h>
#include <KAI/LLM/ModelCache.h>

#include <ModelStore.hpp>

#include <cstdlib>

KAI_BEGIN
namespace LLM {

namespace {

std::string ComputeDefaultHome() {
    if (const char* override_dir = std::getenv("DEEPSEEK_MODEL_HOME");
        override_dir && *override_dir) {
        return override_dir;
    }

    const char* xdg_cache_home = std::getenv("XDG_CACHE_HOME");
    const char* home = std::getenv("HOME");
    std::string base = (xdg_cache_home && *xdg_cache_home)
                           ? xdg_cache_home
                           : (home ? std::string(home) + "/.cache" : ".");
    return base + "/deepseek/models";
}

void EnsureDefaultEnv() {
    const char* override_dir = std::getenv("DEEPSEEK_MODEL_HOME");
    if (override_dir && *override_dir) {
        return;
    }
    const std::string home = ComputeDefaultHome();
    setenv("DEEPSEEK_MODEL_HOME", home.c_str(), 1);
}

}  // namespace

std::string ModelCache::ResolveHome() {
    return ComputeDefaultHome();
}

std::string ModelCache::ResolvePath(std::string_view model_name) {
    EnsureDefaultEnv();
    return deepseek::ModelStore::ResolveModelPath(model_name);
}

std::optional<std::string> ModelCache::Ensure(std::string_view model_name,
                                              std::string* error_out) {
    EnsureDefaultEnv();
    return deepseek::ModelStore::EnsureModelDir(model_name, error_out);
}

bool ModelCache::Exists(std::string_view model_name) {
    EnsureDefaultEnv();
    return deepseek::ModelStore::ModelExists(model_name);
}

}  // namespace LLM
KAI_END
