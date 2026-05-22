#include <KAI/LLM/RepoIndexer.h>

#include <filesystem>
#include <iostream>
#include <string>

using namespace kai;
using namespace std;

namespace fs = std::filesystem;

namespace {

void PrintHelp() {
    cout << "KAI RepoIndex\n"
         << "Usage: RepoIndex [--root PATH] [--out PATH] [--chunk-lines N]\n"
         << "\n"
         << "Builds a local code/test knowledge base under the model cache.\n";
}

}  // namespace

int main(int argc, char** argv) {
    fs::path root = fs::current_path();
    fs::path out = LLM::RepoIndexer::DefaultOutputRoot();
    size_t chunk_lines = 200;

    for (int i = 1; i < argc; ++i) {
        const string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            PrintHelp();
            return 0;
        }
        if (arg == "--root" && i + 1 < argc) {
            root = argv[++i];
            continue;
        }
        if (arg == "--out" && i + 1 < argc) {
            out = argv[++i];
            continue;
        }
        if (arg == "--chunk-lines" && i + 1 < argc) {
            chunk_lines = static_cast<size_t>(stoul(argv[++i]));
            continue;
        }
        cerr << "Unknown or incomplete argument: " << arg << endl;
        return 1;
    }

    string error;
    LLM::RepoIndexOptions options;
    options.root = root;
    options.output_root = out;
    options.chunk_lines = chunk_lines;

    const fs::path built = LLM::RepoIndexer::Build(options, &error);
    if (built.empty()) {
        cerr << error << endl;
        return 1;
    }

    cout << "Repo knowledge base written to " << built << endl;
    return 0;
}
