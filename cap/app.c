/**
 * Cap
 *
 * license: MIT
 *  author: narupo
 *   since: 2016
 */
#include <app.h>

/**
 * numbers
 */
enum {
    MAX_RECURSION_LIMIT = 8,
    NOT_FOUND = -1,
};

/**
 * program option
 */
struct Opts {
    bool is_help;
    bool is_version;
};

/**
 * application structure
 */
typedef struct {
    int argc;
    char **argv;
    int cmd_argc;
    char **cmd_argv;
    CapConfig *config;
    struct Opts opts;
    PadErrStack *errstack;
} app_t;

static int
app_run(app_t *self, int argc, char *argv[]);

/**
 * parse options
 *
 * @param[in] self
 * @return success to true
 * @return failed to false
 */
static bool
app_parse_opts(app_t *self) {
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'V'},
        {0},
    };

    // init status
    self->opts = (struct Opts){0};
    optind = 0;
    opterr = 0;

    // parse options
    for (;;) {
        int optsindex;
        int cur = getopt_long(self->argc, self->argv, "hV", longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 'h': self->opts.is_help = true; break;
        case 'V': self->opts.is_version = true; break;
        case '?':
        default:
            Pad_PushErr("invalid option");
            return false; break;
        }
    }

    if (self->argc < optind) {
        return false;
    }

    return true;
}

/**
 * destruct module
 *
 * @param[in] self
 */
static void
app_del(app_t *self) {
    if (self) {
        PadConfig_del(self->config);
        Pad_FreeArgv(self->argc, self->argv);
        Pad_FreeArgv(self->cmd_argc, self->cmd_argv);
        PadErrStack_Del(self->errstack);
        free(self);
    }
}

/**
 * deploy Cap's environment at user's file system
 *
 * @param[in] self
 * @return success to true
 * @return failed to false
 */
static bool
app_deploy_env(const app_t *self) {
    char userhome[FILE_NPATH];
    if (!PadFile_GetUserHome(userhome, sizeof userhome)) {
        Pad_PushErr("failed to get user's home directory. what is your file system?");
        return false;
    }

    // make application directory
    char appdir[FILE_NPATH];
    if (!PadFile_SolveFmt(appdir, sizeof appdir, "%s/.cap", userhome)) {
        Pad_PushErr("faield to create application directory path");
        return false;
    }

    if (!PadFile_IsExists(appdir)) {
        if (PadFile_MkdirQ(appdir) != 0) {
            Pad_PushErr("failed to make application directory");
            return false;
        }
    }

    // make variable directory
    char vardir[FILE_NPATH];
    if (!PadFile_SolveFmt(vardir, sizeof vardir, "%s/var", appdir)) {
        Pad_PushErr("failed to create path of variable");
        return false;
    }

    if (!PadFile_IsExists(vardir)) {
        if (PadFile_MkdirQ(vardir) != 0) {
            Pad_PushErr("failed to make variable directory");
            return false;
        }
    }

    // make variable files
    char path[FILE_NPATH];
    if (!PadFile_SolveFmt(path, sizeof path, "%s/cd", vardir)) {
        Pad_PushErr("failed to create path of cd variable");
        return false;
    }
    if (!PadFile_IsExists(path)) {
        if (!PadFile_WriteLine(userhome, path)) {
            Pad_PushErr("failed to write line to cd of variable");
            return false;
        }
    }

    if (!PadFile_SolveFmt(path, sizeof path, "%s/home", vardir)) {
        Pad_PushErr("failed to create path of home variable");
        return false;
    }
    if (!PadFile_IsExists(path)) {
        if (!PadFile_WriteLine(userhome, path)) {
            Pad_PushErr("failed to write line to home of variable");
            return false;
        }
    }

    if (!PadFile_SolveFmt(path, sizeof path, "%s/editor", vardir)) {
        Pad_PushErr("failed to create path of home variable");
        return false;
    }
    if (!PadFile_IsExists(path)) {
        if (!PadFile_WriteLine("", path)) {
            Pad_PushErr("failed to write line to home of variable");
            return false;
        }
    }

    // make snippets directory
    if (!PadFile_SolveFmt(path, sizeof path, "%s/codes", appdir)) {
        Pad_PushErr("failed to solve path for snippet codes directory");
        return false;
    }
    if (!PadFile_IsExists(path)) {
        if (PadFile_MkdirQ(path) != 0) {
            Pad_PushErr("failed to make directory for snippet codes directory");
            return false;
        }
    }

    // standard library directory
    if (!PadFile_IsExists(self->config->std_lib_dir_path)) {
        if (PadFile_MkdirQ(self->config->std_lib_dir_path) != 0) {
            Pad_PushErr("failed to make directory for standard libraries directory");
            return false;
        }
    }

    return true;
}

