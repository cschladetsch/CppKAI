#include <KAI/LLM/Session.h>

KAI_BEGIN
namespace llm
{

bool Session::Load(std::string modelName, std::string* errorOut)
{
    std::string error;
    auto ensured = ModelCache::Ensure(modelName, &error);
    if (!ensured) {
        if (errorOut != nullptr) {
            *errorOut = error.empty() ? "Failed to resolve LLM model cache" : error;
        }
        modelName_.clear();
        modelPath_.clear();
        return false;
    }

    modelName_ = std::move(modelName);
    modelPath_ = *ensured;
    return true;
}

std::optional<std::string> Session::Prompt(std::string_view prompt, std::string* errorOut) const
{
    if (!IsLoaded()) {
        if (errorOut != nullptr) {
            *errorOut = "LLM session is not loaded";
        }
        return std::nullopt;
    }

    if (!promptHandler_) {
        if (errorOut != nullptr) {
            *errorOut = "No LLM prompt handler configured for this session";
        }
        return std::nullopt;
    }

    return promptHandler_(*this, prompt, errorOut);
}

} // namespace llm
KAI_END
