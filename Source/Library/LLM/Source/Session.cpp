#include <KAI/LLM/Session.h>

KAI_BEGIN
namespace LLM {

bool Session::Load(std::string model_name, std::string* error_out) {
    std::string error;
    auto ensured = ModelCache::Ensure(model_name, &error);
    if (!ensured) {
        if (error_out) {
            *error_out = error.empty() ? "Failed to resolve LLM model cache"
                                       : error;
        }
        model_name_.clear();
        model_path_.clear();
        return false;
    }

    model_name_ = std::move(model_name);
    model_path_ = *ensured;
    return true;
}

std::optional<std::string> Session::Prompt(std::string_view prompt,
                                           std::string* error_out) const {
    if (!IsLoaded()) {
        if (error_out) {
            *error_out = "LLM session is not loaded";
        }
        return std::nullopt;
    }

    if (!prompt_handler_) {
        if (error_out) {
            *error_out =
                "No LLM prompt handler configured for this session";
        }
        return std::nullopt;
    }

    return prompt_handler_(*this, prompt, error_out);
}

}  // namespace LLM
KAI_END
