#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "KAI/Core/Config/Base.h"

KAI_BEGIN

namespace File {
using Pathname = std::filesystem::path;
using Extension = std::string;  // Pathname::string_type;
using Pathnames = std::vector<Pathname>;
using byte = std::uint8_t;

namespace fs = std::filesystem;

bool Exists(Pathname const &);
bool IsFile(Pathname const &);
bool IsFolder(Pathname const &);

// return a vector of all files that have the given extension, starting from
// _root, recursively
Pathnames GetFilesWithExtensionRecursively(Pathname const &root,
                                           Extension const &ext);
Pathnames GetFilesRecursively(Pathname const &root);
Pathnames GetFiles(Pathname const &root);

// Read all text from given filename
std::string ReadAllText(Pathname const &);
std::wstring ReadAllWideText(Pathname const &);

// replace a file with the contents of a string
bool ReplaceWithText(std::string const &, Pathname const &);

template <class Byte = std::uint8_t>
std::vector<Byte> ReadAllBinary(const fs::path &path) {
    // Open the file using low-level system calls
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file");
    }

    // Determine the file size
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    // Allocate a buffer to hold the file's content
    std::vector<Byte> buffer(size);

    // Read the file into the buffer
    if (!file.read(reinterpret_cast<char *>(buffer.data()), size)) {
        throw std::runtime_error("Error reading file");
    }

    return buffer;
}

// replace a file with a sequence of bytes
bool ReplaceWithBinary(byte const *, size_t num_bytes, Pathname const &);

inline bool ReplaceWithBinary(std::vector<byte> const &bytes,
                              Pathname const &pathname) {
    return ReplaceWithBinary(&*bytes.begin(), bytes.size(), pathname);
}
}  // namespace File

KAI_END
