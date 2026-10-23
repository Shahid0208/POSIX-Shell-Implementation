#include "fun.h"
#include <set>

// List of built-in and common commands for autocomplete
static const std::vector<std::string> known_commands = {
    "cd", "pwd", "echo", "ls", "pinfo", "search", "history", "exit", "cat", "grep", "wc", "clear", "sleep"
};

// Generator function for command completion
char* suggest_match(const char* text, int state) {
    static size_t list_index;
    static size_t len;
    static std::vector<std::string> matches;

    if (!state) {
        list_index = 0;
        len = strlen(text);
        matches.clear();

        // Check built-in and known commands
        for (const auto& cmd : known_commands) {
            if (cmd.compare(0, len, text) == 0) {
                matches.push_back(cmd);
            }
        }

        // Also check executables in current directory
        DIR* dir = opendir(".");
        if (dir) {
            struct dirent* entry;
            while ((entry = readdir(dir)) != nullptr) {
                std::string name = entry->d_name;
                if (name == "." || name == "..") continue;
                if (name.compare(0, len, text) == 0) {
                    struct stat st;
                    if (stat(name.c_str(), &st) == 0 && (st.st_mode & S_IXUSR)) {
                        matches.push_back(name);
                    }
                }
            }
            closedir(dir);
        }
    }

    if (list_index < matches.size()) {
        return strdup(matches[list_index++].c_str());
    }

    return nullptr;
}

// Custom autocomplete function for GNU Readline
char** autocomplete(const char* text, int start, int end) {
    (void)end;
    // Tell readline not to use default filename completion on command position unless matches found
    rl_attempted_completion_over = 0;

    if (start == 0) {
        // First word is a command
        return rl_completion_matches(text, suggest_match);
    } else {
        // Subsequent words are file/directory arguments
        return rl_completion_matches(text, rl_filename_completion_function);
    }
}

// Initialize tab completion
void setup_autocomplete() {
    rl_attempted_completion_function = autocomplete;
}
