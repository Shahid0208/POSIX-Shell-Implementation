#ifndef FUN_H
#define FUN_H

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <ctime>
#include <csignal>
#include <algorithm>
#include <readline/readline.h>
#include <readline/history.h>

// Global tracking variables
extern pid_t fg_pid;
extern std::string SHELL_HOME;
extern std::string PREV_DIR;
extern std::string HISTORY_FILE;
extern std::vector<std::pair<pid_t, std::string>> bg_processes;

// Structure to hold background process information
struct BgProcess {
    pid_t pid;
    std::string command;
};

// Prompt & General Helpers
std::string getPrompt();
std::string formatPathWithTilde(const std::string& path);
void checkBackgroundProcesses();

// Parser & Tokenizer (parser.cpp)
std::vector<std::string> splitCommands(const std::string& input);
std::string handleSpace(const std::string& str);
std::vector<std::string> tokenizeArgs(const std::string& cmd);

// Built-in Commands (builtin.cpp)
bool is_builtin(const std::string& cmd_name);
int execute_builtin(const std::vector<std::string>& args);
int builtin_cd(const std::vector<std::string>& args);
int builtin_pwd(const std::vector<std::string>& args);
int builtin_echo(const std::vector<std::string>& args);
int builtin_exit(const std::vector<std::string>& args);

// Directory Listing (ls.cpp)
void Myls(const std::vector<std::string>& args);

// Process Info (pinfo.cpp)
void pinfo(const std::vector<std::string>& args);

// Search (search.cpp)
bool searchFile(const std::string& target, const std::string& currentDir = ".");
void runSearch(const std::vector<std::string>& args);

// I/O Redirection (io_redirection.cpp)
bool DetectRedirection(const std::string& cmd);
int apply_redirection(const std::string& cmd, std::vector<std::string>& clean_args);
void RunRedirection(const std::string& cmd);

// Pipelines (pipeline.cpp)
bool DetectPipeline(const std::string& cmd);
void RunPipeline(const std::string& cmd);

// Foreground & Background (fg_bg.cpp)
void CheckFgBg(const std::string& cmd);
void executeExternal(const std::vector<std::string>& args, bool is_bg);

// Tab Auto-Completion (autocomplete.cpp)
void setup_autocomplete();
char** autocomplete(const char* text, int start, int end);
char* suggest_match(const char* text, int state);

// Command History (history.cpp)
void LoadHistory();
void AddHistory(const std::string& cmd);
void ShowHistory(int count = 20);
void SaveHistory();

// Signal Handlers
void sigint_handler(int sig);
void sigtstp_handler(int sig);
void setup_signal_handlers();

#endif // FUN_H
