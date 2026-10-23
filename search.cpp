#include "fun.h"

// Recursive search helper
static bool searchRecursive(const std::string& target, const std::string& currentDir) {
    DIR* dir = opendir(currentDir.c_str());
    if (!dir) {
        return false;
    }

    struct dirent* entry;
    bool found = false;

    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;

        // Skip current and parent directory pointers
        if (name == "." || name == "..") {
            continue;
        }

        if (name == target) {
            found = true;
            break;
        }

        std::string full_path = currentDir + "/" + name;
        struct stat st;
        if (stat(full_path.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
            if (searchRecursive(target, full_path)) {
                found = true;
                break;
            }
        }
    }

    closedir(dir);
    return found;
}

// Search for a file/directory recursively
bool searchFile(const std::string& target, const std::string& currentDir) {
    return searchRecursive(target, currentDir);
}

// Wrapper for built-in search command
void runSearch(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cerr << "search: missing target filename/directory name\n";
        std::cerr << "Usage: search <name>\n";
        return;
    }

    std::string target = args[1];
    bool result = searchFile(target, ".");
    if (result) {
        std::cout << "True\n";
    } else {
        std::cout << "False\n";
    }
}
