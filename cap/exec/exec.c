#include <cap/exec/exec.h>

static const int32_t READ = 0;
static const int32_t WRITE = 1;

/**
 * Structure of options
 */
struct Opts {
    bool is_help;
};

/**
 * Structure of command
 */
struct CapExecCmd {
    const CapConfig *config;
    int argc;
    int optind;
    char **argv;
    struct Opts opts;
    PadCmdline *cmdline;
    int32_t cmdline_index;
    char what[1024];
#ifdef CAP__WINDOWS
    PadStr *read_buffer;
#endif
};

/**
 * Show usage of command
 *
 * @param[in] self pointer to CapExecCmd
 */
static int
usage(CapExecCmd *self) {
    fflush(stdout);
    fflush(stderr);
    fprintf(stderr, "Usage:\n"
        "\n"
        "    cap exec [options]... [command-line]\n"
        "\n"
        "The options are:\n"
        "\n"
        "    -h, --help    Show usage\n"
        "\n"
    );
    fflush(stderr);
    return 0;
}

/**
 * Parse options
 *
 * @param[in] self pointer to CapExecCmd
 *
 * @return success to true
 * @return failed to false
 */
static bool
parse_opts(CapExecCmd *self) {
    // parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {0},
    };

    self->opts = (struct Opts){0};

    extern int opterr;
    extern int optind;
    opterr = 0; // ignore error messages
    optind = 0; // init index of parse

    for (;;) {
        int optsindex;
        int cur = getopt_long(self->argc, self->argv, "hf:", longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 0: /* long option only */ break;
        case 'h': self->opts.is_help = true; break;
        case '?':
        default:
            PadErr_Die("unknown option");
            return false;
            break;
        }
    }

    if (self->argc < optind) {
        PadErr_Die("failed to parse option");
        return false;
    }

    self->optind = optind;
    return true;
}

void
CapExecCmd_Del(CapExecCmd *self) {
    if (!self) {
        return;
    }

    PadCmdline_Del(self->cmdline);
#if CAP__WINDOWS
    PadStr_Del(self->read_buffer);
#endif
    Pad_SafeFree(self);
}

CapExecCmd *
CapExecCmd_New(const CapConfig *config, int argc, char **argv) {
    CapExecCmd *self = PadMem_Calloc(1, sizeof(*self));
    if (self == NULL) {
        return NULL;
    }

    self->config = config;
    self->argc = argc;
    self->argv = argv;
    self->cmdline = PadCmdline_New();
#if CAP__WINDOWS
    self->read_buffer = PadStr_New();
#endif

    if (!parse_opts(self)) {
        CapExecCmd_Del(self);
        return NULL;
    }

    return self;
}

static void
set_err(CapExecCmd *self, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(self->what, sizeof self->what, fmt, ap);
    va_end(ap);
}

static bool
has_err(const CapExecCmd *self) {
    return self->what[0] != '\0';
}

static CapExecCmd *
exec_first(CapExecCmd *self) {
    const PadCmdlineObj *first = PadCmdline_Getc(self->cmdline, 0);
    const char *cmd = PadStr_Getc(first->command);
    Pad_SafeSystem(cmd, PAD_SAFESYSTEM__UNSAFE_UNIX_ONLY);
    return self;
}

#ifdef CAP__WINDOWS
static CapExecCmd *
create_read_pipe(CapExecCmd *self, HANDLE process, HANDLE *read_handle, HANDLE *write_handle) {
    HANDLE tmp_read_handle = NULL;

    if (!CreatePipe(&tmp_read_handle, write_handle, NULL, 0)) {
        set_err(self, "failed to create pipe");
        return NULL;
    }

    if (!DuplicateHandle(
        process,
        tmp_read_handle,
        process,
        read_handle,
        0,
        TRUE,
        DUPLICATE_SAME_ACCESS)) {
        set_err(self, "failed to duplicate handle");
        CloseHandle(tmp_read_handle);
        CloseHandle(*write_handle);
        return NULL;
    }

    if (!CloseHandle(tmp_read_handle)) {
        set_err(self, "failed to close handle");
        CloseHandle(*read_handle);
        CloseHandle(*write_handle);
        return NULL;
    }

    return self;
}

