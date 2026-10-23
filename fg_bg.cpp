#include "fun.h"

// Check and reap terminated background child processes
void checkBackgroundProcesses() {
    int status;
    pid_t pid;

    for (auto it = bg_processes.begin(); it != bg_processes.end(); ) {
        pid = waitpid(it->first, &status, WNOHANG | WUNTRACED);
        if (pid > 0) {
            if (WIFEXITED(status)) {
                int exit_code = WEXITSTATUS(status);
                std::cout << "\n[" << it->first << "] (" << it->second << ") exited normally (exit code " << exit_code << ")" << std::endl;
            } else if (WIFSIGNALED(status)) {
                int term_sig = WTERMSIG(status);
                std::cout << "\n[" << it->first << "] (" << it->second << ") terminated by signal " << term_sig << std::endl;
            } else if (WIFSTOPPED(status)) {
                int stop_sig = WSTOPSIG(status);
                std::cout << "\n[" << it->first << "] (" << it->second << ") stopped by signal " << stop_sig << std::endl;
                ++it;
                continue;
            }
            it = bg_processes.erase(it);
        } else {
            ++it;
        }
    }
}

// Executes an external command in foreground or background
void executeExternal(const std::vector<std::string>& args, bool is_bg) {
    if (args.empty()) return;

    if (is_bg && is_builtin(args[0])) {
        std::cerr << "Error: Built-in commands cannot be executed in the background." << std::endl;
        return;
    }

    std::vector<char*> c_args;
    for (const auto& arg : args) {
        c_args.push_back(const_cast<char*>(arg.c_str()));
    }
    c_args.push_back(nullptr);

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        // Child Process
        if (is_bg) {
            // Detach child from terminal's process group so it doesn't receive terminal signals
            setpgid(0, 0);
        }

        // Reset signal handling to default for external commands
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);

        if (execvp(c_args[0], c_args.data()) < 0) {
            std::cerr << "mysh: command not found: " << args[0] << std::endl;
            exit(127);
        }
    } else {
        // Parent Process
        if (is_bg) {
            setpgid(pid, pid);
            bg_processes.push_back({pid, args[0]});
            std::cout << "[" << bg_processes.size() << "] " << pid << std::endl;
        } else {
            fg_pid = pid;
            int status;
            waitpid(pid, &status, WUNTRACED);

            if (WIFSTOPPED(status)) {
                std::cout << "\n[" << pid << "] suspended" << std::endl;
                bg_processes.push_back({pid, args[0]});
            }
            fg_pid = 0;
        }
    }
}

// Inspects command for '&' and dispatches foreground / background execution
void CheckFgBg(const std::string& cmd) {
    std::string trimmed = handleSpace(cmd);
    if (trimmed.empty()) return;

    bool is_bg = false;
    if (trimmed.back() == '&') {
        is_bg = true;
        trimmed.pop_back();
        trimmed = handleSpace(trimmed);
    }

    if (trimmed.empty()) return;

    std::vector<std::string> args = tokenizeArgs(trimmed);
    if (args.empty()) return;

    if (is_builtin(args[0])) {
        if (is_bg) {
            std::cerr << "Error: Built-in commands cannot be executed in the background." << std::endl;
        } else {
            execute_builtin(args);
        }
    } else {
        executeExternal(args, is_bg);
    }
}
