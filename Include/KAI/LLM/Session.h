#pragma once

#include <KAI/Core/Base.h>
#include <KAI/LLM/ModelCache.h>

#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

KAI_BEGIN
namespace LLM {

class Session {
   public:
    using PromptHandler = std::function<std::optional<std::string>(
        const Session& session, std::string_view prompt,
        std::string* error_out)>;

    Session() = default;

    explicit Session(std::string model_name) { Load(std::move(model_name)); }

    bool Load(std::string model_name, std::string* error_out = nullptr);

    bool IsLoaded() const {
        return !model_name_.empty() && !model_path_.empty();
    }
    const std::string& ModelName() const { return model_name_; }
    const std::string& ModelPath() const { return model_path_; }

    void SetPromptHandler(PromptHandler handler) {
        prompt_handler_ = std::move(handler);
    }

    std::optional<std::string> Prompt(std::string_view prompt,
                                      std::string* error_out = nullptr) const;

   private:
    std::string model_name_;
    std::string model_path_;
    PromptHandler prompt_handler_;
};

}  // namespace LLM
KAI_END
