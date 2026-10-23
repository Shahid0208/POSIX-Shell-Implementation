#include "fun.h"

// Definition of global variables
pid_t fg_pid = 0;
std::string SHELL_HOME = "";
std::string PREV_DIR = "";
std::string HISTORY_FILE = "";
std::vector<std::pair<pid_t, std::string>> bg_processes;

// Signal handler for SIGINT (Ctrl-C)
void sigint_handler(int sig) {
    (void)sig;
    if (fg_pid > 0) {
        kill(fg_pid, SIGINT);
    } else {
        std::cout << "\n";
        rl_on_new_line();
        rl_replace_line("", 0);
        rl_redisplay();
    }
}

// Signal handler for SIGTSTP (Ctrl-Z)
void sigtstp_handler(int sig) {
    (void)sig;
    if (fg_pid > 0) {
        kill(fg_pid, SIGTSTP);
    } else {
        std::cout << "\n";
        rl_on_new_line();
        rl_replace_line("", 0);
        rl_redisplay();
    }
}

// Setup signal handling for the shell
void setup_signal_handlers() {
    struct sigaction sa_int;
    sa_int.sa_handler = sigint_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa_int, nullptr);

    struct sigaction sa_tstp;
    sa_tstp.sa_handler = sigtstp_handler;
    sigemptyset(&sa_tstp.sa_mask);
    sa_tstp.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &sa_tstp, nullptr);

    // Ignore terminal stop signals in parent shell
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
}

// Helper to format path substituting SHELL_HOME with ~
std::string formatPathWithTilde(const std::string& path) {
    if (path == SHELL_HOME) {
        return "~";
    }
    if (path.rfind(SHELL_HOME + "/", 0) == 0) {
        return "~" + path.substr(SHELL_HOME.length());
    }
    return path;
}

// Generate the Linux-like formatted shell prompt
std::string getPrompt() {
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) != 0) {
        strncpy(hostname, "unknown", sizeof(hostname));
    }

    std::string username;
    uid_t uid = getuid();
    struct passwd* pw = getpwuid(uid);
    if (pw && pw->pw_name) {
        username = pw->pw_name;
    } else {
        char* user_env = getenv("USER");
        username = user_env ? user_env : "user";
    }

    char cwd[4096];
    std::string cwd_str;
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        cwd_str = formatPathWithTilde(cwd);
    } else {
        cwd_str = "?";
    }

    return "<" + username + "@" + hostname + ":" + cwd_str + ">$ ";
}

int main() {
    // Initialize shell home directory
    char cwd_buf[4096];
    if (getcwd(cwd_buf, sizeof(cwd_buf)) != nullptr) {
        SHELL_HOME = cwd_buf;
    } else {
        SHELL_HOME = "/";
    }

    PREV_DIR = "";
    HISTORY_FILE = SHELL_HOME + "/mysh_history.txt";

    // Setup signals, autocomplete, and history
    setup_signal_handlers();
    setup_autocomplete();
    LoadHistory();

    // Main Shell REPL Loop
    while (true) {
        // Asynchronously check and reap any completed background processes
        checkBackgroundProcesses();

        std::string prompt = getPrompt();
        char* input_line = readline(prompt.c_str());

        // Handle Ctrl-D (EOF)
        if (input_line == nullptr) {
            std::cout << "\n";
            SaveHistory();
            break;
        }

        std::string input_str(input_line);
        std::string trimmed_input = handleSpace(input_str);

        if (trimmed_input.empty()) {
            free(input_line);
            continue;
        }

        // Add non-empty command to history
        AddHistory(trimmed_input);

        // Split multiple commands separated by ';'
        std::vector<std::string> commands = splitCommands(trimmed_input);

        for (const auto& cmd_raw : commands) {
            std::string cmd = handleSpace(cmd_raw);
            if (cmd.empty()) continue;

            if (DetectPipeline(cmd)) {
                RunPipeline(cmd);
            } else if (DetectRedirection(cmd)) {
                RunRedirection(cmd);
            } else {
                std::vector<std::string> args = tokenizeArgs(cmd);
                if (args.empty()) continue;

                if (is_builtin(args[0])) {
                    execute_builtin(args);
                } else {
                    CheckFgBg(cmd);
                }
            }
        }

        free(input_line);
    }

    return 0;
}
