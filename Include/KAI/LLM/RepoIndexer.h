#pragma once

#include <KAI/Core/Base.h>

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

KAI_BEGIN
namespace llm
{

struct RepoIndexOptions {
    std::filesystem::path root;
    std::filesystem::path outputRoot;
    size_t chunkLines = 200;
    bool includeExt = false;
};

class RepoIndexer {
   public:
    static std::filesystem::path DefaultOutputRoot();

    static std::filesystem::path Build(const RepoIndexOptions& options, std::string* errorOut = nullptr);

    static std::filesystem::path Build(std::filesystem::path root, std::filesystem::path outputRoot = {},
                                       size_t chunkLines = 200, std::string* errorOut = nullptr);
};

} // namespace llm
KAI_END