/**
 * parse arguments
 *
 * @param[in]  self
 * @param[in]  argc
 * @param[in]  argv
 *
 * @return success to true
 * @return failed to false
 */
static bool
app_parse_args(app_t *self, int argc, char *argv[]) {
    PadDistriArgs dargs = {0};
    if (!PadDistriArgs_Distribute(&dargs, argc, argv)) {
        Pad_PushErr("failed to distribute arguments");
        return false;
    }

    self->argc = dargs.argc;
    self->argv = dargs.argv;
    self->cmd_argc = dargs.cmd_argc;
    self->cmd_argv = dargs.cmd_argv;
    return true;
}

/**
 * construct module
 *
 * @param[in] argc
 * @param[in] argv
 *
 * @return success to pointer to dynamic allocate memory to app_t
 * @return failed to NULL
 */
static app_t *
app_new(void) {
    app_t *self = PadMem_Calloc(1, sizeof(*self));
    if (self == NULL) {
        return NULL;
    }

    self->errstack = PadErrStack_New();
    self->config = PadConfig_New();

    return self;
}

/**
 * show usage of module
 *
 * @param[in] app
 */
static void
app_usage(app_t *app) {
    static const char usage[] =
        "Cap is shell for snippet codes.\n"
        "\n"
        "Usage:\n"
        "\n"
        "    cap [options] [command] [arguments]\n"
        "\n"
        "The options are:\n"
        "\n"
        "    -h, --help       show usage\n"
        "    -V, --version    show version\n"
        "\n"
        "The commands are:\n"
        "\n"
        "    home       change home directory, or show\n"
        "    cd         change current directory by relative of home\n"
        "    pwd        show current directory\n"
        "    ls         show list in current directory\n"
        "    cat        concatenate files and show\n"
        "    make       compile template language\n"
        "    cook       compile template language without solve the path\n"
        "    run        run script\n"
        "    exec       execute command line\n"
        "    alias      run alias command\n"
        "    edit       run editor with file name\n"
        "    editor     set editor or show\n"
        "    mkdir      make directory at environment\n"
        "    rm         remove file or directory from environment\n"
        "    mv         rename file on environment\n"
        "    cp         copy file\n"
        "    touch      create empty file\n"
        "    snippet    save or show snippet codes\n"
        "    link       create symbolic link\n"
        "    sh         run shell\n"
        "    find       find snippet\n"
        "    bake       bake file (make and replace file content)\n"
        "    insert     insert code at label\n"
        "    clone      clone git repository\n"
        "    replace    replace text of file\n"
    ;
    static const char *examples[] = {
        "    $ cap home\n"
        "    $ cap pwd\n"
        "    $ cap ls\n"
        ,
        "    $ cap editor /usr/bin/vim\n"
        "    $ cap edit my.txt\n"
        ,
        "    $ cap cat path/to/code/file.c\n"
        "    $ cap ls path/to/code\n"
        ,
        "    $ cap mkdir path/to/dir\n"
        "    $ cap edit path/to/dir/file.txt\n"
        ,
    };
    const int exmlen = sizeof(examples)/sizeof(*examples);
    const char *example = NULL;

    srand(time(NULL));
    example = examples[Pad_RandRange(0, exmlen-1)];

    fprintf(stderr,
        "%s\n"
        "Examples:\n\n"
        "%s\n"
    , usage, example);
}

/**
 * show version of module
 *
 * @param[in] self
 */
static void
app_version(app_t *self) {
    fflush(stdout);
    fflush(stderr);
    printf("%s\n", _CAP_VERSION);
    fflush(stdout);
}

/**
 * check if the argument is the command name of Cap
 *
 * @param[in] self
 * @param[in] cmdname command name
 *
 * @return If the cmdname is the command name of Cap to true
 * @return If the cmdname not is the command name of Cap to false
 */
static bool
app_is_cap_cmdname(const app_t *self, const char *cmdname) {
    static const char *capcmdnames[] = {
        "home",
        "cd",
        "pwd",
        "ls",
        "cat",
        "run",
        "exec",
        "alias",
        "edit",
        "editor",
        "mkdir",
        "rm",
        "mv",
        "cp",
        "touch",
        "snippet",
        "link",
        "hub",
        "make",
        "cook",
        "sh",
        "find",
        "bake",
        "insert",
        "clone",
        "replace",
        NULL,
    };

    for (const char **p = capcmdnames; *p; ++p) {
        if (Pad_CStrEq(cmdname, *p)) {
            return true;
        }
    }

    return false;
}

