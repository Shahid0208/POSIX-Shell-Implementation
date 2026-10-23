# ASSIGNMENT-2: POSIX Shell Implementation in C++

This project is part of the **Advanced Operating Systems** assignment and implements a full-featured, modular, and lightweight POSIX shell (`mysh`) in C++.

The shell provides a standard Linux-like terminal interface where users can execute built-ins, run external programs in the foreground or background, chain multiple commands across pipelines, redirect input/output, navigate command history, use tab auto-completion, and handle terminal control signals gracefully.

---

## Table of Contents
1. [Feature Implementation Status](#feature-implementation-status)
2. [Architecture & File Breakdown](#architecture--file-breakdown)
3. [POSIX System Calls Used](#posix-system-calls-used)
4. [Testing & Automated Verification](#testing--automated-verification)
5. [Compilation & How to Run](#compilation--how-to-run)

---

## Feature Implementation Status

All mandatory and extended functionalities have been fully implemented, integrated, and verified against edge cases:

| Feature | Implementation Details | Status |
| :--- | :--- | :---: |
| **Shell Prompt** | Displays `<username@hostname:cwd>$ ` with `~` substitution for home directory and subdirectories. | **Implemented** |
| **Command Parsing & `;`** | Supports multiple commands per line using `;`, handling single quotes (`'...'`), double quotes (`"..."`), and escape characters (`\`). | **Implemented** |
| **Built-in: `cd`** | Supports `cd`, `cd .`, `cd ..`, `cd ~`, `cd -` (using `OLDPWD`), and relative/absolute directory paths. | **Implemented** |
| **Built-in: `pwd` & `echo`** | `pwd` prints current working directory; `echo` parses arguments with whitespace handling. | **Implemented** |
| **Built-in: `ls` (`Myls`)** | Custom implementation supporting flags `-a`, `-l`, combined flags (e.g. `-la`, `-al`), multiple directory/file targets, permissions string, symlink resolution, timestamps, and 1K-block counts. | **Implemented** |
| **Built-in: `pinfo`** | Reads `/proc/<pid>/stat` and `/proc/<pid>/exe` to report PID, Process Status (`+` for foreground), Virtual Memory (`vsize`), and executable path. Accurately handles process names containing spaces. | **Implemented** |
| **Built-in: `search`** | Recursive directory traversal starting from `.` to search for a target file or folder, outputting `True` or `False`. | **Implemented** |
| **Built-in: `history`** | Stores up to 20 commands, persists entries across sessions in `mysh_history.txt`, avoids consecutive duplicates, and supports `history <n>`. | **Implemented** |
| **Foreground & Background (`&`)** | Commands ending with `&` run in background with detached process groups (`setpgid`). Prints PID on launch, prevents background built-ins, and asynchronously reaps completed background jobs on subsequent prompt cycles. | **Implemented** |
| **I/O Redirection (`<`, `>`, `>>`)** | Supports input redirection (`<`), truncate output (`>`), and append output (`>>`), working for built-ins and external binaries. | **Implemented** |
| **Pipelines (`\|`)** | Arbitrary N-stage pipelines using `pipe()` and `dup2()`. Supports built-ins, external commands, and integrated per-stage redirection (`cat < in.txt \| grep word > out.txt`). | **Implemented** |
| **Signal Handling** | `Ctrl-C` (`SIGINT`) and `Ctrl-Z` (`SIGTSTP`) signals interrupt/suspend only the foreground process (`fg_pid`) without exiting the shell. `Ctrl-D` cleanly saves history and exits. | **Implemented** |
| **Tab Auto-Completion** | Custom GNU Readline generator for built-ins, common commands, and current directory executables, falling back to file/path completion for arguments. | **Implemented** |

---

## Architecture & File Breakdown

The codebase is organized into modular C++ source files linked via a central header `fun.h`:

```
├── main.cpp            # Shell REPL loop, prompt generator, signal dispatcher, background process reaper
├── fun.h               # Central declarations, global variables, and module interfaces
├── parser.cpp          # Whitespace trimming, quote-aware semicolon splitting, argument tokenization
├── builtin.cpp         # Built-in command dispatcher and implementations (cd, pwd, echo, exit)
├── ls.cpp              # Custom directory listing (Myls) with -a, -l, -la, file details, symlinks
├── fg_bg.cpp           # Process spawning, foreground tracking, background job management (CheckFgBg)
├── pinfo.cpp           # /proc parser for process state, virtual memory size, and executable link
├── search.cpp          # Recursive file and directory search starting from the working directory
├── io_redirection.cpp  # Parsing & execution of <, >, and >> operators via dup2
├── pipeline.cpp        # Multi-stage pipeline execution (|) with per-stage redirection support
├── autocomplete.cpp    # GNU Readline custom autocomplete for commands and filenames
├── history.cpp         # Command history persistence and management (mysh_history.txt)
├── Makefile            # Modular compilation script with -Wall -Wextra -std=c++17 -lreadline
└── test_shell.sh       # Automated bash test suite testing all 10 feature areas
```

### Module Descriptions

1. **`main.cpp`**
   - Initializes shell environment (`SHELL_HOME`, `HISTORY_FILE`, signal handlers, Readline completion).
   - Runs the main REPL loop using GNU `readline()`.
   - Checks and reaps completed background processes before rendering the prompt.
   - Formats the Linux prompt `<username@hostname:cwd>$ `.

2. **`parser.cpp`**
   - `splitCommands()`: Splits multi-command lines by `;` while respecting single quotes, double quotes, and escape characters.
   - `tokenizeArgs()`: Parses space-separated tokens and strips quote delimiters appropriately.
   - `handleSpace()`: Trims leading and trailing whitespace.

3. **`builtin.cpp`**
   - Implements `cd` with support for `~`, `-` (previous directory via `OLDPWD`), `..`, `.`, and relative/absolute paths.
   - Implements `pwd`, `echo`, `exit`, and dispatches to other built-in handlers.

4. **`ls.cpp`**
   - Implements `Myls()` matching standard `ls` behavior.
   - Supports `-a`, `-l`, combined flags (e.g., `-la`, `-al`), multiple directories, and regular files.
   - Formats file permissions, owner, group, size, modification time, and symlink targets.

5. **`fg_bg.cpp`**
   - Manages foreground execution (`waitpid()` tracking) and background execution (`setpgid()` detachment).
   - Reaps background child processes with exit codes and signal numbers asynchronously.

6. **`pinfo.cpp`**
   - Extracts process status (`R`, `S`, `Z`, `T`, with `+` for foreground), memory (`vsize`), and executable path (`readlink("/proc/<pid>/exe")`).

7. **`search.cpp`**
   - Recursively traverses subdirectories to locate a target file/folder name.

8. **`io_redirection.cpp`**
   - Validates syntax and applies `dup2()` redirection for `<`, `>`, and `>>`.
   - Safely executes commands in a child process so parent file descriptors remain intact.

9. **`pipeline.cpp`**
   - Splits commands by `|`, creates necessary pipe pairs, connects `stdin`/`stdout` between stages, and closes unused descriptors.
   - Integrates `apply_redirection()` to support redirections within any pipeline stage.

10. **`autocomplete.cpp`**
    - Hooks into GNU Readline's `rl_attempted_completion_function`.
    - Auto-completes built-in/common commands on the first token and filenames on subsequent arguments.

11. **`history.cpp`**
    - Persists the last 20 unique commands in `mysh_history.txt`.
    - Automatically loads history at startup and saves on changes/exit.

---

## POSIX System Calls Used

| Category | System Calls | Purpose in Shell |
| :--- | :--- | :--- |
| **Process Control** | `fork()`, `execvp()`, `waitpid()`, `setpgid()`, `getpid()`, `exit()` | Process creation, program execution, background job detachment, process reaping. |
| **Pipelines & IPC** | `pipe()`, `dup2()`, `close()` | Chaining standard streams across pipeline stages. |
| **File Redirection** | `open()`, `close()`, `dup2()` | Redirecting `stdin` (`O_RDONLY`), `stdout` truncate (`O_TRUNC`), and append (`O_APPEND`). |
| **Filesystem & Info** | `chdir()`, `getcwd()`, `opendir()`, `readdir()`, `closedir()`, `lstat()`, `readlink()` | Directory navigation, `ls` listing, `search` recursion, and process path resolution. |
| **User & Host Info** | `getuid()`, `getpwuid()`, `getgrgid()`, `gethostname()`, `getenv()` | Generating shell prompt and file ownership information in `ls -l`. |
| **Signals** | `sigaction()`, `signal()`, `kill()`, `sigemptyset()` | Handling `SIGINT` (Ctrl-C) and `SIGTSTP` (Ctrl-Z) for foreground processes. |

---

## Testing & Automated Verification

An automated test suite is provided in `test_shell.sh` covering 10 test scenarios:

1. **Test 1: Basic Built-ins** (`pwd`, `echo`, `cd ..`, `cd -`, `exit`)
2. **Test 2: Semicolon Command Sequences** (`echo first; echo second; echo third`)
3. **Test 3: I/O Redirection** (`>`, `<`, `>>`)
4. **Test 4: Multi-stage Pipelines** (`ls -l | grep -E '\.cpp|\.h' | wc -l`)
5. **Test 5: Pipelines with Redirection** (`cat < in.txt | grep ap > out.txt`)
6. **Test 6: Search Command** (`search main.cpp`, `search non_existent_file.txt`)
7. **Test 7: Process Info** (`pinfo`)
8. **Test 8: Custom Directory Listing** (`ls -a`, `ls -l`, `ls -la`)
9. **Test 9: Background Process Management** (`sleep 1 &`, reaping check)
10. **Test 10: Command History** (`history 5`)

To run the automated test suite:
```bash
chmod +x test_shell.sh
./test_shell.sh
```

---

## Compilation & How to Run

### Requirements
- `g++` with C++17 support
- GNU `readline` library (`libreadline-dev` on Debian/Ubuntu)
- `make`

### Building the Shell
```bash
make clean
make
```

### Running the Shell
```bash
./mysh
```
