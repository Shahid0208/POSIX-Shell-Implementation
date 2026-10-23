#include "fun.h"
#include <fstream>
#include <deque>

static std::deque<std::string> history_entries;
static const size_t MAX_HISTORY = 20;

// Loads command history from mysh_history.txt
void LoadHistory() {
    history_entries.clear();
    std::ifstream infile(HISTORY_FILE);
    if (!infile.is_open()) {
        return;
    }

    std::string line;
    while (std::getline(infile, line)) {
        std::string trimmed = handleSpace(line);
        if (!trimmed.empty()) {
            history_entries.push_back(trimmed);
            add_history(trimmed.c_str());
        }
    }
    infile.close();

    while (history_entries.size() > MAX_HISTORY) {
        history_entries.pop_front();
    }
}

// Saves command history to mysh_history.txt
void SaveHistory() {
    std::ofstream outfile(HISTORY_FILE, std::ios::trunc);
    if (!outfile.is_open()) {
        return;
    }

    for (const auto& cmd : history_entries) {
        outfile << cmd << "\n";
    }
    outfile.close();
}

// Adds a new command to history (ignoring consecutive duplicates & empty lines)
void AddHistory(const std::string& cmd) {
    std::string trimmed = handleSpace(cmd);
    if (trimmed.empty()) return;

    if (!history_entries.empty() && history_entries.back() == trimmed) {
        return; // Don't add consecutive duplicates
    }

    history_entries.push_back(trimmed);
    add_history(trimmed.c_str());

    if (history_entries.size() > MAX_HISTORY) {
        history_entries.pop_front();
    }

    SaveHistory();
}

// Displays the previous commands
void ShowHistory(int count) {
    if (count <= 0) count = static_cast<int>(MAX_HISTORY);
    int total = static_cast<int>(history_entries.size());
    int start_idx = std::max(0, total - count);

    for (int i = start_idx; i < total; ++i) {
        std::cout << (i + 1) << "  " << history_entries[i] << "\n";
    }
}