static CapExecCmd *
create_write_pipe(CapExecCmd *self, HANDLE process, HANDLE *read_handle, HANDLE *write_handle) {
    HANDLE tmp_write_handle = NULL;

    if (!CreatePipe(read_handle, &tmp_write_handle, NULL, 0)) {
        set_err(self, "failed to create pipe");
        return NULL;
    }

    if (!DuplicateHandle(
        process,
        tmp_write_handle,
        process,
        write_handle,
        0,
        TRUE,
        DUPLICATE_SAME_ACCESS)) {
        set_err(self, "failed to duplicate handle");
        CloseHandle(tmp_write_handle);
        CloseHandle(*read_handle);
        return NULL;
    }

    if (!CloseHandle(tmp_write_handle)) {
        set_err(self, "failed to close handle");
        CloseHandle(*read_handle);
        CloseHandle(*write_handle);
        return NULL;
    }

    return self;
}

static CapExecCmd *
execcmd_pipe(CapExecCmd *self, const PadCmdlineObj *obj, const PadCmdlineObj *ope) {
    HANDLE hs1[2] = {0};
    HANDLE hs2[2] = {0};
    HANDLE process = GetCurrentProcess();

#define close_hs1() { \
        CloseHandle(hs1[READ]); \
        CloseHandle(hs1[WRITE]); \
    }
#define close_hs2() { \
        CloseHandle(hs2[READ]); \
        CloseHandle(hs2[WRITE]); \
    }
#define close_hs() { \
        close_hs1(); \
        close_hs2(); \
    }

    if (!create_read_pipe(self, process, &hs1[READ], &hs1[WRITE])) {
        return NULL;
    }

    if (!create_write_pipe(self, process, &hs2[READ], &hs2[WRITE])) {
        close_hs1();
        return NULL;
    }

    // create process
    STARTUPINFO si = {0};
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = hs1[READ];
    si.hStdOutput = hs2[WRITE];
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    if (si.hStdOutput == INVALID_HANDLE_VALUE) {
        set_err(self, "failed to get stdout handle");
        close_hs();
        return NULL;
    }
    if (si.hStdError == INVALID_HANDLE_VALUE) {
        set_err(self, "failed to get stdout handle");
        close_hs();
        return NULL;
    }

    LPTSTR cmdline = (LPTSTR) PadStr_Getc(obj->command);
    PROCESS_INFORMATION pi = {0};

    if (!CreateProcess(
        NULL,
        cmdline,
        NULL,
        NULL,
        TRUE,
        0,
        NULL,
        NULL,
        &si,
        &pi)) {
        set_err(self, "failed to create process");
        return NULL;
    }

    HANDLE child_process = pi.hProcess;
    if (!CloseHandle(pi.hThread)) {
        set_err(self, "failed to close thread handle");
        close_hs();
        return NULL;
    }

    CloseHandle(hs1[READ]);
    hs1[READ] = NULL;

    CloseHandle(hs2[WRITE]);
    hs2[WRITE] = NULL;

    // write to pipe (parent write to child process)
    DWORD nwrite = 0;
    WriteFile(hs1[WRITE], PadStr_Getc(self->read_buffer), PadStr_Len(self->read_buffer), &nwrite, NULL);
    CloseHandle(hs1[WRITE]);

    // read from pipe (parent read from child process)
    char buf[512];
    DWORD nread = 0;

    PadStr_Clear(self->read_buffer);
    for (;;) {
        memset(buf, 0, sizeof buf);
        ReadFile(hs2[READ], buf, sizeof(buf)-1, &nread, NULL);
        if (nread == 0) {
            break;
        }
        buf[nread] = '\0';

        char *text = PadFile_ConvLineEnc(self->config->line_encoding, buf);
        if (!text) {
            set_err(self, "failed to convert line encoding");
            close_hs();
            return NULL;
        }

        PadStr_App(self->read_buffer, text);
        Pad_SafeFree(text);
    }
    CloseHandle(hs2[READ]);

    // wait for child process
    DWORD result = WaitForSingleObject(child_process, INFINITE);
    switch (result) {
    default:
        set_err(self, "failed to wait");
        close_hs();
        CloseHandle(child_process);
        return NULL;
        break;
    case WAIT_OBJECT_0: // success
        close_hs();
        CloseHandle(child_process);
        if (!ope) {
            printf("%s", PadStr_Getc(self->read_buffer));
            fflush(stdout);
        }
        return self;
        break;
    }

    return NULL; // impossible
}

