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

    WriteFile(root / "README.md", R"(# Root Readme

This top-level README should be included as documentation context.
)");

    WriteFile(root / "Source/Library/Language/Tau/README.md",
              R"(# Tau Readme

Tau interface documentation should be included from nested README files.
)");

    WriteFile(root / "Test/Language/TestPi/Sample.cpp",
              R"(// Sample Pi dataset fixture

TEST(PiSynthetic, StackMath) {
    AssertResult<int>("1 2 +", 3);
}
)");

    WriteFile(root / "Test/Language/TestTau/Sample.cpp",
              R"(// Sample Tau dataset fixture

TEST(TauSynthetic, InterfaceParsing) {
    const char *script = "namespace Demo { interface ICalc { int Add(int a, int b); } }";
}
)");

    WriteFile(root / "Logs/errors.log",
              "Rho parser error: unexpected token near while\n");
    WriteFile(root / "Scripts/Training/rho-repair.md", R"(# Rho Repair

Teach KAI to repair malformed Rho function declarations.
)");
    WriteFile(root / "Scripts/Training/consent-policy.md", R"(# Consent Policy

KAI should ingest ordinary local evidence silently and ask only for large-impact corpus changes.
)");
    WriteFile(root / "Source/App/Console/Source/session.history",
              "rho\nx = 1 + 2\npi\n1 2 +\n");

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
    bool saw_root_readme = false;
    bool saw_nested_readme = false;
    bool saw_pi = false;
    bool saw_tau_gtest = false;
    bool saw_log = false;
    bool saw_training = false;
    bool saw_consent = false;
    bool saw_history = false;
    bool saw_addition = false;
    bool saw_boolean = false;
    for (const auto& record : records) {
        ASSERT_TRUE(record.contains("instruction"));
        ASSERT_TRUE(record.contains("input"));
        ASSERT_TRUE(record.contains("output"));
        ASSERT_TRUE(record.contains("source"));
        ASSERT_TRUE(record.contains("kind"));
        ASSERT_TRUE(record.contains("language"));

        const std::string kind = record["kind"];
        if (kind == "assert") {
            saw_assert = true;
            const std::string language = record["language"];
            EXPECT_NE(record["instruction"].get<std::string>().find(language),
                      std::string::npos);
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
            if (input == "1 2 +") {
                saw_pi = true;
                EXPECT_EQ(language, "Pi");
                EXPECT_NE(output.find("3"), std::string::npos);
            }
        } else if (kind == "script") {
            saw_script = true;
            EXPECT_NE(record["instruction"].get<std::string>().find("example"),
                      std::string::npos);
            EXPECT_NE(
                record["output"].get<std::string>().find("Sample Rho script"),
                std::string::npos);
        } else if (kind == "doc") {
            const std::string source = record["source"];
            const std::string instruction = record["instruction"];
            const std::string output = record["output"];
            const std::string input = record["input"];
            if (source.find("Doc/RhoSample.md") != std::string::npos) {
                saw_doc = true;
                EXPECT_NE(instruction.find("documentation excerpt"),
                          std::string::npos);
                EXPECT_NE(output.find("Sample Rho Doc"), std::string::npos);
            }
            if (source == "README.md") {
                saw_root_readme = true;
                EXPECT_NE(instruction.find("documentation excerpt"),
                          std::string::npos);
                EXPECT_NE(output.find("Root Readme"), std::string::npos);
            }
            if (source.find("Source/Library/Language/Tau/README.md") !=
                std::string::npos) {
                saw_nested_readme = true;
                EXPECT_NE(instruction.find("documentation excerpt"),
                          std::string::npos);
                EXPECT_NE(output.find("Tau Readme"), std::string::npos);
            }
            if (source.find("Scripts/Training/rho-repair.md") !=
                std::string::npos) {
                saw_training = true;
                EXPECT_EQ(record["language"], "KAI");
                EXPECT_NE(instruction.find("incremental KAI training note"),
                          std::string::npos);
                EXPECT_NE(output.find("Rho Repair"), std::string::npos);
            }
            if (source.find("Scripts/Training/consent-policy.md") !=
                std::string::npos) {
                saw_consent = true;
                EXPECT_EQ(record["language"], "KAI");
                EXPECT_NE(instruction.find("incremental KAI training note"),
                          std::string::npos);
                EXPECT_NE(input.find("large-impact corpus changes"),
                          std::string::npos);
            }
            if (source.find("Scripts/Training/") != std::string::npos) {
                saw_doc = true;
            }
        } else if (kind == "gtest") {
            if (record["language"] == "Tau") {
                saw_tau_gtest = true;
                EXPECT_NE(record["output"].get<std::string>().find(
                              "TauSynthetic.InterfaceParsing"),
                          std::string::npos);
            }
        } else if (kind == "log") {
            saw_log = true;
            EXPECT_EQ(record["language"], "KAI");
            EXPECT_NE(record["input"].get<std::string>().find(
                          "Rho parser error"),
                      std::string::npos);
        } else if (kind == "history") {
            saw_history = true;
            EXPECT_EQ(record["language"], "KAI");
            EXPECT_NE(record["input"].get<std::string>().find("1 2 +"),
                      std::string::npos);
        }
    }

    EXPECT_TRUE(saw_assert);
    EXPECT_TRUE(saw_addition);
    EXPECT_TRUE(saw_boolean);
    EXPECT_TRUE(saw_script);
    EXPECT_TRUE(saw_doc);
    EXPECT_TRUE(saw_root_readme);
    EXPECT_TRUE(saw_nested_readme);
    EXPECT_TRUE(saw_pi);
    EXPECT_TRUE(saw_tau_gtest);
    EXPECT_TRUE(saw_log);
    EXPECT_TRUE(saw_training);
    EXPECT_TRUE(saw_consent);
    EXPECT_TRUE(saw_history);

    fs::remove_all(root);
}
