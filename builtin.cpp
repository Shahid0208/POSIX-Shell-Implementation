#include "fun.h"

// Check if a command is a built-in command
bool is_builtin(const std::string& cmd_name) {
    static const std::vector<std::string> builtins = {
        "cd", "pwd", "echo", "ls", "pinfo", "search", "history", "exit"
    };
    return std::find(builtins.begin(), builtins.end(), cmd_name) != builtins.end();
}

// Built-in cd command implementation
int builtin_cd(const std::vector<std::string>& args) {
    if (args.size() > 2) {
        std::cerr << "cd: too many arguments" << std::endl;
        return 1;
    }

    char current_cwd[4096];
    if (getcwd(current_cwd, sizeof(current_cwd)) == nullptr) {
        perror("cd getcwd");
        return 1;
    }

    std::string target_dir;
    bool print_after_cd = false;

    if (args.size() == 1 || args[1] == "~") {
        target_dir = SHELL_HOME;
    } else if (args[1] == "-") {
        if (PREV_DIR.empty()) {
            std::cerr << "cd: OLDPWD not set" << std::endl;
            return 1;
        }
        target_dir = PREV_DIR;
        print_after_cd = true;
    } else {
        target_dir = args[1];
        if (target_dir == "~" || target_dir.rfind("~/", 0) == 0) {
            target_dir = SHELL_HOME + target_dir.substr(1);
        }
    }

    if (chdir(target_dir.c_str()) != 0) {
        perror("cd");
        return 1;
    }

    // Update previous directory to where we were before chdir
    PREV_DIR = current_cwd;

    if (print_after_cd) {
        char new_cwd[4096];
        if (getcwd(new_cwd, sizeof(new_cwd)) != nullptr) {
            std::cout << new_cwd << std::endl;
        }
    }

    return 0;
}

// Built-in pwd command implementation
int builtin_pwd(const std::vector<std::string>& args) {
    (void)args;
    char cwd[4096];
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        std::cout << cwd << std::endl;
        return 0;
    } else {
        perror("pwd");
        return 1;
    }
}

// Built-in echo command implementation
int builtin_echo(const std::vector<std::string>& args) {
    for (size_t i = 1; i < args.size(); ++i) {
        std::cout << args[i];
        if (i + 1 < args.size()) {
            std::cout << " ";
        }
    }
    std::cout << std::endl;
    return 0;
}

// Built-in exit command implementation
int builtin_exit(const std::vector<std::string>& args) {
    (void)args;
    SaveHistory();
    exit(0);
}

// Central dispatcher for built-in commands
int execute_builtin(const std::vector<std::string>& args) {
    if (args.empty()) return -1;

    const std::string& cmd = args[0];

    if (cmd == "cd") {
        return builtin_cd(args);
    } else if (cmd == "pwd") {
        return builtin_pwd(args);
    } else if (cmd == "echo") {
        return builtin_echo(args);
    } else if (cmd == "ls") {
        Myls(args);
        return 0;
    } else if (cmd == "pinfo") {
        pinfo(args);
        return 0;
    } else if (cmd == "search") {
        runSearch(args);
        return 0;
    } else if (cmd == "history") {
        int count = 20;
        if (args.size() > 1) {
            try {
                count = std::stoi(args[1]);
            } catch (...) {
                count = 20;
            }
        }
        ShowHistory(count);
        return 0;
    } else if (cmd == "exit") {
        return builtin_exit(args);
    }

    return -1; // Not a built-in
}
