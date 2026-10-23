#include "fun.h"
#include <iomanip>

// Helper to format permissions string from mode_t
static std::string formatPermissions(mode_t mode) {
    std::string perms = "----------";
    if (S_ISDIR(mode)) perms[0] = 'd';
    else if (S_ISLNK(mode)) perms[0] = 'l';
    else if (S_ISCHR(mode)) perms[0] = 'c';
    else if (S_ISBLK(mode)) perms[0] = 'b';
    else if (S_ISFIFO(mode)) perms[0] = 'p';
    else if (S_ISSOCK(mode)) perms[0] = 's';

    if (mode & S_IRUSR) perms[1] = 'r';
    if (mode & S_IWUSR) perms[2] = 'w';
    if (mode & S_IXUSR) perms[3] = (mode & S_ISUID) ? 's' : 'x';
    else if (mode & S_ISUID) perms[3] = 'S';

    if (mode & S_IRGRP) perms[4] = 'r';
    if (mode & S_IWGRP) perms[5] = 'w';
    if (mode & S_IXGRP) perms[6] = (mode & S_ISGID) ? 's' : 'x';
    else if (mode & S_ISGID) perms[6] = 'S';

    if (mode & S_IROTH) perms[7] = 'r';
    if (mode & S_IWOTH) perms[8] = 'w';
    if (mode & S_IXOTH) perms[9] = (mode & S_ISVTX) ? 't' : 'x';
    else if (mode & S_ISVTX) perms[9] = 'T';

    return perms;
}

// Helper to format modification time like standard ls -l
static std::string formatModTime(time_t mtime) {
    char buf[80];
    struct tm* timeinfo = localtime(&mtime);
    time_t now = time(nullptr);
    double diff = difftime(now, mtime);

    // If within last 6 months (approx 180 days) and not in future
    if (diff >= 0 && diff < 180 * 24 * 60 * 60) {
        strftime(buf, sizeof(buf), "%b %e %H:%M", timeinfo);
    } else {
        strftime(buf, sizeof(buf), "%b %e  %Y", timeinfo);
    }
    return std::string(buf);
}

// Print detailed info for a single file/entry in ls -l
static void printDetailedEntry(const std::string& full_path, const std::string& display_name) {
    struct stat st;
    if (lstat(full_path.c_str(), &st) != 0) {
        perror(("ls: " + display_name).c_str());
        return;
    }

    std::string perms = formatPermissions(st.st_mode);

    struct passwd* pw = getpwuid(st.st_uid);
    std::string owner = pw ? pw->pw_name : std::to_string(st.st_uid);

    struct group* gr = getgrgid(st.st_gid);
    std::string group = gr ? gr->gr_name : std::to_string(st.st_gid);

    std::string mod_time = formatModTime(st.st_mtime);

    std::cout << perms << " "
              << std::setw(3) << st.st_nlink << " "
              << std::setw(8) << owner << " "
              << std::setw(8) << group << " "
              << std::setw(8) << st.st_size << " "
              << mod_time << " "
              << display_name;

    if (S_ISLNK(st.st_mode)) {
        char link_target[4096];
        ssize_t len = readlink(full_path.c_str(), link_target, sizeof(link_target) - 1);
        if (len != -1) {
            link_target[len] = '\0';
            std::cout << " -> " << link_target;
        }
    }
    std::cout << "\n";
}

// Lists contents of a single directory or file
static void listTarget(const std::string& target, bool show_hidden, bool detailed, bool is_multi) {
    std::string resolved_path = target;
    if (resolved_path == "~" || resolved_path.rfind("~/", 0) == 0) {
        resolved_path = SHELL_HOME + resolved_path.substr(1);
    }

    struct stat st;
    if (lstat(resolved_path.c_str(), &st) != 0) {
        perror(("ls: cannot access '" + target + "'").c_str());
        return;
    }

    if (!S_ISDIR(st.st_mode)) {
        // Target is a single regular file/symlink
        if (detailed) {
            printDetailedEntry(resolved_path, target);
        } else {
            std::cout << target << "\n";
        }
        return;
    }

    // Target is a directory
    DIR* dir = opendir(resolved_path.c_str());
    if (!dir) {
        perror(("ls: cannot open directory '" + target + "'").c_str());
        return;
    }

    if (is_multi) {
        std::cout << target << ":\n";
    }

    std::vector<std::string> entries;
    struct dirent* entry;
    long long total_blocks = 0;

    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        if (!show_hidden && name[0] == '.') {
            continue;
        }
        entries.push_back(name);

        if (detailed) {
            std::string entry_path = resolved_path + "/" + name;
            struct stat entry_st;
            if (lstat(entry_path.c_str(), &entry_st) == 0) {
                total_blocks += entry_st.st_blocks;
            }
        }
    }
    closedir(dir);

    std::sort(entries.begin(), entries.end());

    if (detailed) {
        // total in 1K blocks (st_blocks is 512-byte blocks)
        std::cout << "total " << (total_blocks / 2) << "\n";
        for (const auto& name : entries) {
            std::string entry_path = resolved_path + "/" + name;
            printDetailedEntry(entry_path, name);
        }
    } else {
        for (size_t i = 0; i < entries.size(); ++i) {
            std::cout << entries[i] << "  ";
        }
        if (!entries.empty()) {
            std::cout << "\n";
        }
    }
}

// Myls entry function
void Myls(const std::vector<std::string>& args) {
    bool show_hidden = false;
    bool detailed = false;
    std::vector<std::string> targets;

    for (size_t i = 1; i < args.size(); ++i) {
        const std::string& arg = args[i];
        if (arg.length() > 1 && arg[0] == '-') {
            for (size_t j = 1; j < arg.length(); ++j) {
                if (arg[j] == 'a') show_hidden = true;
                else if (arg[j] == 'l') detailed = true;
                else {
                    std::cerr << "ls: invalid option -- '" << arg[j] << "'\n";
                    return;
                }
            }
        } else {
            targets.push_back(arg);
        }
    }

    if (targets.empty()) {
        targets.push_back(".");
    }

    bool is_multi = targets.size() > 1;
    for (size_t i = 0; i < targets.size(); ++i) {
        listTarget(targets[i], show_hidden, detailed, is_multi);
        if (is_multi && i + 1 < targets.size()) {
            std::cout << "\n";
        }
    }
}
