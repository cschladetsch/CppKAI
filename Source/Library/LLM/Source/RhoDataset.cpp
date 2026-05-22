#include <KAI/Core/Base.h>
#include <KAI/LLM/RhoDataset.h>

#include <KAI/LLM/ModelCache.h>

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <cstring>
#include <fstream>
#include <sstream>

KAI_BEGIN
namespace LLM {
namespace fs = std::filesystem;
using json = nlohmann::json;

namespace {

struct Record {
    std::string instruction;
    std::string input;
    std::string output;
    std::string source;
    std::string kind;
};

std::string Trim(std::string text) {
    const auto begin = std::find_if_not(text.begin(), text.end(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    });
    const auto end = std::find_if_not(text.rbegin(), text.rend(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    }).base();
    if (begin >= end) {
        return {};
    }
    return std::string(begin, end);
}

bool StartsWith(const std::string& text, const std::string& prefix) {
    return text.size() >= prefix.size() &&
           std::equal(prefix.begin(), prefix.end(), text.begin());
}

std::vector<std::string> ReadLines(const fs::path& path, std::string* error_out) {
    std::ifstream in(path);
    if (!in) {
        if (error_out) {
            *error_out = "Failed to open " + path.string();
        }
        return {};
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(in, line)) {
        lines.push_back(std::move(line));
    }
    return lines;
}

std::string ReadText(const fs::path& path, std::string* error_out) {
    std::ifstream in(path);
    if (!in) {
        if (error_out) {
            *error_out = "Failed to open " + path.string();
        }
        return {};
    }

    std::ostringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

bool IsRelevantPath(const fs::path& relative) {
    const std::string text = relative.generic_string();
    if (StartsWith(text, "Test/Language/TestRho/")) {
        return true;
    }
    if (StartsWith(text, "Source/App/Console/Source/")) {
        return true;
    }
    if (StartsWith(text, "Doc/")) {
        const std::string filename = relative.filename().string();
        return filename.find("Rho") != std::string::npos ||
               filename.find("Console") != std::string::npos ||
               filename == "LanguageGuide.md";
    }
    return false;
}

bool HasAllowedExtension(const fs::path& path) {
    const std::string ext = path.extension().string();
    return ext == ".cpp" || ext == ".rho" || ext == ".md" || ext == ".txt";
}

std::string RemoveCommentPrefix(std::string line) {
    line = Trim(std::move(line));
    if (StartsWith(line, "//")) {
        line.erase(0, 2);
    } else if (StartsWith(line, "/*")) {
        line.erase(0, 2);
    } else if (!line.empty() && line.front() == '*') {
        line.erase(0, 1);
    }
    return Trim(std::move(line));
}

std::string ExtractLeadingCommentSummary(const std::vector<std::string>& lines) {
    std::vector<std::string> collected;
    bool started = false;

    for (const auto& raw : lines) {
        const std::string line = Trim(raw);
        if (line.empty()) {
            if (started) {
                break;
            }
            continue;
        }

        if (StartsWith(line, "//") || StartsWith(line, "/*") ||
            StartsWith(line, "*") || line == "*/") {
            started = true;
            std::string cleaned = RemoveCommentPrefix(line);
            if (!cleaned.empty() && cleaned.find("====") == std::string::npos) {
                collected.push_back(std::move(cleaned));
            }
            continue;
        }

        if (started) {
            break;
        }
        break;
    }

    std::ostringstream out;
    for (size_t i = 0; i < collected.size(); ++i) {
        if (i != 0) {
            out << ' ';
        }
        out << collected[i];
    }
    return Trim(out.str());
}

std::string ExtractMarkdownSummary(const std::vector<std::string>& lines) {
    std::vector<std::string> collected;
    bool saw_heading = false;

    for (const auto& raw : lines) {
        const std::string line = Trim(raw);
        if (line.empty()) {
            if (saw_heading && !collected.empty()) {
                break;
            }
            continue;
        }

        if (StartsWith(line, "#")) {
            saw_heading = true;
            const size_t first = line.find_first_not_of('#');
            if (first == std::string::npos) {
                continue;
            }
            std::string text = Trim(line.substr(first));
            if (!text.empty()) {
                collected.push_back(std::move(text));
            }
            continue;
        }

        if (saw_heading) {
            if (StartsWith(line, "```")) {
                break;
            }
            collected.push_back(line);
            if (collected.size() >= 3) {
                break;
            }
        }
    }

    std::ostringstream out;
    for (size_t i = 0; i < collected.size(); ++i) {
        if (i != 0) {
            out << ' ';
        }
        out << collected[i];
    }
    return Trim(out.str());
}

std::string ExtractSnippet(const std::string& text, size_t max_chars) {
    if (text.size() <= max_chars) {
        return text;
    }
    return text.substr(0, max_chars);
}

std::optional<Record> ParseAssertResult(const std::string& text, size_t& pos,
                                        const fs::path& source,
                                        size_t max_input_chars) {
    const size_t start = text.find("AssertResult<", pos);
    if (start == std::string::npos) {
        pos = text.size();
        return std::nullopt;
    }

    size_t cursor = start + std::strlen("AssertResult<");
    const size_t template_end = text.find('>', cursor);
    if (template_end == std::string::npos) {
        pos = cursor;
        return std::nullopt;
    }

    cursor = template_end + 1;
    cursor = text.find('(', cursor);
    if (cursor == std::string::npos) {
        pos = template_end;
        return std::nullopt;
    }
    ++cursor;

    while (cursor < text.size() &&
           std::isspace(static_cast<unsigned char>(text[cursor])) != 0) {
        ++cursor;
    }
    if (cursor >= text.size() || text[cursor] != '"') {
        pos = cursor;
        return std::nullopt;
    }

    ++cursor;
    std::string input;
    bool escape = false;
    while (cursor < text.size()) {
        const char ch = text[cursor++];
        if (escape) {
            input.push_back(ch);
            escape = false;
            continue;
        }
        if (ch == '\\') {
            escape = true;
            continue;
        }
        if (ch == '"') {
            break;
        }
        input.push_back(ch);
    }
    input = ExtractSnippet(input, max_input_chars);

    cursor = text.find(',', cursor);
    if (cursor == std::string::npos) {
        pos = text.size();
        return std::nullopt;
    }
    ++cursor;

    int depth = 0;
    bool in_string = false;
    escape = false;
    std::string output;
    while (cursor < text.size()) {
        const char ch = text[cursor++];
        if (in_string) {
            output.push_back(ch);
            if (escape) {
                escape = false;
                continue;
            }
            if (ch == '\\') {
                escape = true;
                continue;
            }
            if (ch == '"') {
                in_string = false;
            }
            continue;
        }

        if (ch == '"') {
            in_string = true;
            output.push_back(ch);
            continue;
        }
        if (ch == '(') {
            ++depth;
            output.push_back(ch);
            continue;
        }
        if (ch == ')') {
            if (depth == 0) {
                break;
            }
            --depth;
            output.push_back(ch);
            continue;
        }
        if (ch == ';' && depth == 0) {
            break;
        }
        output.push_back(ch);
    }

    pos = cursor;

    Record record;
    record.instruction = "Evaluate this Rho expression.";
    record.input = Trim(std::move(input));
    record.output = Trim(std::move(output));
    record.source = source.generic_string();
    record.kind = "assert";
    if (record.input.empty() || record.output.empty()) {
        return std::nullopt;
    }
    return record;
}

std::vector<Record> BuildCppRecords(const fs::path& path,
                                    const std::vector<std::string>& lines,
                                    size_t max_input_chars) {
    std::vector<Record> records;
    const std::string text = ReadText(path, nullptr);
    const std::string summary = ExtractLeadingCommentSummary(lines);

    size_t pos = 0;
    while (true) {
        auto maybe = ParseAssertResult(text, pos, path, max_input_chars);
        if (!maybe) {
            break;
        }
        if (!summary.empty()) {
            maybe->output = summary + "\n" + maybe->output;
        }
        records.push_back(std::move(*maybe));
    }

    return records;
}

std::vector<Record> BuildTextRecords(const fs::path& path,
                                     const std::vector<std::string>& lines,
                                     size_t max_input_chars) {
    std::vector<Record> records;
    const std::string text = ReadText(path, nullptr);
    const std::string summary =
        path.extension() == ".md" ? ExtractMarkdownSummary(lines)
                                  : ExtractLeadingCommentSummary(lines);
    if (summary.empty()) {
        return records;
    }

    Record record;
    record.instruction = path.extension() == ".md"
                             ? "Summarize this Rho documentation excerpt."
                             : "Explain this Rho example.";
    record.input = ExtractSnippet(text, max_input_chars);
    record.output = summary;
    record.source = path.generic_string();
    record.kind = path.extension() == ".md" ? "doc" : "script";
    records.push_back(std::move(record));
    return records;
}

void WriteJsonl(const fs::path& path, const std::vector<Record>& records) {
    std::ofstream out(path);
    for (const auto& record : records) {
        json entry = json::object();
        entry["instruction"] = record.instruction;
        entry["input"] = record.input;
        entry["output"] = record.output;
        entry["source"] = record.source;
        entry["kind"] = record.kind;
        out << entry.dump() << '\n';
    }
}

}  // namespace

std::filesystem::path RhoDatasetBuilder::DefaultOutputRoot() {
    return fs::path(ModelCache::ResolveHome()) / "kai-rho-training";
}

std::filesystem::path RhoDatasetBuilder::Build(std::filesystem::path root,
                                               std::filesystem::path output_root,
                                               size_t max_input_chars,
                                               size_t max_records,
                                               std::string* error_out) {
    RhoDatasetOptions options;
    options.root = std::move(root);
    options.output_root = std::move(output_root);
    options.max_input_chars = max_input_chars;
    options.max_records = max_records;
    return Build(options, error_out);
}

std::filesystem::path RhoDatasetBuilder::Build(const RhoDatasetOptions& options,
                                               std::string* error_out) {
    if (options.root.empty()) {
        if (error_out) {
            *error_out = "Repository root is empty";
        }
        return {};
    }

    const fs::path output_root =
        options.output_root.empty() ? DefaultOutputRoot() : options.output_root;
    std::error_code ec;
    fs::create_directories(output_root, ec);
    if (ec) {
        if (error_out) {
            *error_out = "Failed to create output directory: " + output_root.string() +
                         " (" + ec.message() + ")";
        }
        return {};
    }

    const std::vector<fs::path> roots = {
        options.root / "Test/Language/TestRho",
        options.root / "Doc",
        options.root / "Source/App/Console/Source",
    };

    std::vector<Record> records;
    json manifest = json::object();
    manifest["schema"] = 1;
    manifest["repo_root"] = options.root.generic_string();
    manifest["output_root"] = output_root.generic_string();
    manifest["max_input_chars"] = options.max_input_chars;
    manifest["max_records"] = options.max_records;
    manifest["files"] = json::array();

    for (const auto& root : roots) {
        if (!fs::exists(root)) {
            continue;
        }

        for (fs::recursive_directory_iterator it(
                 root, fs::directory_options::skip_permission_denied, ec);
             it != fs::recursive_directory_iterator(); ++it) {
            if (ec) {
                if (error_out) {
                    *error_out = "Filesystem traversal error: " + ec.message();
                }
                return {};
            }

            if (!it->is_regular_file()) {
                continue;
            }

            const fs::path path = it->path();
            const fs::path relative = fs::relative(path, options.root, ec);
            if (ec || relative.empty()) {
                ec.clear();
                continue;
            }
            if (!IsRelevantPath(relative) || !HasAllowedExtension(path)) {
                continue;
            }

            std::string read_error;
            const std::vector<std::string> lines = ReadLines(path, &read_error);
            if (!read_error.empty()) {
                if (error_out) {
                    *error_out = read_error;
                }
                return {};
            }

            std::vector<Record> file_records;
            if (path.extension() == ".cpp") {
                file_records = BuildCppRecords(path, lines, options.max_input_chars);
            } else {
                file_records = BuildTextRecords(path, lines, options.max_input_chars);
            }

            if (file_records.empty()) {
                continue;
            }

            json file_entry = json::object();
            file_entry["path"] = relative.generic_string();
            file_entry["records"] = file_records.size();
            manifest["files"].push_back(std::move(file_entry));

            for (auto& record : file_records) {
                if (options.max_records != 0 && records.size() >= options.max_records) {
                    break;
                }
                records.push_back(std::move(record));
            }
            if (options.max_records != 0 && records.size() >= options.max_records) {
                break;
            }
        }
        if (options.max_records != 0 && records.size() >= options.max_records) {
            break;
        }
    }

    const fs::path dataset_path = output_root / "dataset.jsonl";
    WriteJsonl(dataset_path, records);

    manifest["records"] = records.size();
    std::ofstream manifest_out(output_root / "manifest.json");
    manifest_out << std::setw(2) << manifest << '\n';

    return output_root;
}

}  // namespace LLM
KAI_END
