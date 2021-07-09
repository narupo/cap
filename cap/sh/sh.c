#include <sh/sh.h>

enum {
    LINE_BUFFER_SIZE = 1024 * 10,
};

/**
 * Structure of options
 */
struct Opts {
    bool is_help;
};

/**
 * Structure of command
 */
struct sh {
    CapConfig *config;
    int argc;
    int optind;
    char **argv;
    struct Opts opts;
    PadCmdline *cmdline;
    PadKit *kit;
    int last_exit_code;
    char line_buf[LINE_BUFFER_SIZE];
};

/*************
* prototypes *
*************/

static int 
exec_cmd(CapShCmd *self, int argc, char **argv);

/************
* functions *
************/

/**
 * Show usage of command
 *
 * @param[in] self pointer to CapShCmd
 */
static void
usage(CapShCmd *self) {
    fflush(stdout);
    fflush(stderr);
    fprintf(stderr, "Usage:\n"
        "\n"
        "    cap sh [options]...\n"
        "\n"
        "The options are:\n"
        "\n"
        "    -h, --help    Show usage\n"
        "\n"
    );
    fflush(stderr);
    exit(0);
}

/**
 * Parse options
 *
 * @param[in] self pointer to CapShCmd 
 *
 * @return success to true
 * @return failed to false
 */