/**
 * execute command by command name
 *
 * @param[in] self
 * @param[in] name command name
 *
 * @return success to 0
 * @return failed to not 0
 */
static int
app_execute_command_by_name(app_t *self, const char *name) {
#define routine(cmd) { \
        cmd##_t *cmd = cmd##_new(self->config, self->cmd_argc, self->cmd_argv); \
        if (!cmd) { \
            return 1; \
        } \
        result = cmd##_run(cmd); \
        cmd##_del(cmd); \
    } \

    int result = 0;

    if (PadCStr_Eq(name, "home")) {
        routine(homecmd);
    } else if (PadCStr_Eq(name, "cd")) {
        routine(cdcmd);
    } else if (PadCStr_Eq(name, "pwd")) {
        routine(pwdcmd);
    } else if (PadCStr_Eq(name, "ls")) {
        routine(lscmd);
    } else if (PadCStr_Eq(name, "cat")) {
        routine(catcmd);
    } else if (PadCStr_Eq(name, "run")) {
        routine(runcmd);
    } else if (PadCStr_Eq(name, "exec")) {
        routine(execcmd);
    } else if (PadCStr_Eq(name, "alias")) {
        routine(alcmd);
    } else if (PadCStr_Eq(name, "edit")) {
        routine(editcmd);
    } else if (PadCStr_Eq(name, "editor")) {
        routine(editorcmd);
    } else if (PadCStr_Eq(name, "mkdir")) {
        routine(mkdircmd);
    } else if (PadCStr_Eq(name, "rm")) {
        rmcmd_t *cmd = rmcmd_new(self->config, self->cmd_argc, self->cmd_argv);
        result = rmcmd_run(cmd);
        switch (rmcmd_errno(cmd)) {
        case RMCMD_ERR_NOERR: break;
        default: Pad_PushErr(rmcmd_what(cmd)); break;
        }
        rmcmd_del(cmd);
    } else if (PadCStr_Eq(name, "mv")) {
        routine(mvcmd);
    } else if (PadCStr_Eq(name, "cp")) {
        routine(cpcmd);
    } else if (PadCStr_Eq(name, "touch")) {
        routine(touchcmd);
    } else if (PadCStr_Eq(name, "snippet")) {
        routine(snptcmd);
    } else if (PadCStr_Eq(name, "link")) {
        routine(linkcmd);
    } else if (PadCStr_Eq(name, "make")) {
        routine(makecmd);
    } else if (PadCStr_Eq(name, "cook")) {
        routine(cookcmd);
    } else if (PadCStr_Eq(name, "sh")) {
        routine(shcmd);
    } else if (PadCStr_Eq(name, "find")) {
        routine(findcmd);
    } else if (PadCStr_Eq(name, "bake")) {
        routine(bakecmd);
    } else if (PadCStr_Eq(name, "insert")) {
        routine(insertcmd);
    } else if (PadCStr_Eq(name, "clone")) {
        routine(clonecmd);
    } else if (PadCStr_Eq(name, "replace")) {
        routine(replacecmd);
    } else {
        Pad_PushErr("invalid command name \"%s\"", name);
        result = 1;
    }

    return result;
}

/**
 * execute alias by alias name
 *
 * @param[in]  self
 * @param[in]  name alias name
 *
 * @return success to 0
 * @return failed to not 0
 */