static CapExecCmd *
execcmd_redirect(CapExecCmd *self, const PadCmdlineObj *obj, const PadCmdlineObj *fileobj) {
    HANDLE hs1[2] = {0};
    HANDLE hs2[2] = {0};
    HANDLE process = GetCurrentProcess();

#define close_hs1() { \
        CloseHandle(hs1[READ]); \
        CloseHandle(hs1[WRITE]); \
    }
#define close_hs2() { \
        CloseHandle(hs2[READ]); \
        CloseHandle(hs2[WRITE]); \
    }
#define close_hs() { \
        close_hs1(); \
        close_hs2(); \
    }

    if (!create_read_pipe(self, process, &hs1[READ], &hs1[WRITE])) {
        return NULL;
    }

    if (!create_write_pipe(self, process, &hs2[READ], &hs2[WRITE])) {
        close_hs1();
        return NULL;
    }

    // create process
    STARTUPINFO si = {0};
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = hs1[READ];
    si.hStdOutput = hs2[WRITE];
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    if (si.hStdOutput == INVALID_HANDLE_VALUE) {
        set_err(self, "failed to get stdout handle");
        close_hs();
        return NULL;
    }
    if (si.hStdError == INVALID_HANDLE_VALUE) {
        set_err(self, "failed to get stdout handle");
        close_hs();
        return NULL;
    }

    LPTSTR cmdline = (LPTSTR) PadStr_Getc(obj->command);
    PROCESS_INFORMATION pi = {0};

    if (!CreateProcess(
        NULL,
        cmdline,
        NULL,
        NULL,
        TRUE,
        0,
        NULL,
        NULL,
        &si,
        &pi)) {
        set_err(self, "failed to create process");
        return NULL;
    }

    HANDLE child_process = pi.hProcess;
    if (!CloseHandle(pi.hThread)) {
        set_err(self, "failed to close thread handle");
        close_hs();
        return NULL;
    }

    CloseHandle(hs1[READ]);
    hs1[READ] = NULL;

    CloseHandle(hs2[WRITE]);
    hs2[WRITE] = NULL;

    // write to pipe (parent write to child process)
    DWORD nwrite = 0;
    WriteFile(hs1[WRITE], PadStr_Getc(self->read_buffer), PadStr_Len(self->read_buffer), &nwrite, NULL);
    CloseHandle(hs1[WRITE]);

    // read from pipe (parent read from child process)
    char buf[512];
    DWORD nread = 0;

    PadStr_Clear(self->read_buffer);
    for (;;) {
        memset(buf, 0, sizeof buf);
        ReadFile(hs2[READ], buf, sizeof(buf)-1, &nread, NULL);
        if (nread == 0) {
            break;
        }
        buf[nread] = '\0';

        char *text = PadFile_ConvLineEnc(self->config->line_encoding, buf);
        if (!text) {
            set_err(self, "failed to convert line encoding");
            close_hs();
            return NULL;
        }

        PadStr_App(self->read_buffer, text);
        Pad_SafeFree(text);
    }
    CloseHandle(hs2[READ]);

    // wait for child process
    DWORD result = WaitForSingleObject(child_process, INFINITE);
    switch (result) {
    default:
        set_err(self, "failed to wait");
        close_hs();
        CloseHandle(child_process);
        return NULL;
        break;
    case WAIT_OBJECT_0: // success
        close_hs();
        CloseHandle(child_process);

        char fname[PAD_FILE__NPATH];
        if (!Cap_SolveCmdlineArgPath(self->config, fname, sizeof fname, PadStr_Getc(fileobj->command))) {
            set_err(self, "failed to solve path of command line argument");
            return NULL;
        }

        FILE *fout = PadFile_Open(fname, "wb");
        if (!fout) {
            set_err(self, "failed to open \"%s\"", fname);
            return NULL;
        }

        fwrite(PadStr_Getc(self->read_buffer), PadStr_Len(self->read_buffer), 1, fout);

        fclose(fout);

        return self;
        break;
    }

    return NULL; // impossible
}

