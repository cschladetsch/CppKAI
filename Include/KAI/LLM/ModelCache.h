#pragma once

#include <KAI/Core/Base.h>

#include <optional>
#include <string>
#include <string_view>

KAI_BEGIN
namespace LLM {

class ModelCache {
 public:
  static std::string ResolveHome();
  static std::string ResolvePath(std::string_view model_name);

  static std::optional<std::string> Ensure(std::string_view model_name,
                                           std::string* error_out = nullptr);
  static bool Exists(std::string_view model_name);
};

}  // namespace LLM
KAI_END
