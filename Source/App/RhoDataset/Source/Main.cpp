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
         << "Usage: RhoDataset [--root PATH] [--out PATH] "
            "[--max-input-chars N] [--max-records N]\n"
         << "\n"
         << "Builds a JSONL language dataset from Rho, Pi, Tau, gtests, "
            "logs, history files, and README documentation.\n";
}

}  // namespace

int main(int argc, char** argv) {
    fs::path root = fs::current_path();
    fs::path out = kai::llm::RhoDatasetBuilder::DefaultOutputRoot();
    size_t maxInputChars = 4096;
    size_t maxRecords = 0;

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
            maxInputChars = static_cast<size_t>(stoul(argv[++i]));
            continue;
        }
        if (arg == "--max-records" && i + 1 < argc) {
            maxRecords = static_cast<size_t>(stoul(argv[++i]));
            continue;
        }
        cerr << "Unknown or incomplete argument: " << arg << '\n';
        return 1;
    }

    string error;
    kai::llm::RhoDatasetOptions options;
    options.root = root;
    options.outputRoot = out;
    options.maxInputChars = maxInputChars;
    options.maxRecords = maxRecords;

    const fs::path built = kai::llm::RhoDatasetBuilder::Build(options, &error);
    if (built.empty()) {
        cerr << error << '\n';
        return 1;
    }

    cout << "KAI language dataset written to " << built << '\n';
    return 0;
}
