#include <KAI/Core/Base.h>
#include <KAI/LLM/ModelCache.h>
#include <KAI/LLM/RepoIndexer.h>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <sstream>
#include <vector>

KAI_BEGIN
namespace LLM {
namespace fs = std::filesystem;
using json = nlohmann::json;

namespace {

bool HasAllowedExtension(const fs::path& path) {
    const std::string ext = path.extension().string();
    static const std::vector<std::string> allowed = {
        ".cpp",   ".cc",  ".cxx", ".h",    ".hpp", ".md", ".txt",
        ".cmake", ".tau", ".sh",  ".json", ".py",  ".in", ".am"};
    return std::find(allowed.begin(), allowed.end(), ext) != allowed.end() ||
           path.filename() == "README" || path.filename() == "LICENSE";
}

bool IsIgnoredPath(const fs::path& relative) {
    for (const auto& part : relative) {
        const std::string name = part.string();
        if (name == ".git" || name == "Bin" || name == "build" ||
            name == "Build" || name == "Logs" || name == "CMakeFiles" ||
            name == "Ext") {
            return true;
        }
    }
    return false;
}

std::string CategoryFor(const fs::path& relative) {
    const std::string top = relative.begin() != relative.end()
                                ? relative.begin()->string()
                                : std::string();
    if (top == "Include") return "header";
    if (top == "Source") return "source";
    if (top == "Test") return "test";
    if (top == "Doc") return "doc";
    if (top == "Scripts") return "script";
    if (top.empty()) return "root";
    return "misc";
}

std::string SanitizeName(const fs::path& relative) {
    std::string name = relative.generic_string();
    for (char& ch : name) {
        if (ch == '/' || ch == '\\' || ch == ':' || ch == ' ' || ch == '.') {
            ch = '_';
        }
    }
    return name;
}

std::vector<std::string> ReadLines(const fs::path& path,
                                   std::string* error_out) {
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
        lines.push_back(line);
    }
    return lines;
}

void WriteChunkFile(const fs::path& output_path, const fs::path& source_path,
                    size_t start_line, size_t end_line,
                    const std::vector<std::string>& lines) {
    std::ofstream out(output_path);
    out << "Source: " << source_path.generic_string() << '\n';
    out << "Lines: " << start_line << "-" << end_line << '\n';
    out << '\n';
    for (const auto& line : lines) {
        out << line << '\n';
    }
}

}  // namespace

std::filesystem::path RepoIndexer::DefaultOutputRoot() {
    return fs::path(ModelCache::ResolveHome()) / "kai-repo-index";
}

std::filesystem::path RepoIndexer::Build(std::filesystem::path root,
                                         std::filesystem::path output_root,
                                         size_t chunk_lines,
                                         std::string* error_out) {
    RepoIndexOptions options;
    options.root = std::move(root);
    options.output_root = std::move(output_root);
    options.chunk_lines = chunk_lines;
    return Build(options, error_out);
}

std::filesystem::path RepoIndexer::Build(const RepoIndexOptions& options,
                                         std::string* error_out) {
    if (options.root.empty()) {
        if (error_out) {
            *error_out = "Repository root is empty";
        }
        return {};
    }

    const fs::path output_root =
        options.output_root.empty() ? DefaultOutputRoot() : options.output_root;
    const fs::path chunks_root = output_root / "chunks";
    std::error_code ec;
    fs::create_directories(chunks_root, ec);
    if (ec) {
        if (error_out) {
            *error_out =
                "Failed to create output directory: " + output_root.string() +
                " (" + ec.message() + ")";
        }
        return {};
    }

    json index = json::object();
    index["schema"] = 1;
    index["repo_root"] = options.root.generic_string();
    index["output_root"] = output_root.generic_string();
    index["chunk_lines"] = options.chunk_lines;
    index["entries"] = json::array();

    for (fs::recursive_directory_iterator it(
             options.root, fs::directory_options::skip_permission_denied, ec);
         it != fs::recursive_directory_iterator(); ++it) {
        if (ec) {
            if (error_out) {
                *error_out = "Filesystem traversal error: " + ec.message();
            }
            return {};
        }

        const fs::path path = it->path();
        const fs::path relative = fs::relative(path, options.root, ec);
        if (ec || relative.empty()) {
            ec.clear();
            continue;
        }

        if (it->is_directory()) {
            if (IsIgnoredPath(relative)) {
                it.disable_recursion_pending();
            }
            continue;
        }

        if (IsIgnoredPath(relative) || !HasAllowedExtension(path)) {
            continue;
        }

        std::string read_error;
        std::vector<std::string> lines = ReadLines(path, &read_error);
        if (!read_error.empty()) {
            if (error_out) {
                *error_out = read_error;
            }
            return {};
        }

        json entry = json::object();
        entry["path"] = relative.generic_string();
        entry["category"] = CategoryFor(relative);
        entry["lines"] = lines.size();
        entry["bytes"] = fs::file_size(path, ec);
        if (ec) {
            ec.clear();
            entry["bytes"] = 0;
        }

        json chunks = json::array();
        size_t chunk_index = 0;
        for (size_t start = 0; start < lines.size();
             start += options.chunk_lines) {
            const size_t end =
                std::min(start + options.chunk_lines, lines.size());
            const size_t start_line = start + 1;
            const size_t end_line = end;

            const fs::path chunk_name = SanitizeName(relative) + "." +
                                        std::to_string(++chunk_index) + ".md";
            const fs::path chunk_path = chunks_root / chunk_name;
            std::vector<std::string> chunk_lines(lines.begin() + start,
                                                 lines.begin() + end);
            WriteChunkFile(chunk_path, relative, start_line, end_line,
                           chunk_lines);

            json chunk = json::object();
            chunk["path"] =
                fs::relative(chunk_path, output_root, ec).generic_string();
            if (ec) {
                ec.clear();
                chunk["path"] = chunk_path.generic_string();
            }
            chunk["start_line"] = start_line;
            chunk["end_line"] = end_line;
            chunks.push_back(std::move(chunk));
        }

        entry["chunks"] = std::move(chunks);
        index["entries"].push_back(std::move(entry));
    }

    std::ofstream out(output_root / "index.json");
    out << std::setw(2) << index << '\n';
    return output_root;
}

}  // namespace LLM
KAI_END