static CapExecCmd *
exec_all_win(CapExecCmd *self) {
    for (int32_t i = 0; i < PadCmdline_Len(self->cmdline); i += 2) {
        const PadCmdlineObj *obj = PadCmdline_Getc(self->cmdline, i);
        const PadCmdlineObj *ope = PadCmdline_Getc(self->cmdline, i+1);

        if (ope && ope->type == PAD_CMDLINE_OBJ_TYPE__AND) {
            int exit_code = Pad_SafeSystem(PadStr_Getc(obj->command), PAD_SAFESYSTEM__UNSAFE_UNIX_ONLY);
            if (exit_code != 0) {
                break;
            }
        } else if (ope && ope->type == PAD_CMDLINE_OBJ_TYPE__REDIRECT) {
            const PadCmdlineObj *fileobj = PadCmdline_Getc(self->cmdline, i+2);
            if (!execcmd_redirect(self, obj, fileobj)) {
                return NULL;
            }
            break; // done
        } else {
            if (!execcmd_pipe(self, obj, ope)) {
                return NULL;
            }
        }
    }

    return self;
}
#else

static CapExecCmd *
exec_all_unix(CapExecCmd *self) {
    int stdinno = dup(STDIN_FILENO);
    int stdoutno = dup(STDOUT_FILENO);
    int exit_code = 0;

    for (int32_t i = 0; i < PadCmdline_Len(self->cmdline); i += 2) {
        const PadCmdlineObj *obj = PadCmdline_Getc(self->cmdline, i);
        const PadCmdlineObj *ope = PadCmdline_Getc(self->cmdline, i+1);

        if (ope && ope->type == PAD_CMDLINE_OBJ_TYPE__AND) {
            // AND
            const char *cmd = PadStr_Getc(obj->command);
            int status = Pad_SafeSystem(cmd, PAD_SAFESYSTEM__UNSAFE_UNIX_ONLY);
            exit_code = WEXITSTATUS(status);
            if (exit_code != 0) {
                break;
            }
        } else if (ope && ope->type == PAD_CMDLINE_OBJ_TYPE__REDIRECT) {
            // REDIRECT
            const PadCmdlineObj *fileobj = PadCmdline_Getc(self->cmdline, i+2);
            if (!fileobj) {
                set_err(self, "not found file object in redirect");
                break;
            }

            int fd[2] = {0};
            if (pipe(fd) != 0) {
                set_err(self, "failed to create pipe");
                return NULL;
            }

            pid_t pid = fork();
            switch (pid) {
            default: { // parent
                // STDOUT_FILENOに出力する
                close(fd[READ]);
                dup2(fd[WRITE], STDOUT_FILENO);
                close(fd[WRITE]);

                const char *cmd = PadStr_Getc(obj->command);
                int status = Pad_SafeSystem(cmd, PAD_SAFESYSTEM__UNSAFE_UNIX_ONLY);
                exit_code = WEXITSTATUS(status);
                (void) &exit_code;

                dup2(stdoutno, STDOUT_FILENO);
                dup2(stdinno, STDIN_FILENO);

                wait(NULL);
                goto done;
            } break;
            case 0: { // child
                // fd[READ]から入力を読み込み、fnameのファイルに出力する
                close(fd[WRITE]);

                char fname[PAD_FILE__NPATH];
                if (!Cap_SolveCmdlineArgPath(self->config, fname, sizeof fname, PadStr_Getc(fileobj->command))) {
                    set_err(self, "failed to solve path of command line argument");
                    return NULL;
                }

                int filefd = open(fname, O_CREAT | O_WRONLY | O_TRUNC);
                if (filefd == -1) {
                    set_err(self, "failed to open \"%s\"", fname);
                    return NULL;
                }

                char buf[1024+1];
                for (;;) {
                    ssize_t nread = read(fd[READ], buf, sizeof(buf)-1);
                    if (nread == -1) {
                        set_err(self, "failed to read from read descriptor");
                        return NULL;
                    } else if (nread == 0) {
                        break;
                    }
                    buf[nread] = '\0';

                    if (write(filefd, buf, nread) == -1) {
                        set_err(self, "failed to write to file descriptor");
                        break;
                    }
                }

                close(filefd);
                close(fd[READ]);

                // execコマンドはテンプレート言語の組み込み関数（functions.c@builtin_exec）からも呼ばれる
                // ここでexit()しないと言語のパースを継続してしまい、プロセスが作られるごとに
                // 言語がパースされることになる。子プロセスでは言語のパースを継続しない（親ではする）
                // そのため、パースを継続しないようにここでexit()する
                exit(0);
            } break;
            case -1: { // error
                set_err(self, "failed to fork (2)");
                close(fd[READ]);
                close(fd[WRITE]);
                return NULL;
            } break;
            }
        } else {
            // PIPE
            int fd[2] = {0};
            if (pipe(fd) != 0) {
                set_err(self, "failed to create pipe");
                return NULL;
            }

            pid_t pid = fork();
            switch (pid) {
            default: { // parent
                close(fd[READ]);
                if (ope) {
                    dup2(fd[WRITE], STDOUT_FILENO);
                    close(fd[WRITE]);
                } else {
                    close(fd[WRITE]);
                }

                const char *cmd = PadStr_Getc(obj->command);
                int status = Pad_SafeSystem(cmd, PAD_SAFESYSTEM__UNSAFE_UNIX_ONLY);
                exit_code = WEXITSTATUS(status);
                (void) &exit_code;

                dup2(stdoutno, STDOUT_FILENO);
                dup2(stdinno, STDIN_FILENO);

                wait(NULL);

                goto done;
            } break;
            case 0: { // child
                close(fd[WRITE]);
                dup2(fd[READ], STDIN_FILENO);
                close(fd[READ]);
            } break;
            case -1: { // error
                set_err(self, "failed to fork");
                close(fd[READ]);
                close(fd[WRITE]);
                return NULL;
            } break;
            }
        }
    }

done:
    close(stdinno);
    close(stdoutno);
    return self;
}
#endif

