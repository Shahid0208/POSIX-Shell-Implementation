#include "fun.h"

// Check if a pipeline character '|' exists outside quotes
bool DetectPipeline(const std::string& cmd) {
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
            if (c == '|') {
                return true;
            }
        }
    }
    return false;
}

// Splits a pipeline command string by '|' outside quotes
static std::vector<std::string> splitPipeline(const std::string& cmd) {
    std::vector<std::string> stages;
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
            current += c;
            continue;
        }

        if (c == '\'' && !in_double_quote) {
            in_single_quote = !in_single_quote;
            current += c;
        } else if (c == '"' && !in_single_quote) {
            in_double_quote = !in_double_quote;
            current += c;
        } else if (c == '|' && !in_single_quote && !in_double_quote) {
            std::string trimmed = handleSpace(current);
            if (!trimmed.empty()) {
                stages.push_back(trimmed);
            }
            current.clear();
        } else {
            current += c;
        }
    }

    std::string trimmed = handleSpace(current);
    if (!trimmed.empty()) {
        stages.push_back(trimmed);
    }

    return stages;
}

// Executes multi-stage pipelines
void RunPipeline(const std::string& cmd) {
    std::vector<std::string> stages = splitPipeline(cmd);
    size_t num_cmds = stages.size();

    if (num_cmds == 0) return;
    if (num_cmds == 1) {
        // Only one command, fallback to redirection or regular execution
        if (DetectRedirection(stages[0])) {
            RunRedirection(stages[0]);
        } else {
            CheckFgBg(stages[0]);
        }
        return;
    }

    int num_pipes = static_cast<int>(num_cmds - 1);
    std::vector<int> pipefds(2 * num_pipes);

    for (int i = 0; i < num_pipes; ++i) {
        if (pipe(pipefds.data() + i * 2) < 0) {
            perror("pipe");
            return;
        }
    }

    std::vector<pid_t> pids(num_cmds);

    for (size_t i = 0; i < num_cmds; ++i) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork in pipeline");
            return;
        }

        if (pid == 0) {
            // Child process for stage i
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);

            // Connect input from previous stage
            if (i > 0) {
                if (dup2(pipefds[(i - 1) * 2], STDIN_FILENO) < 0) {
                    perror("dup2 pipeline stdin");
                    exit(1);
                }
            }

            // Connect output to next stage
            if (i < num_cmds - 1) {
                if (dup2(pipefds[i * 2 + 1], STDOUT_FILENO) < 0) {
                    perror("dup2 pipeline stdout");
                    exit(1);
                }
            }

            // Close all pipe copies in child
            for (int j = 0; j < 2 * num_pipes; ++j) {
                close(pipefds[j]);
            }

            // Handle possible I/O redirection within this pipeline command stage
            std::vector<std::string> clean_args;
            if (DetectRedirection(stages[i])) {
                if (apply_redirection(stages[i], clean_args) < 0) {
                    exit(1);
                }
            } else {
                clean_args = tokenizeArgs(stages[i]);
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
            pids[i] = pid;
        }
    }

    // Parent closes all pipe file descriptors
    for (int j = 0; j < 2 * num_pipes; ++j) {
        close(pipefds[j]);
    }

    // Track the last stage as the foreground process
    fg_pid = pids.back();

    // Wait for all child processes to complete
    for (size_t i = 0; i < num_cmds; ++i) {
        int status;
        waitpid(pids[i], &status, WUNTRACED);
    }

    fg_pid = 0;
}
