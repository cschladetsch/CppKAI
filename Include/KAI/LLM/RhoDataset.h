#pragma once

#include <KAI/Core/Base.h>

#include <filesystem>
#include <string>

KAI_BEGIN
namespace LLM {

struct RhoDatasetOptions {
    std::filesystem::path root;
    std::filesystem::path output_root;
    size_t max_records = 0;
    size_t max_input_chars = 4096;
};

class RhoDatasetBuilder {
   public:
    static std::filesystem::path DefaultOutputRoot();

    static std::filesystem::path Build(const RhoDatasetOptions& options,
                                       std::string* error_out = nullptr);

    static std::filesystem::path Build(std::filesystem::path root,
                                       std::filesystem::path output_root = {},
                                       size_t max_input_chars = 4096,
                                       size_t max_records = 0,
                                       std::string* error_out = nullptr);
};

}  // namespace LLM
KAI_END