static CapExecCmd *
exec_all(CapExecCmd *self) {
    if (PadCmdline_Len(self->cmdline) < 3) {
        set_err(self, "too few command line objects");
        return NULL;
    }

#ifdef CAP__WINDOWS
    return exec_all_win(self);
#else
    return exec_all_unix(self);
#endif
}

static CapExecCmd *
cmd_exec(CapExecCmd *self, const char *cltxt) {
    if (!PadCmdline_Parse(self->cmdline, cltxt)) {
        set_err(self, "failed to parse command line");
        return NULL;
    }

    if (PadCmdline_Len(self->cmdline) == 1) {
        return exec_first(self);
    } else {
        return exec_all(self);
    }

    return self;
}

static char *
Pad_Unescape_cl(const char *escaped) {
    PadStr *s = PadStr_New();

    for (const char *p = escaped; *p; p += 1) {
        if (*p == '\\') {
            Pad_Unescape(s, &p, "\"'");
        } else {
            PadStr_PushBack(s, *p);
        }
    }

    return PadStr_EscDel(s);
}

int
CapExecCmd_Run(CapExecCmd *self) {
    if (self->argc - self->optind == 0 ||
        self->opts.is_help) {
        return usage(self);
    }

    for (int32_t i = self->optind; i < self->argc; ++i) {
        const char *escaped_cltxt = self->argv[i];
        char *cltxt = Pad_Unescape_cl(escaped_cltxt);
        // printf("escaped_cltxt[%s]\n", escaped_cltxt);
        // printf("cltxt[%s]\n", cltxt);

        if (!cmd_exec(self, cltxt)) {
            Pad_SafeFree(cltxt);
            PadErr_Err(self->what);
            return 1;
        }
        Pad_SafeFree(cltxt);
    }

    return 0;
}
