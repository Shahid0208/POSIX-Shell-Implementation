#include "fun.h"
#include <fstream>

void pinfo(const std::vector<std::string>& args) {
    pid_t target_pid;

    if (args.size() == 1) {
        target_pid = getpid();
    } else if (args.size() == 2) {
        try {
            target_pid = std::stoi(args[1]);
        } catch (...) {
            std::cerr << "pinfo: invalid pid '" << args[1] << "'\n";
            return;
        }
    } else {
        std::cerr << "pinfo: too many arguments\n";
        return;
    }

    std::string stat_path = "/proc/" + std::to_string(target_pid) + "/stat";
    std::ifstream stat_file(stat_path);
    if (!stat_file.is_open()) {
        std::cerr << "pinfo: process with pid " << target_pid << " does not exist or cannot be accessed.\n";
        return;
    }

    // Read the contents of /proc/<pid>/stat
    std::string stat_content;
    std::getline(stat_file, stat_content);
    stat_file.close();

    // The comm field (process name) is enclosed in parentheses, which may contain spaces.
    size_t last_paren = stat_content.rfind(')');
    if (last_paren == std::string::npos) {
        std::cerr << "pinfo: failed to parse /proc/" << target_pid << "/stat\n";
        return;
    }

    std::string rest = stat_content.substr(last_paren + 2); // skip ") "
    std::istringstream iss(rest);

    char state_char;
    int ppid, pgrp, session, tty_nr, tpgid;
    unsigned int flags;
    unsigned long minflt, cminflt, majflt, cmajflt, utime, stime;
    long cutime, cstime, priority, nice, num_threads, itrealvalue;
    unsigned long long starttime;
    unsigned long vsize = 0;

    iss >> state_char >> ppid >> pgrp >> session >> tty_nr >> tpgid
        >> flags >> minflt >> cminflt >> majflt >> cmajflt >> utime >> stime
        >> cutime >> cstime >> priority >> nice >> num_threads >> itrealvalue
        >> starttime >> vsize;

    std::string process_status(1, state_char);
    if (pgrp == tpgid && tpgid != -1) {
        process_status += "+";
    }

    // Read executable path via /proc/<pid>/exe
    std::string exe_path = "/proc/" + std::to_string(target_pid) + "/exe";
    char buffer[4096];
    ssize_t len = readlink(exe_path.c_str(), buffer, sizeof(buffer) - 1);
    std::string exec_path_str;

    if (len != -1) {
        buffer[len] = '\0';
        exec_path_str = formatPathWithTilde(buffer);
    } else {
        exec_path_str = "Executable Path Not Accessible";
    }

    std::cout << "pid -- " << target_pid << "\n";
    std::cout << "Process Status -- " << process_status << "\n";
    std::cout << "memory -- " << vsize << " {Virtual Memory}\n";
    std::cout << "Executable Path -- " << exec_path_str << "\n";
}
