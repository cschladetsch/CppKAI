#include <KAI/LLM/Session.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

using namespace kai;
using namespace std;

namespace fs = std::filesystem;

namespace {

struct EvaluationCase {
    string name;
    string language;
    string prompt;
    vector<string> must_contain;
    vector<string> must_not_contain;
};

struct EvaluationSuite {
    vector<EvaluationCase> cases;
};

string Trim(string text) {
    const auto begin = find_if_not(text.begin(), text.end(), [](unsigned char ch) {
        return isspace(ch) != 0;
    });
    const auto end = find_if_not(text.rbegin(), text.rend(), [](unsigned char ch) {
        return isspace(ch) != 0;
    }).base();
    if (begin >= end) {
        return {};
    }
    return string(begin, end);
}

bool StartsWith(const string& text, const string& prefix) {
    return text.size() >= prefix.size() &&
           equal(prefix.begin(), prefix.end(), text.begin());
}

EvaluationSuite LoadSuite(const fs::path& path) {
    ifstream in(path);
    EXPECT_TRUE(in) << "Failed to open evaluation suite: " << path;

    EvaluationSuite suite;
    EvaluationCase current;
    bool in_case = false;
    string line;

    auto flush_case = [&]() {
        if (in_case) {
            suite.cases.push_back(current);
            current = {};
            in_case = false;
        }
    };

    while (getline(in, line)) {
        const string trimmed = Trim(line);
        if (trimmed.empty() || StartsWith(trimmed, "#")) {
            continue;
        }
        if (StartsWith(trimmed, "case:")) {
            flush_case();
            current.name = Trim(trimmed.substr(5));
            in_case = true;
            continue;
        }
        if (!in_case) {
            continue;
        }
        if (StartsWith(trimmed, "language:")) {
            current.language = Trim(trimmed.substr(9));
            continue;
        }
        if (StartsWith(trimmed, "prompt:")) {
            current.prompt = Trim(trimmed.substr(7));
            continue;
        }
        if (StartsWith(trimmed, "must_contain:")) {
            current.must_contain.push_back(Trim(trimmed.substr(13)));
            continue;
        }
        if (StartsWith(trimmed, "must_not_contain:")) {
            current.must_not_contain.push_back(Trim(trimmed.substr(17)));
            continue;
        }
    }

    flush_case();
    return suite;
}

string ShellQuote(string_view text) {
#if defined(_WIN32)
    string quoted = "\"";
    for (char ch : text) {
        if (ch == '"') {
            quoted.push_back('\\');
        }
        quoted.push_back(ch);
    }
    quoted.push_back('"');
    return quoted;
#else
    string quoted = "'";
    for (char ch : text) {
        if (ch == '\'') {
            quoted += "'\"'\"'";
        } else {
            quoted.push_back(ch);
        }
    }
    quoted.push_back('\'');
    return quoted;
#endif
}

optional<string> RunCommand(const string& command) {
    array<char, 4096> buffer{};
    string output;

#if defined(_WIN32)
    FILE* pipe = _popen(command.c_str(), "r");
#else
    FILE* pipe = popen(command.c_str(), "r");
#endif
    if (!pipe) {
        return nullopt;
    }

    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
        output.append(buffer.data());
    }

#if defined(_WIN32)
    const int status = _pclose(pipe);
#else
    const int status = pclose(pipe);
#endif
    if (status != 0) {
        return nullopt;
    }

    return output;
}

string ExpandPromptCommand(string command_template, string_view prompt) {
    const string quoted_prompt = ShellQuote(prompt);
    const string token = "{PROMPT}";
    size_t pos = 0;
    while ((pos = command_template.find(token, pos)) != string::npos) {
        command_template.replace(pos, token.size(), quoted_prompt);
        pos += quoted_prompt.size();
    }
    return command_template;
}

string ModelResponse(string_view command_template, string_view prompt) {
    const string command = ExpandPromptCommand(string(command_template), prompt);
    auto response = RunCommand(command);
    if (!response) {
        return {};
    }
    return Trim(std::move(*response));
}

using ResponseProvider = function<string(const EvaluationCase&)>;

void RunSuite(const EvaluationSuite& suite, const ResponseProvider& provider) {
    for (const auto& test_case : suite.cases) {
        const string reply = provider(test_case);
        ASSERT_FALSE(reply.empty()) << test_case.name << ": model command returned no output";

        for (const auto& needle : test_case.must_contain) {
            EXPECT_NE(reply.find(needle), string::npos)
                << test_case.name << ": missing `" << needle << "`";
        }
        for (const auto& needle : test_case.must_not_contain) {
            EXPECT_EQ(reply.find(needle), string::npos)
                << test_case.name << ": contained `" << needle << "`";
        }
    }
}

string DeterministicResponse(const EvaluationCase& test_case) {
    if (test_case.name == "rho-repair") {
        return "Rho repair: foo fun(a, b) ...\nPi transpile to Pi ...";
    }
    if (test_case.name == "pi-stack") {
        return "Pi stack effect: push 1, push 2, add, leave [0] = 3.";
    }
    if (test_case.name == "tau-idl") {
        return "Tau repair: interface stays intact, then generate proxy and agent.";
    }
    return test_case.language + ": no answer";
}

}  // namespace

TEST(TestLlmEvaluation, RunsPromptSuiteFromTrainingFolder) {
    const fs::path suite_path =
        fs::current_path() / "Scripts/Training/eval/llm-eval.md";
    const EvaluationSuite suite = LoadSuite(suite_path);
    ASSERT_FALSE(suite.cases.empty());
    RunSuite(suite, DeterministicResponse);
}

TEST(TestLlmEvaluation, RunsPromptSuiteAgainstRealModelIfConfigured) {
    const char* command_template = std::getenv("KAI_LLM_EVAL_COMMAND");
    if (command_template == nullptr || string(command_template).empty()) {
        GTEST_SKIP() << "Set KAI_LLM_EVAL_COMMAND to a real model command with {PROMPT}.";
    }

    const fs::path suite_path =
        fs::current_path() / "Scripts/Training/eval/llm-eval.md";
    const EvaluationSuite suite = LoadSuite(suite_path);
    ASSERT_FALSE(suite.cases.empty());

    RunSuite(suite, [command_template](const EvaluationCase& test_case) {
        return ModelResponse(command_template, test_case.prompt);
    });
}
