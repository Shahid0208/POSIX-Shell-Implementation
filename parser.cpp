#include "fun.h"

// Trims leading and trailing whitespace and tabs from a string
std::string handleSpace(const std::string& str) {
    if (str.empty()) return "";
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

// Splits input into multiple commands by ';' while respecting quotes and escapes
std::vector<std::string> splitCommands(const std::string& input) {
    std::vector<std::string> commands;
    std::string current;
    bool in_single_quote = false;
    bool in_double_quote = false;
    bool escape_next = false;

    for (size_t i = 0; i < input.length(); ++i) {
        char c = input[i];

        if (escape_next) {
            current += c;
            escape_next = false;
            continue;
        }

        if (c == '\\') {
            escape_next = true;
            current += c;
            continue;
        }

        if (c == '\'' && !in_double_quote) {
            in_single_quote = !in_single_quote;
            current += c;
        } else if (c == '"' && !in_single_quote) {
            in_double_quote = !in_double_quote;
            current += c;
        } else if (c == ';' && !in_single_quote && !in_double_quote) {
            std::string trimmed = handleSpace(current);
            if (!trimmed.empty()) {
                commands.push_back(trimmed);
            }
            current.clear();
        } else {
            current += c;
        }
    }

    std::string trimmed = handleSpace(current);
    if (!trimmed.empty()) {
        commands.push_back(trimmed);
    }

    return commands;
}

// Tokenizes a command into arguments respecting quotes and escape characters
std::vector<std::string> tokenizeArgs(const std::string& cmd) {
    std::vector<std::string> tokens;
    std::string token;
    bool in_single_quote = false;
    bool in_double_quote = false;
    bool escape_next = false;

    for (size_t i = 0; i < cmd.length(); ++i) {
        char c = cmd[i];

        if (escape_next) {
            token += c;
            escape_next = false;
            continue;
        }

        if (c == '\\') {
            escape_next = true;
            continue;
        }

        if (c == '\'' && !in_double_quote) {
            in_single_quote = !in_single_quote;
            continue; // strip quote
        } else if (c == '"' && !in_single_quote) {
            in_double_quote = !in_double_quote;
            continue; // strip quote
        } else if ((c == ' ' || c == '\t' || c == '\n') && !in_single_quote && !in_double_quote) {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
        } else {
            token += c;
        }
    }

    if (!token.empty()) {
        tokens.push_back(token);
    }

    return tokens;
}
