#pragma once

#include <KAI/Core/Base.h>
#include <KAI/LLM/ModelCache.h>

#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

KAI_BEGIN
namespace llm
{

class Session {
   public:
       using PromptHandler = std::function<std::optional<std::string>(const Session& session, std::string_view prompt,
                                                                      std::string* errorOut)>;

       Session() = default;

       explicit Session(std::string modelName)
       {
           Load(std::move(modelName));
       }

       bool Load(std::string modelName, std::string* errorOut = nullptr);

       [[nodiscard]] bool IsLoaded() const
       {
           return !modelName_.empty() && !modelPath_.empty();
       }
       [[nodiscard]] const std::string& ModelName() const
       {
           return modelName_;
       }
       [[nodiscard]] const std::string& ModelPath() const
       {
           return modelPath_;
       }

    void SetPromptHandler(PromptHandler handler) {
        promptHandler_ = std::move(handler);
    }

    std::optional<std::string> Prompt(std::string_view prompt, std::string* errorOut = nullptr) const;

private:
    std::string modelName_;
    std::string modelPath_;
    PromptHandler promptHandler_;
};

} // namespace llm
KAI_END
