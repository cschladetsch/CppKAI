#pragma once

#include <KAI/Core/Base.h>

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

KAI_BEGIN
namespace LLM {

struct RepoIndexOptions {
    std::filesystem::path root;
    std::filesystem::path output_root;
    size_t chunk_lines = 200;
    bool include_ext = false;
};

class RepoIndexer {
   public:
    static std::filesystem::path DefaultOutputRoot();

    static std::filesystem::path Build(const RepoIndexOptions& options,
                                       std::string* error_out = nullptr);

    static std::filesystem::path Build(std::filesystem::path root,
                                       std::filesystem::path output_root = {},
                                       size_t chunk_lines = 200,
                                       std::string* error_out = nullptr);
};

}  // namespace LLM
KAI_END
