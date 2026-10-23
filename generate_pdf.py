import sys
import os
from reportlab.lib.pagesizes import letter
from reportlab.lib import colors
from reportlab.lib.units import inch
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.platypus import (
    SimpleDocTemplate, Paragraph, Spacer, Table, TableStyle, PageBreak, KeepTogether, HRFlowable
)
from reportlab.pdfgen import canvas

class NumberedCanvas(canvas.Canvas):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._saved_page_states = []

    def showPage(self):
        self._saved_page_states.append(dict(self.__dict__))
        self._startPage()

    def save(self):
        num_pages = len(self._saved_page_states)
        for state in self._saved_page_states:
            self.__dict__.update(state)
            self.draw_header_footer(num_pages)
            super().showPage()
        super().save()

    def draw_header_footer(self, page_count):
        self.saveState()
        self.setFont("Helvetica", 8)
        self.setFillColor(colors.HexColor("#555555"))
        
        # Header (pages > 1)
        if self._pageNumber > 1:
            self.drawString(54, 750, "POSIX Shell (mysh) — Architecture & Technical Documentation")
            self.setStrokeColor(colors.HexColor("#D0D7DE"))
            self.setLineWidth(0.5)
            self.line(54, 743, letter[0] - 54, 743)
        
        # Footer
        footer_text = f"Page {self._pageNumber} of {page_count}"
        self.drawRightString(letter[0] - 54, 36, footer_text)
        self.drawString(54, 36, "Advanced Operating Systems | Assignment-2")
        self.setStrokeColor(colors.HexColor("#D0D7DE"))
        self.setLineWidth(0.5)
        self.line(54, 48, letter[0] - 54, 48)
        
        self.restoreState()