static bool
parse_opts(CapShCmd *self) {
    // parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {"fname", required_argument, 0, 'f'},
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
        case 'f': printf("%s\n", optarg); break;
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
CapShCmd_Del(CapShCmd *self) {
    if (!self) {
        return;
    }

    // DO NOT DELETE config and argv
    PadCmdline_Del(self->cmdline);
    PadKit_Del(self->kit);
    Pad_SafeFree(self);
}

CapShCmd *
CapShCmd_New(CapConfig *config, int argc, char **argv) {
    CapShCmd *self = PadMem_Calloc(1, sizeof(*self));
    if (self == NULL) {
        goto error;
    }

    self->config = config;
    self->argc = argc;
    self->argv = argv;
    self->cmdline = PadCmdline_New();
    if (self->cmdline == NULL) {
        goto error;
    }

    self->kit = PadKit_New(config);
    if (self->kit == NULL) {
        goto error;
    }

    if (!parse_opts(self)) {
        goto error;
    }

    return self;
error:
    CapShCmd_Del(self);
    return NULL;
}

static char *
create_prompt(CapShCmd *self, char *dst, int32_t dstsz) {
    const char *home = self->config->home_path;
    const char *cd = self->config->cd_path;
    const char *b = cd;

    for (const char *a = home; *a && *b; ++a, ++b) {
    }

    char *dend = dst + dstsz - 1;
    char *dp = dst;

    for (; *b && dp < dend; ++b, ++dp) {
#ifdef CAP__WINDOWS
        *dp = *b == '\\' ? '/' : *b;
#else
        *dp = *b;
#endif
    }

    *dp = '\0';

    return dst;
}

int
shcmd_input(CapShCmd *self) {
    char prompt[PAD_FILE__NPATH];
    create_prompt(self, prompt, sizeof prompt);

    PadTerm_CFPrintf(stderr, PAD_TERM__CYAN, TERM_DEFAULT, PAD_TERM__BRIGHT, "(cap) ");
    PadTerm_CFPrintf(stderr, PAD_TERM__GREEN, TERM_DEFAULT, PAD_TERM__BRIGHT, "%s", prompt);
    PadTerm_CFPrintf(stderr, TERM_BLUE, TERM_DEFAULT, PAD_TERM__BRIGHT, "$ ");
    fflush(stderr);

    self->line_buf[0] = '\0';
    if (PadFile_GetLine(self->line_buf, sizeof self->line_buf, stdin) == EOF) {
        return 1;
    }

    return 0;
}

static int
exec_alias(CapShCmd *self, bool *found, int argc, char **argv) {
    *found = false;
    CapAliasMgr *almgr = CapAliasMgr_New(self->config);

    // find alias value by name
    // find first from local scope
    // not found to find from global scope
    const char *cmdname = argv[0];
    char alias_val[1024];
    if (CapAliasMgr_FindAliasValue(almgr, alias_val, sizeof alias_val, cmdname, CAP_SCOPE__LOCAL) == NULL) {
        CapAliasMgr_ClearError(almgr);
        if (CapAliasMgr_FindAliasValue(almgr, alias_val, sizeof alias_val, cmdname, CAP_SCOPE__GLOBAL) == NULL) {
            *found = false;
            return 1;
        }
    }
    CapAliasMgr_Del(almgr);
    *found = true;

    // create cap's command line with alias value
    PadStr *cmdline = PadStr_New();

    PadStr_App(cmdline, alias_val);
    PadStr_App(cmdline, " ");
    for (int i = 1; i < argc; ++i) {
        PadStr_App(cmdline, "\"");
        PadStr_App(cmdline, argv[i]);
        PadStr_App(cmdline, "\"");
        PadStr_App(cmdline, " ");
    }
    PadStr_PopBack(cmdline);

    // convert command to application's arguments
    PadCL *cl = PadCL_New();
    PadCL_ParseStr(cl, PadStr_Getc(cmdline));
    PadStr_Del(cmdline);

    int re_argc = PadCL_Len(cl);
    char **re_argv = PadCL_EscDel(cl);

    exec_cmd(self, re_argc, re_argv);

    Pad_FreeArgv(re_argc, re_argv);
    return 0;
}

static int
execute_all(CapShCmd *self, int argc, char *argv[]) {
    const char *cmdname = argv[0];
    bool found = false;
    int result;

    result = exec_alias(self, &found, argc, argv);
    if (found) {
        return result;
    }
    
    result = Cap_ExecSnippet(self->config, &found, argc, argv, cmdname);
    if (found) {
        return result;
    }
    
    result = Cap_ExecProg(self->config, &found, argc, argv, cmdname);
    if (found) {
        return result;
    }

    PadCStrAry *args = Pad_PushFrontArgv(argc, argv, "run");
    int run_argc = PadCStrAry_Len(args);
    char **run_argv = PadCStrAry_EscDel(args);
    return Cap_ExecRun(self->config, run_argc, run_argv);
}

int 
exec_cmd(CapShCmd *self, int argc, char **argv) {
    int result = 0;

#define routine(cmd) { \
        Cap##cmd *cmd = Cap##cmd##_New(self->config, argc, argv); \
        if (!cmd) { \
            return 0; \
        } \
        result = Cap##cmd##_Run(cmd); \
        Cap##cmd##_Del(cmd); \
    } \

    const char *cmdname = argv[0];

    if (PadCStr_Eq(cmdname, "exit")) {
        return 1;
    } else if (PadCStr_Eq(cmdname, "clear")) {
        Pad_ClearScreen();
    } else if (argc >= 2 && PadCStr_Eq(cmdname, "echo") && PadCStr_Eq(argv[1], "$?")) {
        printf("%d\n", self->last_exit_code);
    } else if (PadCStr_Eq(cmdname, "home")) {
        routine(homecmd);
        CapConfig_Init(self->config);
    } else if (PadCStr_Eq(cmdname, "cd")) {
        routine(cdcmd);
        CapConfig_Init(self->config);
    } else if (PadCStr_Eq(cmdname, "pwd")) {
        routine(pwdcmd);
    } else if (PadCStr_Eq(cmdname, "ls")) {
        routine(lscmd);
    } else if (PadCStr_Eq(cmdname, "cat")) {
        routine(catcmd);
    } else if (PadCStr_Eq(cmdname, "run")) {
        routine(runcmd);
    } else if (PadCStr_Eq(cmdname, "exec")) {
        routine(execcmd);
    } else if (PadCStr_Eq(cmdname, "alias")) {
        routine(alcmd);
    } else if (PadCStr_Eq(cmdname, "edit")) {
        routine(editcmd);
    } else if (PadCStr_Eq(cmdname, "editor")) {
        routine(editorcmd);
    } else if (PadCStr_Eq(cmdname, "mkdir")) {
        routine(mkdircmd);
    } else if (PadCStr_Eq(cmdname, "rm")) {
        CapRmCmd *cmd = CapRmCmd_New(self->config, argc, argv);
        result = CapRmCmd_Run(cmd);
        switch (CapRmCmd_Errno(cmd)) {
        case CAP_RMCMD_ERR__NOERR: break;
        default: PadErr_Err(CapRmCmd_What(cmd)); break;
        }
        CapRmCmd_Del(cmd);
    } else if (PadCStr_Eq(cmdname, "mv")) {
        routine(mvcmd);
    } else if (PadCStr_Eq(cmdname, "cp")) {
        routine(cpcmd);
    } else if (PadCStr_Eq(cmdname, "snippet")) {
        routine(snptcmd);
    } else if (PadCStr_Eq(cmdname, "link")) {
        routine(linkcmd);
    } else if (PadCStr_Eq(cmdname, "make")) {
        routine(makecmd);
    } else if (PadCStr_Eq(cmdname, "cook")) {
        routine(cookcmd);
    } else if (PadCStr_Eq(cmdname, "find")) {
        routine(findcmd);
    } else {
        result = execute_all(self, argc, argv);
    }

    self->last_exit_code = result;
    return 0;
}

int
shcmd_update(CapShCmd *self) {
    if (strstr(self->line_buf, "{@")) {
        PadKit_ClearCtxBuf(self->kit);
        if (!PadKit_CompileFromStr(self->kit, self->line_buf)) {
            PadKitrace_error(self->kit, stderr);
            return 1;
        }
        const char *result = PadKit_GetcStdoutBuf(self->kit);
        printf("%s", result);
        fflush(stdout);
        return 0;
    }

    if (!PadCmdline_Parse(self->cmdline, self->line_buf)) {
        PadErr_Err("failed to parse command line");
        return 1;
    }

    if (PadCmdline_Len(self->cmdline) == 0) {
        return 0;
    }

    const cmdline_object_t *obj = PadCmdline_Getc(self->cmdline, 0);
    if (obj->type != CMDLINE_OBJECT_TYPE_CMD) {
        return 0;
    }

    int argc = PadCL_Len(obj->cl);
    char **argv = PadCL_GetArgv(obj->cl);

    return exec_cmd(self, argc, argv);
}

int
CapShCmd_Run(CapShCmd *self) {
    for (;;) {
        if (shcmd_input(self) != 0) {
            break;
        }

        if (shcmd_update(self) != 0) {
            break;
        }
    }

    return 0;
}
