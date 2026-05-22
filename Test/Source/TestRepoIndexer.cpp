#include "TestCommon.h"

#include <KAI/LLM/RepoIndexer.h>

#include <filesystem>
#include <fstream>

#include <nlohmann/json.hpp>

using namespace kai;
using namespace std;

namespace fs = std::filesystem;
using json = nlohmann::json;

TEST(TestRepoIndexer, BuildsChunkedKnowledgeBase) {
    const fs::path root = fs::temp_directory_path() / "kai-repo-indexer-root";
    const fs::path out = fs::temp_directory_path() / "kai-repo-indexer-out";
    std::error_code ec;
    fs::remove_all(root, ec);
    fs::remove_all(out, ec);

    fs::create_directories(root / "Include/KAI/LLM");
    fs::create_directories(root / "Test/Source");
    fs::create_directories(root / "build");

    {
        std::ofstream(root / "Include/KAI/LLM/Example.h")
            << "#pragma once\n"
            << "int add(int a, int b);\n";
        std::ofstream(root / "Test/Source/ExampleTest.cpp")
            << "#include <gtest/gtest.h>\n"
            << "TEST(Example, Works) {\n"
            << "  EXPECT_EQ(1 + 1, 2);\n"
            << "}\n";
        std::ofstream(root / "README.md") << "# Example\n";
        std::ofstream(root / "build/ignored.txt") << "ignore me\n";
    }

    std::string error;
    const fs::path built = LLM::RepoIndexer::Build(root, out, 1, &error);
    ASSERT_FALSE(built.empty()) << error;

    const fs::path index_path = built / "index.json";
    ASSERT_TRUE(fs::exists(index_path));

    std::ifstream in(index_path);
    json index = json::parse(in);
    ASSERT_TRUE(index.contains("entries"));
    EXPECT_EQ(index["entries"].size(), 3);

    const auto first_chunk = built / index["entries"][0]["chunks"][0]["path"].get<string>();
    ASSERT_TRUE(fs::exists(first_chunk));

    fs::remove_all(root, ec);
    fs::remove_all(out, ec);
}
