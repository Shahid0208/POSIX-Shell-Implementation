#include "fun.h"

// Check if redirection characters exist outside of quotes
bool DetectRedirection(const std::string& cmd) {
    bool in_single_quote = false;
    bool in_double_quote = false;
    bool escape_next = false;

    for (size_t i = 0; i < cmd.length(); ++i) {
        char c = cmd[i];

        if (escape_next) {
            escape_next = false;
            continue;
        }

        if (c == '\\') {
            escape_next = true;
            continue;
        }

        if (c == '\'' && !in_double_quote) {
            in_single_quote = !in_single_quote;
        } else if (c == '"' && !in_single_quote) {
            in_double_quote = !in_double_quote;
        } else if (!in_single_quote && !in_double_quote) {
            if (c == '<' || c == '>') {
                return true;
            }
        }
    }
    return false;
}

// Parses redirection operators, applies dup2, and returns clean command tokens
int apply_redirection(const std::string& cmd, std::vector<std::string>& clean_args) {
    std::string input_file;
    std::string output_file;
    bool append_output = false;

    // Tokenize with awareness of <, >, >> as separate tokens even if adjacent
    std::vector<std::string> raw_tokens;
    std::string current;
    bool in_single_quote = false;
    bool in_double_quote = false;
    bool escape_next = false;

    for (size_t i = 0; i < cmd.length(); ++i) {
        char c = cmd[i];

        if (escape_next) {
            current += c;
            escape_next = false;
            continue;
        }

        if (c == '\\') {
            escape_next = true;
            continue;
        }

        if (c == '\'' && !in_double_quote) {
            in_single_quote = !in_single_quote;
            continue;
        } else if (c == '"' && !in_single_quote) {
            in_double_quote = !in_double_quote;
            continue;
        }

        if (!in_single_quote && !in_double_quote) {
            if (c == '<') {
                if (!current.empty()) {
                    raw_tokens.push_back(current);
                    current.clear();
                }
                raw_tokens.push_back("<");
                continue;
            } else if (c == '>') {
                if (!current.empty()) {
                    raw_tokens.push_back(current);
                    current.clear();
                }
                if (i + 1 < cmd.length() && cmd[i + 1] == '>') {
                    raw_tokens.push_back(">>");
                    i++;
                } else {
                    raw_tokens.push_back(">");
                }
                continue;
            } else if (c == ' ' || c == '\t' || c == '\n') {
                if (!current.empty()) {
                    raw_tokens.push_back(current);
                    current.clear();
                }
                continue;
            }
        }

        current += c;
    }

    if (!current.empty()) {
        raw_tokens.push_back(current);
    }

    clean_args.clear();

    for (size_t i = 0; i < raw_tokens.size(); ++i) {
        if (raw_tokens[i] == "<") {
            if (i + 1 >= raw_tokens.size()) {
                std::cerr << "mysh: syntax error near unexpected token 'newline'\n";
                return -1;
            }
            input_file = raw_tokens[++i];
        } else if (raw_tokens[i] == ">") {
            if (i + 1 >= raw_tokens.size()) {
                std::cerr << "mysh: syntax error near unexpected token 'newline'\n";
                return -1;
            }
            output_file = raw_tokens[++i];
            append_output = false;
        } else if (raw_tokens[i] == ">>") {
            if (i + 1 >= raw_tokens.size()) {
                std::cerr << "mysh: syntax error near unexpected token 'newline'\n";
                return -1;
            }
            output_file = raw_tokens[++i];
            append_output = true;
        } else {
            clean_args.push_back(raw_tokens[i]);
        }
    }

    if (clean_args.empty()) {
        return -1;
    }

    // Apply Input Redirection
    if (!input_file.empty()) {
        int fd_in = open(input_file.c_str(), O_RDONLY);
        if (fd_in < 0) {
            perror(("mysh: " + input_file).c_str());
            return -1;
        }
        if (dup2(fd_in, STDIN_FILENO) < 0) {
            perror("mysh: dup2 stdin");
            close(fd_in);
            return -1;
        }
        close(fd_in);
    }

    // Apply Output Redirection
    if (!output_file.empty()) {
        int flags = O_WRONLY | O_CREAT | (append_output ? O_APPEND : O_TRUNC);
        int fd_out = open(output_file.c_str(), flags, 0644);
        if (fd_out < 0) {
            perror(("mysh: " + output_file).c_str());
            return -1;
        }
        if (dup2(fd_out, STDOUT_FILENO) < 0) {
            perror("mysh: dup2 stdout");
            close(fd_out);
            return -1;
        }
        close(fd_out);
    }

    return 0;
}

// Executes a command with I/O redirection in a separate child process
void RunRedirection(const std::string& cmd) {
    std::string trimmed = handleSpace(cmd);
    if (trimmed.empty()) return;

    bool is_bg = false;
    if (trimmed.back() == '&') {
        is_bg = true;
        trimmed.pop_back();
        trimmed = handleSpace(trimmed);
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        // Child process
        if (is_bg) {
            setpgid(0, 0);
        }

        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);

        std::vector<std::string> clean_args;
        if (apply_redirection(trimmed, clean_args) < 0) {
            exit(1);
        }

        if (clean_args.empty()) {
            exit(0);
        }

        if (is_builtin(clean_args[0])) {
            int status = execute_builtin(clean_args);
            exit(status == 0 ? 0 : 1);
        } else {
            std::vector<char*> c_args;
            for (const auto& arg : clean_args) {
                c_args.push_back(const_cast<char*>(arg.c_str()));
            }
            c_args.push_back(nullptr);

            if (execvp(c_args[0], c_args.data()) < 0) {
                std::cerr << "mysh: command not found: " << clean_args[0] << std::endl;
                exit(127);
            }
        }
    } else {
        // Parent process
        if (is_bg) {
            setpgid(pid, pid);
            std::vector<std::string> raw_tokens = tokenizeArgs(trimmed);
            std::string proc_name = raw_tokens.empty() ? "redirect_bg" : raw_tokens[0];
            bg_processes.push_back({pid, proc_name});
            std::cout << "[" << bg_processes.size() << "] " << pid << std::endl;
        } else {
            fg_pid = pid;
            int status;
            waitpid(pid, &status, WUNTRACED);
            if (WIFSTOPPED(status)) {
                std::cout << "\n[" << pid << "] suspended" << std::endl;
                bg_processes.push_back({pid, trimmed});
            }
            fg_pid = 0;
        }
    }
}
