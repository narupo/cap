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
    kit_t *kit;
    int last_exit_code;
    char line_buf[LINE_BUFFER_SIZE];
};

/*************
* prototypes *
*************/

int 
shcmd_exec_command(shcmd_t *self, int argc, char **argv);

/************
* functions *
************/

/**
 * Show usage of command
 *
 * @param[in] self pointer to shcmd_t
 */
static void
shcmd_show_usage(shcmd_t *self) {
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
}

/**
 * Parse options
 *
 * @param[in] self pointer to shcmd_t 
 *
 * @return success to true
 * @return failed to false
 */
static bool
shcmd_parse_opts(shcmd_t *self) {
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
shcmd_del(shcmd_t *self) {
    if (!self) {
        return;
    }

    // DO NOT DELETE config and argv
    PadCmdline_Del(self->cmdline);
    kit_del(self->kit);
    free(self);
}

shcmd_t *
shcmd_new(CapConfig *config, int argc, char **argv) {
    shcmd_t *self = PadMem_ECalloc(1, sizeof(*self));

    self->config = config;
    self->argc = argc;
    self->argv = argv;
    self->cmdline = PadCmdline_New();
    self->kit = kit_new(config);

    if (!shcmd_parse_opts(self)) {
        shcmd_del(self);
        return NULL;
    }

    return self;
}

char *
shcmd_create_prompt(shcmd_t *self, char *dst, int32_t dstsz) {
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
shcmd_input(shcmd_t *self) {
    char prompt[FILE_NPATH];
    shcmd_create_prompt(self, prompt, sizeof prompt);

    PadTerm_CFPrintf(stderr, TERM_CYAN, TERM_DEFAULT, TERM_BRIGHT, "(cap) ");
    PadTerm_CFPrintf(stderr, TERM_GREEN, TERM_DEFAULT, TERM_BRIGHT, "%s", prompt);
    PadTerm_CFPrintf(stderr, TERM_BLUE, TERM_DEFAULT, TERM_BRIGHT, "$ ");
    fflush(stderr);

    self->line_buf[0] = '\0';
    if (file_getline(self->line_buf, sizeof self->line_buf, stdin) == EOF) {
        return 1;
    }

    return 0;
}

int
shcmd_exec_alias(shcmd_t *self, bool *found, int argc, char **argv) {
    *found = false;
    CapAliasMgr *almgr = CapAliasMgr_New(self->config);

    // find alias value by name
    // find first from local scope
    // not found to find from global scope
    const char *cmdname = argv[0];
    char alias_val[1024];
    if (almgr_find_alias_value(almgr, alias_val, sizeof alias_val, cmdname, CAP_SCOPE_LOCAL) == NULL) {
        CapAliasMgr_Clear_error(almgr);
        if (almgr_find_alias_value(almgr, alias_val, sizeof alias_val, cmdname, CAP_SCOPE_GLOBAL) == NULL) {
            *found = false;
            return 1;
        }
    }
    almgr_del(almgr);
    *found = true;

    // create cap's command line with alias value
    string_t *cmdline = PadStr_New();

    PadStr_App(cmdline, alias_val);
    PadStr_App(cmdline, " ");
    for (int i = 1; i < argc; ++i) {
        PadStr_App(cmdline, "\"");
        PadStr_App(cmdline, argv[i]);
        PadStr_App(cmdline, "\"");
        PadStr_App(cmdline, " ");
    }
    str_popb(cmdline);

    // convert command to application's arguments
    cl_t *cl = cl_new();
    cl_parse_str(cl, PadStr_Getc(cmdline));
    PadStr_Del(cmdline);

    int re_argc = cl_len(cl);
    char **re_argv = cl_escdel(cl);

    shcmd_exec_command(self, re_argc, re_argv);

    freeargv(re_argc, re_argv);
    return 0;
}

static int
execute_all(shcmd_t *self, int argc, char *argv[]) {
    const char *cmdname = argv[0];
    bool found = false;
    int result;

    result = shcmd_exec_alias(self, &found, argc, argv);
    if (found) {
        return result;
    }
    
    result = execute_snippet(self->config, &found, argc, argv, cmdname);
    if (found) {
        return result;
    }
    
    result = execute_program(self->config, &found, argc, argv, cmdname);
    if (found) {
        return result;
    }

    PadCStrAry *args = pushf_argv(argc, argv, "run");
    int run_argc = PadCStrAry_Len(args);
    char **run_argv = cstrarr_escdel(args);
    return execute_run(self->config, run_argc, run_argv);
}

int 
shcmd_exec_command(shcmd_t *self, int argc, char **argv) {
    int result = 0;

#define routine(cmd) { \
        cmd##_t *cmd = cmd##_new(self->config, argc, argv); \
        if (!cmd) { \
            return 0; \
        } \
        result = cmd##_run(cmd); \
        cmd##_del(cmd); \
    } \

    const char *cmdname = argv[0];

    if (PadCStr_Eq(cmdname, "exit")) {
        return 1;
    } else if (PadCStr_Eq(cmdname, "clear")) {
        clear_screen();
    } else if (argc >= 2 && PadCStr_Eq(cmdname, "echo") && PadCStr_Eq(argv[1], "$?")) {
        printf("%d\n", self->last_exit_code);
    } else if (PadCStr_Eq(cmdname, "home")) {
        routine(homecmd);
        config_init(self->config);
    } else if (PadCStr_Eq(cmdname, "cd")) {
        routine(cdcmd);
        config_init(self->config);
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
        rmcmd_t *cmd = rmcmd_new(self->config, argc, argv);
        result = rmcmd_run(cmd);
        switch (rmcmd_errno(cmd)) {
        case RMCMD_ERR_NOERR: break;
        default: PadErr_Err(rmcmd_what(cmd)); break;
        }
        rmcmd_del(cmd);
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
shcmd_update(shcmd_t *self) {
    if (strstr(self->line_buf, "{@")) {
        kit_clear_context_buffer(self->kit);
        if (!kit_compile_from_string(self->kit, self->line_buf)) {
            kit_trace_error(self->kit, stderr);
            return 1;
        }
        const char *result = kit_getc_stdout_buf(self->kit);
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

    int argc = cl_len(obj->cl);
    char **argv = cl_get_argv(obj->cl);

    return shcmd_exec_command(self, argc, argv);
}

int
shcmd_run(shcmd_t *self) {
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
