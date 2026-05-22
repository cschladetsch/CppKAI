#include <KAI/LLM/RhoDataset.h>

#include <filesystem>
#include <iostream>
#include <string>

using namespace kai;
using namespace std;

namespace fs = std::filesystem;

namespace {

void PrintHelp() {
    cout << "KAI RhoDataset\n"
         << "Usage: RhoDataset [--root PATH] [--out PATH] [--max-input-chars N] [--max-records N]\n"
         << "\n"
         << "Builds a JSONL training set from Rho tests, scripts, and docs.\n";
}

}  // namespace

int main(int argc, char** argv) {
    fs::path root = fs::current_path();
    fs::path out = kai::LLM::RhoDatasetBuilder::DefaultOutputRoot();
    size_t max_input_chars = 4096;
    size_t max_records = 0;

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
        if (arg == "--max-input-chars" && i + 1 < argc) {
            max_input_chars = static_cast<size_t>(stoul(argv[++i]));
            continue;
        }
        if (arg == "--max-records" && i + 1 < argc) {
            max_records = static_cast<size_t>(stoul(argv[++i]));
            continue;
        }
        cerr << "Unknown or incomplete argument: " << arg << endl;
        return 1;
    }

    string error;
    kai::LLM::RhoDatasetOptions options;
    options.root = root;
    options.output_root = out;
    options.max_input_chars = max_input_chars;
    options.max_records = max_records;

    const fs::path built = kai::LLM::RhoDatasetBuilder::Build(options, &error);
    if (built.empty()) {
        cerr << error << endl;
        return 1;
    }

    cout << "Rho training dataset written to " << built << endl;
    return 0;
}
