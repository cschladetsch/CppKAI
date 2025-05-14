#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <filesystem>
#include <set>
#include <map>

namespace fs = std::filesystem;

// Structure to hold field information
struct FieldInfo {
    std::string className;
    std::string fileName;
    int lineNumber;
    
    bool operator<(const FieldInfo& other) const {
        if (className != other.className) return className < other.className;
        if (fileName != other.fileName) return fileName < other.fileName;
        return lineNumber < other.lineNumber;
    }
};

// Map field name to its occurrence info
std::map<std::string, std::set<FieldInfo>> fieldOccurrences;

// Find class definition and member variables with underscore prefix
void searchFile(const fs::path& filePath) {
    std::ifstream file(filePath);
    if (!file) return;
    
    std::string line, currentClass;
    int lineNumber = 0;
    bool inClass = false;
    bool inPrivate = false;
    bool inProtected = false;
    
    // Regex to match class/struct definition
    std::regex classRegex(R"(^\s*(class|struct)\s+(\w+))");
    
    // Regex to match member variables with underscore prefix
    std::regex fieldRegex(R"(^\s+(\w+(?:<[^>]+>)?(?:\s+\w+)*\s+)(_\w+)(\s*\{[^}]*\})?(\s*;\s*))");
    
    // Regex to match class section visibility
    std::regex sectionRegex(R"(^\s*(private|protected|public):)");
    
    while (std::getline(file, line)) {
        lineNumber++;
        
        std::smatch classMatch;
        if (std::regex_search(line, classMatch, classRegex)) {
            currentClass = classMatch[2];
            inClass = true;
            continue;
        }
        
        if (inClass) {
            std::smatch sectionMatch;
            if (std::regex_search(line, sectionMatch, sectionRegex)) {
                std::string section = sectionMatch[1];
                inPrivate = (section == "private");
                inProtected = (section == "protected");
                continue;
            }
            
            // Check for class ending
            if (line.find("};") != std::string::npos) {
                inClass = false;
                continue;
            }
            
            // Only look at private and protected sections
            if (inPrivate || inProtected) {
                std::smatch fieldMatch;
                if (std::regex_search(line, fieldMatch, fieldRegex)) {
                    std::string fieldName = fieldMatch[2];
                    
                    FieldInfo info;
                    info.className = currentClass;
                    info.fileName = filePath.string();
                    info.lineNumber = lineNumber;
                    
                    fieldOccurrences[fieldName].insert(info);
                }
            }
        }
    }
}

void searchDirectory(const fs::path& directory) {
    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        std::cerr << "Directory doesn't exist: " << directory << std::endl;
        return;
    }
    
    for (const auto& entry : fs::recursive_directory_iterator(directory)) {
        if (fs::is_regular_file(entry.path())) {
            std::string ext = entry.path().extension().string();
            if (ext == ".h" || ext == ".hpp") {
                searchFile(entry.path());
            }
        }
    }
}

int main() {
    // Search in Include directory for class definitions and member variables
    searchDirectory("/home/xian/local/KAI/Include");
    
    std::cout << "# Fields with Underscore Prefix to Convert\n\n";
    std::cout << "| Field Name | New Name | Class | File | Line |\n";
    std::cout << "|------------|----------|-------|------|------|\n";
    
    for (const auto& [fieldName, occurrences] : fieldOccurrences) {
        // Skip if field name doesn't start with underscore
        if (fieldName[0] != '_') continue;
        
        // Calculate new field name by moving underscore to the end
        std::string newName = fieldName.substr(1) + "_";
        
        // Only display one occurrence per field (the first one)
        if (!occurrences.empty()) {
            const auto& firstOccurrence = *occurrences.begin();
            std::cout << "| " << fieldName << " | " << newName << " | " 
                      << firstOccurrence.className << " | " 
                      << fs::path(firstOccurrence.fileName).filename().string() << " | "
                      << firstOccurrence.lineNumber << " |\n";
        }
    }
    
    std::cout << "\n\nTotal unique fields to rename: " << fieldOccurrences.size() << std::endl;
    
    return 0;
}