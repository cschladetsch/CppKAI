#pragma once

#include <KAI/Core/Base.h>

#include <optional>
#include <string>
#include <string_view>

KAI_BEGIN
namespace llm
{

class ModelCache {
   public:
    static std::string ResolveHome();
    static std::string ResolvePath(std::string_view modelName);

    static std::optional<std::string> Ensure(std::string_view modelName, std::string* errorOut = nullptr);
    static bool Exists(std::string_view modelName);
};

} // namespace llm
KAI_END
