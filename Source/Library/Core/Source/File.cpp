#include "KAI/Core/File.h"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

#include "KAI/Core/Exception.h"
#include "KAI/Core/Object.h"

using namespace std;
using namespace std::filesystem;

KAI_BEGIN

namespace File {
namespace fs = std::filesystem;

// return a vector of all files that have the given extension, starting from
// root, recursively
Pathnames GetFilesWithExtensionRecursively(Pathname const &root,
                                           Extension const &ext) {
    recursive_directory_iterator iter(root);
    const recursive_directory_iterator end;

    vector<path> result;
    for (; iter != end; ++iter) {
        auto const &val = *iter;
        const auto regular_file = is_regular_file(val);
        auto extension = val.path().extension();
        const auto same_extenstion = extension == ext;

        if (regular_file && same_extenstion) result.push_back(val.path());
    }

    return result;
}

Pathnames GetFilesRecursively(Pathname const &root) { KAI_NOT_IMPLEMENTED(); }

Pathnames GetFiles(Pathname const &root) { KAI_NOT_IMPLEMENTED(); }

// Read all text from given filename
string ReadAllText(Pathname const &path) {
    std::ifstream f(path);
    return string(istreambuf_iterator<char>(f), istreambuf_iterator<char>());
}

std::wstring ReadAllWideText(Pathname const &path) {
    std::ifstream f(path);
    return wstring(istreambuf_iterator<char>(f), istreambuf_iterator<char>());
}

// replace a file with the contents of a string
bool ReplaceWithText(std::string const &text, Pathname const &path) {
    std::ofstream out(path.string().c_str(), ios_base::out);
    if (!out) return false;

    std::copy(text.begin(), text.end(), ostream_iterator<char>(out));
    return out.good() && out.tellp() == text.size();
}

// replace a file with a sequence of bytes
bool ReplaceWithBinary(byte const *bytes, size_t num_bytes,
                       Pathname const &path) {
    KAI_NOT_IMPLEMENTED();
}
}  // namespace File

KAI_END

// EOF
