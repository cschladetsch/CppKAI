#include <KAI/LLM/RhoDataset.h>
#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <vector>

using json = nlohmann::json;

namespace fs = std::filesystem;

namespace {

fs::path UniqueTempRoot() {
    const auto now =
        std::chrono::steady_clock::now().time_since_epoch().count();
    return fs::temp_directory_path() /
           ("kai-rho-dataset-" + std::to_string(now));
}

void WriteFile(const fs::path& path, const std::string& text) {
    fs::create_directories(path.parent_path());
    std::ofstream out(path);
    out << text;
}

std::vector<json> ReadJsonl(const fs::path& path) {
    std::ifstream in(path);
    std::vector<json> records;
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty()) {
            records.push_back(json::parse(line));
        }
    }
    return records;
}

}  // namespace

TEST(TestRhoDataset, BuildsJsonlCorpusFromRhoSources) {
    const fs::path root = UniqueTempRoot();
    const fs::path output = root / "out";
    fs::remove_all(root);

    WriteFile(root / "Test/Language/TestRho/Sample.cpp",
              R"(// Sample Rho dataset fixture
// Covers arithmetic and booleans.

TEST(RhoSynthetic, Arithmetic) {
    AssertResult<int>("1 + 2", 3);
    AssertResult<bool>("true && false", false);
}
)");

    WriteFile(root / "Test/Language/TestRho/Sample.rho", R"(// Sample Rho script
// Demonstrates assignment and a print.

x = 42;
print(x);
)");

    WriteFile(root / "Doc/RhoSample.md", R"(# Sample Rho Doc

This document describes a small Rho example for the training set.

```rho
x = 1 + 2
```
)");

    std::string error;
    kai::LLM::RhoDatasetOptions options;
    options.root = root;
    options.output_root = output;
    options.max_input_chars = 2048;

    const fs::path built = kai::LLM::RhoDatasetBuilder::Build(options, &error);
    ASSERT_FALSE(built.empty()) << error;

    const auto records = ReadJsonl(output / "dataset.jsonl");
    ASSERT_GE(records.size(), 3u);

    bool saw_assert = false;
    bool saw_script = false;
    bool saw_doc = false;
    bool saw_addition = false;
    bool saw_boolean = false;
    for (const auto& record : records) {
        ASSERT_TRUE(record.contains("instruction"));
        ASSERT_TRUE(record.contains("input"));
        ASSERT_TRUE(record.contains("output"));
        ASSERT_TRUE(record.contains("source"));
        ASSERT_TRUE(record.contains("kind"));

        const std::string kind = record["kind"];
        if (kind == "assert") {
            saw_assert = true;
            EXPECT_EQ(record["instruction"], "Evaluate this Rho expression.");
            const std::string input = record["input"];
            const std::string output = record["output"];
            if (input == "1 + 2") {
                saw_addition = true;
                EXPECT_NE(output.find("3"), std::string::npos);
            }
            if (input == "true && false") {
                saw_boolean = true;
                EXPECT_NE(output.find("false"), std::string::npos);
            }
        } else if (kind == "script") {
            saw_script = true;
            EXPECT_EQ(record["instruction"], "Explain this Rho example.");
            EXPECT_NE(
                record["output"].get<std::string>().find("Sample Rho script"),
                std::string::npos);
        } else if (kind == "doc") {
            saw_doc = true;
            EXPECT_EQ(record["instruction"],
                      "Summarize this Rho documentation excerpt.");
            EXPECT_NE(
                record["output"].get<std::string>().find("Sample Rho Doc"),
                std::string::npos);
        }
    }

    EXPECT_TRUE(saw_assert);
    EXPECT_TRUE(saw_addition);
    EXPECT_TRUE(saw_boolean);
    EXPECT_TRUE(saw_script);
    EXPECT_TRUE(saw_doc);

    fs::remove_all(root);
}
