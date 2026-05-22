#include <KAI/Core/Base.h>
#include <KAI/LLM/ModelCache.h>

#include <ModelStore.hpp>

KAI_BEGIN
namespace LLM {

std::string ModelCache::ResolveHome() {
    return deepseek::ModelStore::ResolveModelHome();
}

std::string ModelCache::ResolvePath(std::string_view model_name) {
    return deepseek::ModelStore::ResolveModelPath(model_name);
}

std::optional<std::string> ModelCache::Ensure(std::string_view model_name,
                                              std::string* error_out) {
    return deepseek::ModelStore::EnsureModelDir(model_name, error_out);
}

bool ModelCache::Exists(std::string_view model_name) {
    return deepseek::ModelStore::ModelExists(model_name);
}

}  // namespace LLM
KAI_END