static int
app_execute_alias_by_name(app_t *self, bool *found, const char *name) {
    CapAliasMgr *almgr = CapAliasMgr_New(self->config);

    // find alias value by name
    // find first from local scope
    // not found to find from global scope
    char val[1024];
    if (almgr_find_alias_value(almgr, val, sizeof val, name, CAP_SCOPE_LOCAL) == NULL) {
        CapAliasMgr_Clear_error(almgr);
        if (almgr_find_alias_value(almgr, val, sizeof val, name, CAP_SCOPE_GLOBAL) == NULL) {
            *found = false;
            return 1;
        }
    }
    almgr_del(almgr);
    *found = true;

    // create cap's command line with alias value
    PadStr *cmdline = PadStr_New();

    PadStr_App(cmdline, "cap ");
    PadStr_App(cmdline, val);
    PadStr_App(cmdline, " ");
    char escarg[1024];
    for (int i = 1; i < self->cmd_argc; ++i) {
        const char * arg = self->cmd_argv[i];
        PadStr_App(cmdline, "\"");
        Pad_Escape(escarg, sizeof escarg, arg, "\"");
        PadStr_App(cmdline, escarg);
        PadStr_App(cmdline, "\"");
        PadStr_App(cmdline, " ");
    }
    PadStr_PopBack(cmdline);

    // convert command to application's arguments
    PadCL *cl = PadCL_New();
    PadCL_ParseStrOpts(cl, PadStr_Getc(cmdline), 0);
    PadStr_Del(cmdline);

    int argc = PadCL_Len(cl);
    char **argv = PadCL_EscDel(cl);

    // re-parse application's arguments
    Pad_FreeArgv(self->argc, self->argv);
    Pad_FreeArgv(self->cmd_argc, self->cmd_argv);

    // increment recursion count for safety
    self->config->recursion_count++;

    // run application
    if (self->config->recursion_count >= MAX_RECURSION_LIMIT) {
        Pad_PushErr("reached recursion limit");
        return 1;
    }

    int result = app_run(self, argc, argv);
    Pad_FreeArgv(argc, argv);

    return result;
}

/**
 * run module by cmdname of first argument of command arguments
 * 
 * @param[in] *self 
 * 
 * @return 
 */
static int
app_run_cmdname(app_t *self) {
    const char *cmdname = self->cmd_argv[0];
    if (!cmdname) {
        Pad_PushErr("command name is null");
        return 1; // impossible
    }

    if (app_is_cap_cmdname(self, cmdname)) {
        return app_execute_command_by_name(self, cmdname);
    }

    bool found = false;
    int result = app_execute_alias_by_name(self, &found, cmdname);
    if (found) {
        return result;
    }

    found = false;
    result = execute_snippet(self->config, &found, self->cmd_argc, self->cmd_argv, cmdname);
    if (found) {
        return result;
    }

    found = false;
    result = execute_program(self->config, &found, self->cmd_argc, self->cmd_argv, cmdname);
    if (found) {
        return result;
    }

    PadCStrAry *new_argv = Pad_PushFrontArgv(self->cmd_argc, self->cmd_argv, "run");
    int argc = PadCStrAry_Len(new_argv);
    char **argv = PadCStrAry_EscDel(new_argv);
    return execute_run(self->config, argc, argv);
}

static bool
app_init(app_t *self, int argc, char *argv[]) {
    if (!config_init(self->config)) {
        PadErrStack *es = PadConfig_GetErrStack(self->config);
        errstack_extendb_other(self->errstack, es);
        Pad_PushErr("failed to configuration");
        return false;
    }

    if (!app_parse_args(self, argc, argv)) {
        Pad_PushErr("failed to parse arguments");
        return false;
    }

    if (!app_parse_opts(self)) {
        Pad_PushErr("failed to parse options");
        return false;
    }

    if (!app_deploy_env(self)) {
        Pad_PushErr("failed to deploy environment at file system");
        return false;
    }

    return true;
}

/**
 * run module
 *
 * @param[in] self
 *
 * @return success to 0
 * @return failed to not 0
 */
static int
app_run(app_t *self, int argc, char *argv[]) {
    if (!app_init(self, argc, argv)) {
        return 1;
    }

    if (self->opts.is_help) {
        app_usage(self);
        return 0;
    }

    if (self->opts.is_version) {
        app_version(self);
        return 0;
    }

    if (self->cmd_argc == 0) {
        app_usage(self);
        return 0;
    }

    return app_run_cmdname(self);
}

/**
 * stack trace
 * 
 * @param[in] *self
 */
static void
app_trace(const app_t *self) {
    if (PadErrStack_Len(self->errstack)) {
        fflush(stdout);
        PadErrStack_TraceSimple(self->errstack, stderr);
        fflush(stderr);        
    }
}

/**
 * main routine
 *
 * @param[in] argc
 * @param[in] argv
 *
 * @return success to 0
 * @return failed to not 0
 */
int
main(int argc, char *argv[]) {
    // set locale for unicode object (char32_t, char16_t)
    setlocale(LC_CTYPE, "");

    app_t *app = app_new();
    if (!app) {
        PadErr_Die("failed to start application");
    }

    int result = app_run(app, argc, argv);
    if (result != 0) {
        app_trace(app);
    }

    app_del(app);

    return result;
}
