#pragma once

#include <KAI/Core/Base.h>

#include <filesystem>
#include <string>

KAI_BEGIN
namespace llm
{

struct RhoDatasetOptions {
    std::filesystem::path root;
    std::filesystem::path outputRoot;
    size_t maxRecords = 0;
    size_t maxInputChars = 4096;
};

class RhoDatasetBuilder {
   public:
    static std::filesystem::path DefaultOutputRoot();

    static std::filesystem::path Build(const RhoDatasetOptions& options, std::string* errorOut = nullptr);

    static std::filesystem::path Build(std::filesystem::path root, std::filesystem::path outputRoot = {},
                                       size_t maxInputChars = 4096, size_t maxRecords = 0,
                                       std::string* errorOut = nullptr);
};

} // namespace llm
KAI_END
