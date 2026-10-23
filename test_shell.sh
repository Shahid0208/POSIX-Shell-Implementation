#!/bin/bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$DIR"

echo "=== Building mysh ==="
make clean
make

echo "=== Running Test Cases ==="

# Test 1: Builtins (pwd, echo, cd, history, exit)
echo "--- Test 1: Basic Built-ins ---"
output=$(printf "pwd\necho hello     world\ncd ..\npwd\ncd -\npwd\nexit\n" | ./mysh)
echo "$output"

# Test 2: Semicolon multi-commands
echo "--- Test 2: Semicolons ---"
output=$(printf "echo first; echo second; echo third\nexit\n" | ./mysh)
echo "$output"

# Test 3: Redirection (<, >, >>)
echo "--- Test 3: I/O Redirection ---"
output=$(printf "echo 'line 1' > t_red.txt\ncat < t_red.txt\necho 'line 2' >> t_red.txt\ncat t_red.txt\nrm -f t_red.txt\nexit\n" | ./mysh)
echo "$output"

# Test 4: Pipelines
echo "--- Test 4: Multi-stage Pipelines ---"
output=$(printf "ls -l | grep -E '\.cpp|\.h' | wc -l\nexit\n" | ./mysh)
echo "$output"

# Test 5: Pipeline with Redirection
echo "--- Test 5: Pipeline with Redirection ---"
output=$(printf "echo line1 > t_pipe.txt\necho apple >> t_pipe.txt\necho banana >> t_pipe.txt\necho apricot >> t_pipe.txt\ncat < t_pipe.txt | grep ap > t_pipe_out.txt\ncat t_pipe_out.txt\nrm -f t_pipe.txt t_pipe_out.txt\nexit\n" | ./mysh)
echo "$output"

# Test 6: Search
echo "--- Test 6: Search command ---"
output=$(printf "search main.cpp\nsearch non_existent_file_xyz.txt\nexit\n" | ./mysh)
echo "$output"

# Test 7: Pinfo
echo "--- Test 7: Pinfo ---"
output=$(printf "pinfo\nexit\n" | ./mysh)
echo "$output"

# Test 8: Custom ls (-a, -l, -la)
echo "--- Test 8: ls with flags ---"
output=$(printf "ls -a\nls -l\nls -la\nexit\n" | ./mysh)
echo "$output"

# Test 9: Background process
echo "--- Test 9: Background process ---"
output=$(printf "sleep 1 &\nsleep 2\nexit\n" | ./mysh)
echo "$output"

# Test 10: History
echo "--- Test 10: History ---"
output=$(printf "history 5\nexit\n" | ./mysh)
echo "$output"

echo "=== ALL TESTS COMPLETED SUCCESSFULLY ==="