def create_pdf(filename):
    doc = SimpleDocTemplate(
        filename,
        pagesize=letter,
        leftMargin=54,
        rightMargin=54,
        topMargin=60,
        bottomMargin=60
    )

    styles = getSampleStyleSheet()
    
    primary_color = colors.HexColor("#0969DA")
    text_dark = colors.HexColor("#24292F")
    
    title_style = ParagraphStyle(
        'DocTitle',
        parent=styles['Normal'],
        fontName='Helvetica-Bold',
        fontSize=24,
        leading=28,
        textColor=colors.HexColor("#0F172A"),
        alignment=0,
        spaceAfter=6
    )
    
    subtitle_style = ParagraphStyle(
        'DocSubtitle',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=12,
        leading=16,
        textColor=colors.HexColor("#475569"),
        spaceAfter=15
    )

    h1_style = ParagraphStyle(
        'Heading1_Custom',
        parent=styles['Normal'],
        fontName='Helvetica-Bold',
        fontSize=15,
        leading=19,
        textColor=colors.HexColor("#0F172A"),
        spaceBefore=14,
        spaceAfter=8,
        keepWithNext=True
    )

    h2_style = ParagraphStyle(
        'Heading2_Custom',
        parent=styles['Normal'],
        fontName='Helvetica-Bold',
        fontSize=11,
        leading=15,
        textColor=colors.HexColor("#1E293B"),
        spaceBefore=10,
        spaceAfter=4,
        keepWithNext=True
    )

    body_style = ParagraphStyle(
        'Body_Custom',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=9.5,
        leading=13.5,
        textColor=text_dark,
        spaceAfter=6
    )

    bullet_style = ParagraphStyle(
        'Bullet_Custom',
        parent=body_style,
        leftIndent=15,
        firstLineIndent=-10,
        spaceAfter=3
    )

    table_header_style = ParagraphStyle(
        'TableHeader',
        parent=styles['Normal'],
        fontName='Helvetica-Bold',
        fontSize=8.5,
        leading=11,
        textColor=colors.white
    )

    table_cell_style = ParagraphStyle(
        'TableCell',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=8,
        leading=10.5,
        textColor=text_dark
    )

    table_cell_bold = ParagraphStyle(
        'TableCellBold',
        parent=table_cell_style,
        fontName='Helvetica-Bold'
    )

    story = []

    # Title Block
    story.append(Paragraph("POSIX Shell Implementation in C++ (mysh)", title_style))
    story.append(Paragraph("<b>Course:</b> Advanced Operating Systems | <b>Project:</b> Lightweight Unix Shell (`mysh`) | <b>Language:</b> C++17", subtitle_style))
    story.append(HRFlowable(width="100%", thickness=1.5, color=primary_color, spaceBefore=0, spaceAfter=12))

    # Section 1: Executive Overview
    story.append(Paragraph("1. Executive Overview & Scope", h1_style))
    story.append(Paragraph(
        "This project implements a complete, modular, and standard-compliant POSIX shell named <b>mysh</b> in C++. "
        "The shell is architected to replicate standard Linux command-line capabilities, process control mechanisms, "
        "I/O manipulation, stream pipelining, signal handling, and interactive user convenience features such as command history and tab auto-completion. "
        "Every component has been verified with comprehensive unit and integration test suites.",
        body_style
    ))

    # Verification Matrix Table
    story.append(Paragraph("<b>Key Deliverables & Verification Matrix</b>", h2_style))
    table_data = [
        [Paragraph("Feature Component", table_header_style), Paragraph("Implementation Summary", table_header_style), Paragraph("Status", table_header_style)],
        [Paragraph("Shell Prompt", table_cell_bold), Paragraph("Displays <username@hostname:cwd>$ with ~ home directory substitution", table_cell_style), Paragraph("Verified", table_cell_bold)],
        [Paragraph("Command Parsing", table_cell_bold), Paragraph("Semicolons (;) splitting with quote ('...', \"...\") & escape (\\) support", table_cell_style), Paragraph("Verified", table_cell_bold)],
        [Paragraph("Built-in: cd", table_cell_bold), Paragraph("Supports cd, cd .., cd ~, cd -, and arbitrary paths with OLDPWD", table_cell_style), Paragraph("Verified", table_cell_bold)],
        [Paragraph("Built-in: pwd & echo", table_cell_bold), Paragraph("Current directory printing and space-trimmed text rendering", table_cell_style), Paragraph("Verified", table_cell_bold)],
        [Paragraph("Built-in: ls (Myls)", table_cell_bold), Paragraph("Full custom ls with -a, -l, -la, permissions, symlinks, block counts", table_cell_style), Paragraph("Verified", table_cell_bold)],
        [Paragraph("Built-in: pinfo", table_cell_bold), Paragraph("Extracts state, virtual memory, and exe link from Linux /proc filesystem", table_cell_style), Paragraph("Verified", table_cell_bold)],
        [Paragraph("Built-in: search", table_cell_bold), Paragraph("Recursive DFS traversal returning True/False for target files/folders", table_cell_style), Paragraph("Verified", table_cell_bold)],
        [Paragraph("Built-in: history", table_cell_bold), Paragraph("Maintains last 20 commands, disk persistence in mysh_history.txt", table_cell_style), Paragraph("Verified", table_cell_bold)],
        [Paragraph("Process Mgmt (&)", table_cell_bold), Paragraph("Background execution, setpgid() detachment, async status reaping", table_cell_style), Paragraph("Verified", table_cell_bold)],
        [Paragraph("I/O Redirection", table_cell_bold), Paragraph("Supports < (input), > (overwrite), >> (append) with dup2()", table_cell_style), Paragraph("Verified", table_cell_bold)],
        [Paragraph("Pipelines (|)", table_cell_bold), Paragraph("Arbitrary N-stage pipelines chained via pipe() with stage redirection", table_cell_style), Paragraph("Verified", table_cell_bold)],
        [Paragraph("Signal Handling", table_cell_bold), Paragraph("Ctrl-C (SIGINT) & Ctrl-Z (SIGTSTP) affect only foreground; Ctrl-D exits", table_cell_style), Paragraph("Verified", table_cell_bold)],
        [Paragraph("Tab Completion", table_cell_bold), Paragraph("GNU Readline autocomplete for commands and filesystem arguments", table_cell_style), Paragraph("Verified", table_cell_bold)],
    ]
    t_matrix = Table(table_data, colWidths=[110, 314, 80])
    t_matrix.setStyle(TableStyle([
        ('BACKGROUND', (0, 0), (-1, 0), primary_color),
        ('ALIGN', (0, 0), (-1, -1), 'LEFT'),
        ('VALIGN', (0, 0), (-1, -1), 'MIDDLE'),
        ('BOTTOMPADDING', (0, 0), (-1, -1), 3),
        ('TOPPADDING', (0, 0), (-1, -1), 3),
        ('GRID', (0, 0), (-1, -1), 0.5, colors.HexColor("#CBD5E1")),
        ('ROWBACKGROUNDS', (0, 1), (-1, -1), [colors.white, colors.HexColor("#F8FAFC")]),
    ]))
    story.append(t_matrix)
    story.append(Spacer(1, 10))

    # Section 2: Architecture & File Breakdown
    story.append(Paragraph("2. System Architecture & Modular File Breakdown", h1_style))
    story.append(Paragraph(
        "The project follows a clean, decoupled design pattern where each major responsibility is encapsulated into its own translation unit (.cpp file) linked via the global header <code>fun.h</code>.",
        body_style
    ))

    files_info = [
        ("main.cpp", "Shell entry point. Manages the REPL loop, prompt rendering, signal handler installation, and asynchronous background process reaping before prompt display."),
        ("fun.h", "Central header declaring shared data structures (BgProcess), extern global state (fg_pid, SHELL_HOME, PREV_DIR, bg_processes), and all function signatures."),
        ("parser.cpp", "Contains string manipulation routines: handleSpace() for whitespace trimming, splitCommands() for semicolon tokenization respecting quotes, and tokenizeArgs()."),
        ("builtin.cpp", "Dispatches built-in commands (is_builtin(), execute_builtin()). Implements cd (handling OLDPWD, tilde, and relative paths), pwd, echo, and exit."),
        ("ls.cpp", "Implements Myls() mimicking GNU ls. Formats file permissions (rwxr-xr-x), hardlinks, owner/group lookup (getpwuid/getgrgid), file sizes, modification timestamps, and symlink targets."),
        ("fg_bg.cpp", "Handles external program execution (executeExternal()) and background job management (CheckFgBg()). Manages process groups with setpgid() and reaps dead children."),
        ("pinfo.cpp", "Parses /proc/<pid>/stat and /proc/<pid>/exe to extract PID, execution status (R, S, Z with '+' for foreground), virtual memory consumption, and binary path."),
        ("search.cpp", "Recursive filesystem search starting from current working directory. Traverses subdirectories via opendir/readdir and returns True/False."),
        ("io_redirection.cpp", "Detects redirection symbols (<, >, >>) and applies dup2() for file descriptors. Executes commands in child process ensuring parent shell isolation."),
        ("pipeline.cpp", "Orchestrates multi-stage pipelines (RunPipeline()). Creates pipe file descriptors, chains child stdout to next child stdin, closes descriptors, and waits for completion."),
        ("autocomplete.cpp", "Configures GNU Readline hooks (rl_attempted_completion_function). Suggests commands and executables on first token, and filesystem paths on arguments."),
        ("history.cpp", "Implements command history persistence (LoadHistory(), SaveHistory(), AddHistory(), ShowHistory()). Caches up to 20 commands in mysh_history.txt without duplicates."),
        ("Makefile", "Build automation file with strict flags (-Wall -Wextra -std=c++17 -lreadline) ensuring clean, reproducible builds."),
        ("test_shell.sh", "Automated bash test script covering all 10 feature areas with automated assertions.")
    ]

    t_files_data = [[Paragraph("File / Module", table_header_style), Paragraph("Core Responsibility & Architecture Role", table_header_style)]]
    for fname, fdesc in files_info:
        t_files_data.append([Paragraph(fname, table_cell_bold), Paragraph(fdesc, table_cell_style)])
    
    t_files = Table(t_files_data, colWidths=[110, 394])
    t_files.setStyle(TableStyle([
        ('BACKGROUND', (0, 0), (-1, 0), colors.HexColor("#1E293B")),
        ('ALIGN', (0, 0), (-1, -1), 'LEFT'),
        ('VALIGN', (0, 0), (-1, -1), 'TOP'),
        ('BOTTOMPADDING', (0, 0), (-1, -1), 2.5),
        ('TOPPADDING', (0, 0), (-1, -1), 2.5),
        ('GRID', (0, 0), (-1, -1), 0.5, colors.HexColor("#CBD5E1")),
        ('ROWBACKGROUNDS', (0, 1), (-1, -1), [colors.white, colors.HexColor("#F8FAFC")]),
    ]))
    story.append(t_files)
    story.append(Spacer(1, 10))

    # Section 3: Deep Dive
    story.append(Paragraph("3. Technical Deep Dive: Mechanics & Inner Workings", h1_style))
    story.append(Paragraph("3.1 REPL Loop & Dynamic Prompt Rendering", h2_style))
    story.append(Paragraph(
        "• <b>Initialization:</b> The shell begins by obtaining its startup directory via <code>getcwd()</code> and assigning it to <code>SHELL_HOME</code>. Signal handlers for <code>SIGINT</code> and <code>SIGTSTP</code> are configured, GNU Readline autocomplete is hooked, and past history is loaded from disk.<br/>"
        "• <b>Prompt Generation:</b> The prompt is dynamically computed on each iteration as <code>&lt;username@hostname:cwd&gt;$ </code>. The username is looked up via <code>getpwuid(getuid())</code>, hostname via <code>gethostname()</code>, and the current path is formatted using <code>formatPathWithTilde()</code> where paths starting with <code>SHELL_HOME</code> are replaced with <code>~</code>.<br/>"
        "• <b>Asynchronous Reaping:</b> Before displaying the prompt, <code>checkBackgroundProcesses()</code> calls <code>waitpid(-1, &status, WNOHANG | WUNTRACED)</code> to reap any terminated background jobs and output their termination status.",
        body_style
    ))

    story.append(Paragraph("3.2 Parsing, Quoting Rules, and Semicolons", h2_style))
    story.append(Paragraph(
        "• <b>Semicolon Splitting:</b> Input lines containing multiple commands separated by <code>;</code> are split by <code>splitCommands()</code>. The parser tracks quote states (<code>in_single_quote</code> and <code>in_double_quote</code>) as well as escape backslashes (<code>\\</code>), ensuring that semicolons inside quotes (e.g. <code>echo \"hello; world\"</code>) are not treated as separators.<br/>"
        "• <b>Argument Tokenization:</b> <code>tokenizeArgs()</code> processes each command into an array of string tokens while stripping quote delimiters and preserving embedded spaces.",
        body_style
    ))

    story.append(Paragraph("3.3 Built-in Commands Implementation", h2_style))
    story.append(Paragraph(
        "• <b>cd:</b> Modifies current working directory using <code>chdir()</code>. Supports <code>cd</code> or <code>cd ~</code> (jumping to <code>SHELL_HOME</code>), <code>cd ..</code> (parent), <code>cd .</code> (current), and <code>cd -</code> (jumping to <code>PREV_DIR</code> and echoing the resolved directory).<br/>"
        "• <b>pwd:</b> Fetches and prints the exact current directory via <code>getcwd()</code>.<br/>"
        "• <b>echo:</b> Prints all space-delimited argument tokens to standard output.<br/>"
        "• <b>ls (Myls):</b> Iterates over directories using <code>opendir()</code> / <code>readdir()</code>. When <code>-l</code> is provided, it invokes <code>lstat()</code> to extract <code>st_mode</code> (formatted into 10-character permissions e.g. <code>-rw-r--r--</code>), hardlink count, owner (via <code>getpwuid</code>), group (via <code>getgrgid</code>), byte size, formatted timestamp, and symlink targets (via <code>readlink()</code>). It calculates 1K filesystem block totals by summing <code>st_blocks / 2</code>.<br/>"
        "• <b>pinfo:</b> Reads <code>/proc/&lt;pid&gt;/stat</code> to retrieve process state (e.g. R, S, Z, T) and Virtual Memory (<code>vsize</code>). Appends <code>+</code> if the process is in the foreground process group (<code>pgrp == tpgid</code>). It resolves the executable path via <code>readlink(\"/proc/&lt;pid&gt;/exe\")</code>.<br/>"
        "• <b>search:</b> Recursively traverses subdirectories using depth-first search. Returns <code>True</code> if the target file/directory exists or <code>False</code> otherwise.<br/>"
        "• <b>history:</b> Maintains a deque of the last 20 unique commands in memory and synchronizes with <code>mysh_history.txt</code> on disk.",
        body_style
    ))

    story.append(Paragraph("3.4 Foreground and Background Execution Engine", h2_style))
    story.append(Paragraph(
        "• <b>Forking:</b> External commands are executed by creating a child process via <code>fork()</code>.<br/>"
        "• <b>Background Process Group Isolation:</b> When a command ends with <code>&</code>, the child process detaches from the terminal's process group by calling <code>setpgid(0, 0)</code>. This prevents background processes from receiving terminal interrupt signals (like Ctrl-C). The parent stores the PID in <code>bg_processes</code> and displays <code>[job_id] PID</code>.<br/>"
        "• <b>Foreground Process Tracking:</b> For foreground processes, the parent assigns <code>fg_pid = pid</code> and blocks using <code>waitpid(pid, &status, WUNTRACED)</code>. If the child is suspended via Ctrl-Z, it is moved to the background list.",
        body_style
    ))

    story.append(Paragraph("3.5 I/O Redirection & Multi-Stage Pipelines", h2_style))
    story.append(Paragraph(
        "• <b>I/O Redirection:</b> Handled by <code>apply_redirection()</code>. Opens input files with <code>O_RDONLY</code> and replaces stdin via <code>dup2(fd_in, STDIN_FILENO)</code>. Output files are opened with <code>O_WRONLY | O_CREAT | O_TRUNC</code> (for <code>&gt;</code>) or <code>O_APPEND</code> (for <code>&gt;&gt;</code>) and redirected via <code>dup2(fd_out, STDOUT_FILENO)</code>.<br/>"
        "• <b>Pipeline Orchestration:</b> Multi-stage pipelines (<code>cmd1 | cmd2 | ... | cmdN</code>) create $N-1$ pipes via <code>pipe()</code>. For child $i$, <code>dup2()</code> attaches stdin to the read end of pipe $i-1$ (if $i &gt; 0$) and stdout to the write end of pipe $i$ (if $i &lt; N-1$). All unused pipe file descriptors are closed across all children and the parent process to prevent deadlock and ensure proper EOF propagation.<br/>"
        "• <b>Combined Features:</b> Pipelines fully support integrated I/O redirection on individual stages (e.g. <code>cat &lt; in.txt | grep text &gt; out.txt</code>).",
        body_style
    ))

    story.append(Paragraph("3.6 Signal Handling & GNU Readline Integration", h2_style))
    story.append(Paragraph(
        "• <b>SIGINT (Ctrl-C):</b> If a foreground process is running (<code>fg_pid &gt; 0</code>), <code>kill(fg_pid, SIGINT)</code> forwards the interrupt to terminate the child without affecting the shell. If idle at the prompt, Readline moves to a fresh new line.<br/>"
        "• <b>SIGTSTP (Ctrl-Z):</b> Forwards <code>SIGTSTP</code> to <code>fg_pid</code>, suspending the child process and pushing it into the background job table.<br/>"
        "• <b>EOF (Ctrl-D):</b> Detected when <code>readline()</code> returns <code>nullptr</code>. The shell gracefully saves history and terminates.<br/>"
        "• <b>Tab Completion:</b> Hooks <code>rl_attempted_completion_function</code>. Suggests built-in commands and local directory binaries on the first token, and filesystem paths for arguments.",
        body_style
    ))

    # Section 4: System Calls Matrix
    story.append(Spacer(1, 6))
    story.append(Paragraph("4. POSIX System Calls Reference Table", h1_style))
    syscalls_data = [
        [Paragraph("System Call", table_header_style), Paragraph("Category", table_header_style), Paragraph("Specific Purpose in mysh", table_header_style)],
        [Paragraph("fork()", table_cell_bold), Paragraph("Process", table_cell_style), Paragraph("Spawns child processes for external commands, redirection, and pipeline stages.", table_cell_style)],
        [Paragraph("execvp()", table_cell_bold), Paragraph("Process", table_cell_style), Paragraph("Replaces child process image with the specified executable binary.", table_cell_style)],
        [Paragraph("waitpid()", table_cell_bold), Paragraph("Process", table_cell_style), Paragraph("Waits for foreground processes or checks background status non-blockingly (WNOHANG).", table_cell_style)],
        [Paragraph("setpgid()", table_cell_bold), Paragraph("Process", table_cell_style), Paragraph("Sets process group ID to isolate background processes from terminal signals.", table_cell_style)],
        [Paragraph("pipe()", table_cell_bold), Paragraph("IPC", table_cell_style), Paragraph("Creates unidirectional data channels connecting pipeline stages.", table_cell_style)],
        [Paragraph("dup2()", table_cell_bold), Paragraph("I/O", table_cell_style), Paragraph("Duplicates file descriptors to redirect stdin/stdout for pipes and files.", table_cell_style)],
        [Paragraph("open() / close()", table_cell_bold), Paragraph("I/O", table_cell_style), Paragraph("Opens file handles for redirection with O_RDONLY, O_WRONLY, O_CREAT, O_APPEND, O_TRUNC.", table_cell_style)],
        [Paragraph("chdir() / getcwd()", table_cell_bold), Paragraph("Filesystem", table_cell_style), Paragraph("Navigates directories in cd and resolves current working directory in pwd and prompt.", table_cell_style)],
        [Paragraph("opendir() / readdir()", table_cell_bold), Paragraph("Filesystem", table_cell_style), Paragraph("Iterates through directory entries in custom ls and recursive search.", table_cell_style)],
        [Paragraph("lstat() / stat()", table_cell_bold), Paragraph("Filesystem", table_cell_style), Paragraph("Queries file metadata (permissions, sizes, timestamps, blocks, file types).", table_cell_style)],
        [Paragraph("readlink()", table_cell_bold), Paragraph("Filesystem", table_cell_style), Paragraph("Reads target of symbolic links in ls -l and resolves /proc/<pid>/exe in pinfo.", table_cell_style)],
        [Paragraph("sigaction() / kill()", table_cell_bold), Paragraph("Signals", table_cell_style), Paragraph("Installs handlers for SIGINT, SIGTSTP and forwards signals to foreground process.", table_cell_style)],
    ]
    t_syscalls = Table(syscalls_data, colWidths=[100, 75, 329])
    t_syscalls.setStyle(TableStyle([
        ('BACKGROUND', (0, 0), (-1, 0), primary_color),
        ('ALIGN', (0, 0), (-1, -1), 'LEFT'),
        ('VALIGN', (0, 0), (-1, -1), 'TOP'),
        ('BOTTOMPADDING', (0, 0), (-1, -1), 2.5),
        ('TOPPADDING', (0, 0), (-1, -1), 2.5),
        ('GRID', (0, 0), (-1, -1), 0.5, colors.HexColor("#CBD5E1")),
        ('ROWBACKGROUNDS', (0, 1), (-1, -1), [colors.white, colors.HexColor("#F8FAFC")]),
    ]))
    story.append(t_syscalls)
    story.append(Spacer(1, 10))

    # Section 5: Automated Testing
    story.append(Paragraph("5. Automated Test Suite (test_shell.sh)", h1_style))
    story.append(Paragraph(
        "The project includes a robust automated validation script (<code>test_shell.sh</code>) verifying all functional requirements:",
        body_style
    ))
    test_cases = [
        ("Test 1: Basic Built-ins", "Validates pwd, echo, cd .., cd -, and clean exit."),
        ("Test 2: Semicolon Sequences", "Executes multiple commands chained via ; sequentially."),
        ("Test 3: I/O Redirection", "Tests file output creation (>), input reading (<), and appending (>>)."),
        ("Test 4: Multi-stage Pipelines", "Validates multi-process chaining: ls -l | grep -E '\\.cpp|\\.h' | wc -l."),
        ("Test 5: Pipelines with Redirection", "Tests integrated input/output redirection within pipeline stages."),
        ("Test 6: Search Command", "Validates recursive file search returning True for existing files and False otherwise."),
        ("Test 7: Process Info (pinfo)", "Tests /proc inspection for PID, state (R+), memory, and executable path."),
        ("Test 8: Custom ls Listing", "Tests ls -a, ls -l, and combined ls -la with full permissions & details."),
        ("Test 9: Background Processes", "Spawns background sleep 1 &, executes foreground task, and verifies async reaping."),
        ("Test 10: Command History", "Verifies history retrieval and disk persistence with history 5.")
    ]
    for t_name, t_desc in test_cases:
        story.append(Paragraph(f"• <b>{t_name}:</b> {t_desc}", bullet_style))

    # Section 6: Compilation & Run
    story.append(Spacer(1, 6))
    story.append(Paragraph("6. Compilation and Execution Guide", h1_style))
    story.append(Paragraph(
        "<b>Compilation:</b><br/>"
        "<code>$ make clean && make</code><br/><br/>"
        "<b>Running the Shell:</b><br/>"
        "<code>$ ./mysh</code><br/><br/>"
        "<b>Running Automated Test Suite:</b><br/>"
        "<code>$ chmod +x test_shell.sh && ./test_shell.sh</code>",
        body_style
    ))

    # Build PDF
    doc.build(story, canvasmaker=NumberedCanvas)
    print(f"PDF generated successfully at {filename}")

if __name__ == "__main__":
    current_dir = os.path.dirname(os.path.abspath(__file__))
    pdf_path = os.path.join(current_dir, "POSIX_Shell_Documentation.pdf")
    create_pdf(pdf_path)
