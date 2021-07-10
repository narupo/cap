/**
 * Cap
 *
 * License: MIT
 *  Author: narupo
 *   Since: 2016
 */
#include <cap/tests.h>

/********
* utils *
********/

/**
 * Show error message and exit from process.
 *
 * @param string fmt message format.
 * @param ...    ... format arguments.
 */
static void
die(const char *fmt, ...) {
    if (!fmt) {
        return;
    }

    size_t fmtlen = strlen(fmt);
    va_list args;
    va_start(args, fmt);

    fflush(stdout);
    fprintf(stderr, "die: ");

    if (isalpha(fmt[0])) {
        fprintf(stderr, "%c", toupper(fmt[0]));
        vfprintf(stderr, fmt+1, args);
    } else {
        vfprintf(stderr, fmt, args);
    }

    if (fmtlen && fmt[fmtlen-1] != '.') {
        fprintf(stderr, ". ");
    }

    if (errno != 0) {
        fprintf(stderr, "%s.", strerror(errno));
    }

    fprintf(stderr, "\n");

    va_end(args);
    fflush(stderr);
    exit(EXIT_FAILURE);
}

/**
 * Show error message.
 *
 * @param string fmt message format.
 * @param ...    ... format arguments.
 */
static void
warn(const char *fmt, ...) {
    if (!fmt) {
        return;
    }

    size_t fmtlen = strlen(fmt);
    va_list args;
    va_start(args, fmt);

    fflush(stdout);
    fprintf(stderr, "die: ");

    if (isalpha(fmt[0])) {
        fprintf(stderr, "%c", toupper(fmt[0]));
        vfprintf(stderr, fmt+1, args);
    } else {
        vfprintf(stderr, fmt, args);
    }

    if (fmtlen && fmt[fmtlen-1] != '.') {
        fprintf(stderr, ". ");
    }

    if (errno != 0) {
        fprintf(stderr, "%s.", strerror(errno));
    }

    fprintf(stderr, "\n");

    va_end(args);
    fflush(stderr);
}

/**
 * solve path
 * fix for valgrind issue
 *
 * @param[in] *dst
 * @param[in] dstsz
 * @param[in] *path
 *
 * @return
 */
static char *
solve_path(char *dst, int32_t dstsz, const char *path) {
    char tmp[PAD_FILE__NPATH] = {0};
    assert(PadFile_Solve(tmp, sizeof tmp, path));
    snprintf(dst, dstsz, "%s", tmp);
    return dst;
}

/********
* tests *
********/

struct testcase {
    const char *name;
    void (*test)(void);
};

struct testmodule {
    const char *name;
    const struct testcase *tests;
};

/********
* array *
********/

void
_freeescarr(char **arr) {
    for (char **p = arr; *p; ++p) {
        free(*p);
    }
    free(arr);
}

int
_countescarr(char **arr) {
    int i = 0;
    for (char **p = arr; *p; ++p) {
        ++i;
    }
    return i;
}

/*******
* util *
*******/

static void
test_util_Cap_IsOutOfHome(void) {
    char userhome[PAD_FILE__NPATH];
    assert(PadFile_GetUserHome(userhome, sizeof userhome) != NULL);

    char varhome[PAD_FILE__NPATH];
    assert(PadFile_SolveFmt(varhome, sizeof varhome, "%s/.cap/var/home", userhome) != NULL);

    char caphome[PAD_FILE__NPATH];
    assert(PadFile_ReadLine(caphome, sizeof caphome, varhome) != NULL);

    assert(Cap_IsOutOfHome(caphome, "/not/found/dir"));
    assert(!Cap_IsOutOfHome(caphome, caphome));
}

static void
test_util_Cap_IsOutOfHome_2(void) {
    assert(!Cap_IsOutOfHome(NULL, NULL));
    assert(!Cap_IsOutOfHome("", NULL));

    assert(Cap_IsOutOfHome("/my/home", "/path/to/dir"));
    assert(Cap_IsOutOfHome("/my/home", "/my"));
    assert(!Cap_IsOutOfHome("/my/home", "/my/home/file"));
}

static void
test_util_Cap_SolveCmdlineArgPath(void) {
    CapConfig *config = CapConfig_New();
    config->scope = CAP_SCOPE__LOCAL;

    char fname[PAD_FILE__NPATH];

#ifdef CAP__WINDOWS
    snprintf(config->home_path, sizeof config->home_path, "C:\\path\\to\\home");
    snprintf(config->cd_path, sizeof config->cd_path, "C:\\path\\to\\cd");

    assert(Cap_SolveCmdlineArgPath(config, fname, sizeof fname, "path/to/file"));
    assert(!strcmp(fname, "C:\\path\\to\\cd\\path\\to\\file"));

    assert(Cap_SolveCmdlineArgPath(config, fname, sizeof fname, "/path/to/file"));
    assert(!strcmp(fname, "C:\\path\\to\\home\\path\\to\\file"));
#else
    snprintf(config->home_path, sizeof config->home_path, "/path/to/home");
    snprintf(config->cd_path, sizeof config->cd_path, "/path/to/cd");

    assert(Cap_SolveCmdlineArgPath(config, fname, sizeof fname, "path/to/file"));
    assert(!strcmp(fname, "/path/to/cd/path/to/file"));

    assert(Cap_SolveCmdlineArgPath(config, fname, sizeof fname, "/path/to/file"));
    assert(!strcmp(fname, "/path/to/home/path/to/file"));
#endif
    CapConfig_Del(config);
}

static void
test_util_Cap_GetOrigin(void) {
    CapConfig *config = CapConfig_New();

    strcpy(config->home_path, "/home");
    strcpy(config->cd_path, "/cd");

    assert(Cap_GetOrigin(NULL, NULL) == NULL);
    assert(Cap_GetOrigin(config, NULL) == NULL);

    const char *org = Cap_GetOrigin(config, "/file");
    assert(!strcmp(org, "/home"));

    config->scope = CAP_SCOPE__LOCAL;
    org = Cap_GetOrigin(config, "file");
    assert(!strcmp(org, "/cd"));

    config->scope = CAP_SCOPE__GLOBAL;
    org = Cap_GetOrigin(config, "file");
    assert(!strcmp(org, "/home"));

    CapConfig_Del(config);
}

/*
static void
test_util_show_snippet(void) {
    char root[PAD_FILE__NPATH];
    PadFile_SolveFmt(root, sizeof root, "tests_env/util");

    if (PadFile_IsExists(root)) {
        PadFile_MkdirQ(root);
    }

    FILE *fout = fopen("tests_env/util/file.txt", "wt");
    fputs("abc\n", fout);
    fclose(fout);

    CapConfig *config = CapConfig_New();
    strcpy(config->codes_dir_path, root);
    int argc = 0;
    char *argv[] = {
        NULL,
    };
    assert(show_snippet(NULL, NULL, argc, NULL));
    assert(show_snippet(config, NULL, argc, NULL));
    assert(show_snippet(config, "", argc, NULL));

    char buf[1024] = {0};
    setbuf(stdout, buf);
    assert(show_snippet(config, "file.txt", argc, argv));
    setbuf(stdout, NULL);
    assert(!strcmp(buf, "abc\n"));

    CapConfig_Del(config);
}
*/

static void
test_util_Cap_ExecSnippet(void) {
    char root[PAD_FILE__NPATH];
    PadFile_SolveFmt(root, sizeof root, "./tests_env/util");
    char path[PAD_FILE__NPATH];
    PadFile_SolveFmt(path, sizeof path, "./tests_env/util/snippet.txt");

    FILE *fout = fopen(path, "wt");
    fputs("abc\n", fout);
    fclose(fout);

    CapConfig *config = CapConfig_New();
    strcpy(config->codes_dir_path, root);
    bool found = false;
    int argc = 0;
    char *argv[] = {
        NULL,
    };
    assert(Cap_ExecSnippet(NULL, NULL, 0, NULL, NULL) == 1);
    assert(Cap_ExecSnippet(config, NULL, 0, NULL, NULL) == 1);
    assert(Cap_ExecSnippet(config, &found, 0, NULL, NULL) == 1);
    assert(Cap_ExecSnippet(config, &found, argc, argv, NULL) == 1);

    assert(Cap_ExecSnippet(config, &found, argc, argv, "nothing.txt") == -1);

    char buf[1024] = {0};
    setbuf(stdout, buf);
    assert(Cap_ExecSnippet(config, &found, argc, argv, "snippet.txt") == 0);
    setbuf(stdout, NULL);
    assert(!strcmp(buf, "abc"));

    strcpy(config->codes_dir_path, "nothing");
    assert(Cap_ExecSnippet(config, &found, argc, argv, "snippet.txt") == 1);

    CapConfig_Del(config);
    PadFile_Remove("./tests_env/util/snippet.txt");
}

static void
test_util_Cap_ExecRun(void) {
    // nothing todo
}

static void
test_util_Cap_ExecProg(void) {
    CapConfig *config = CapConfig_New();

    config->scope = CAP_SCOPE__LOCAL;
    strcpy(config->cd_path, "tests_env/util");
    strcpy(config->home_path, "tests_env/util");

    char rcpath[PAD_FILE__NPATH] = {0};
    PadFile_SolveFmt(rcpath, sizeof rcpath, "tests_env/util/.caprc");

    FILE *fout = fopen(rcpath, "wt");
    fputs("PATH = \"bin\"\n", fout);
    fclose(fout);

    if (!PadFile_IsExists("tests_env/util/bin")) {
        PadFile_MkdirQ("tests_env/util/bin");
    }

    bool found = false;
    int argc = 0;
    char *argv[] = {NULL};
    assert(Cap_ExecProg(config, &found, argc, argv, "nothing") == 1);

    CapConfig_Del(config);
    PadFile_Remove("tests_env/util/.caprc");
}

static const struct testcase
utiltests[] = {
    {"Cap_IsOutOfHome", test_util_Cap_IsOutOfHome},
    {"Cap_IsOutOfHome_2", test_util_Cap_IsOutOfHome_2},
    {"Cap_SolveCmdlineArgPath", test_util_Cap_SolveCmdlineArgPath},
    {"Cap_GetOrigin", test_util_Cap_GetOrigin},
    // {"show_snippet", test_util_show_snippet},
    {"Cap_ExecSnippet", test_util_Cap_ExecSnippet},
    {"Cap_ExecRun", test_util_Cap_ExecRun},
    {"Cap_ExecProg", test_util_Cap_ExecProg},
    {0},
};

/**********
* symlink *
**********/

static void
test_CapSymlink_NormPath(void) {
    CapConfig * config = CapConfig_New();

    char path[PAD_FILE__NPATH];
    assert(CapSymlink_NormPath(NULL, NULL, 0, NULL) == NULL);
    assert(CapSymlink_NormPath(config, NULL, 0, NULL) == NULL);
    assert(CapSymlink_NormPath(config, path, 0, NULL) == NULL);
    assert(CapSymlink_NormPath(config, path, sizeof path, NULL) == NULL);

#ifdef CAP__WINDOWS
    assert(CapSymlink_NormPath(config, path, sizeof path, "C:\\path\\to\\dir") == path);
    assert(strcmp(path, "C:\\path\\to\\dir") == 0);

    assert(CapSymlink_NormPath(config, path, sizeof path, "\\path\\to\\dir") == path);
    assert(strcmp(path, "\\path\\to\\dir") == 0);

    assert(CapSymlink_NormPath(config, path, sizeof path, "\\path\\..\\to\\dir") == path);
    assert(strcmp(path, "\\to\\dir") == 0);

    assert(CapSymlink_NormPath(config, path, sizeof path, "\\path\\..\\to\\..\\dir") == path);
    assert(strcmp(path, "\\dir") == 0);

    assert(CapSymlink_NormPath(config, path, sizeof path, "\\path\\to\\..\\..\\dir") == path);
    assert(strcmp(path, "\\dir") == 0);

    assert(CapSymlink_NormPath(config, path, sizeof path, "C:\\path\\to\\dir\\") == path);
    assert(strcmp(path, "C:\\path\\to\\dir") == 0);

    assert(CapSymlink_NormPath(config, path, sizeof path, "C:\\path\\..\\to\\dir") == path);
    assert(strcmp(path, "C:\\to\\dir") == 0);

    assert(CapSymlink_NormPath(config, path, sizeof path, "C:\\path\\..\\to\\..\\dir") == path);
    assert(strcmp(path, "C:\\dir") == 0);

    assert(CapSymlink_NormPath(config, path, sizeof path, "C:\\path\\to\\..\\..\\dir") == path);
    assert(strcmp(path, "C:\\dir") == 0);

#else
    assert(CapSymlink_NormPath(config, path, sizeof path, "/path/to/dir") == path);
    assert(strcmp(path, "/path/to/dir") == 0);

    assert(CapSymlink_NormPath(config, path, sizeof path, "/path/to/dir/") == path);
    assert(strcmp(path, "/path/to/dir") == 0);

    assert(CapSymlink_NormPath(config, path, sizeof path, "path/to/dir") == path);
    assert(strcmp(path, "path/to/dir") == 0);

    assert(CapSymlink_NormPath(config, path, sizeof path, "path/../to/dir") == path);
    assert(strcmp(path, "to/dir") == 0);

    assert(CapSymlink_NormPath(config, path, sizeof path, "path/../to/../dir") == path);
    assert(strcmp(path, "dir") == 0);

    assert(CapSymlink_NormPath(config, path, sizeof path, "path/to/../../dir") == path);
    assert(strcmp(path, "dir") == 0);

    assert(CapSymlink_NormPath(config, path, sizeof path, "/path/../to/dir") == path);
    assert(strcmp(path, "/to/dir") == 0);

    assert(CapSymlink_NormPath(config, path, sizeof path, "/path/../to/../dir") == path);
    assert(strcmp(path, "/dir") == 0);

    assert(CapSymlink_NormPath(config, path, sizeof path, "/path/to/../../dir") == path);
    assert(strcmp(path, "/dir") == 0);
#endif

    CapConfig_Del(config);
}

static const struct testcase
symlink_tests[] = {
    {"CapSymlink_NormPath", test_CapSymlink_NormPath},
    {0},
};

/***************
* home command *
***************/

static void
test_homecmd_default(void) {
    CapConfig *config = CapConfig_New();
    int argc = 2;
    char *argv[] = {
        "cd",
        "tests_env/home",
        NULL,
    };

    if (!PadFile_IsExists("tests_env/.cap")) {
        PadFile_MkdirQ("tests_env/.cap");
    }
    if (!PadFile_IsExists("tests_env/.cap/var")) {
        PadFile_MkdirQ("tests_env/.cap/var");
    }

    assert(solve_path(config->var_home_path, sizeof config->var_home_path, "./tests_env/.cap/var/home"));

    CapHomeCmd *homecmd = CapHomeCmd_New(config, argc, argv);
    CapHomeCmd_Run(homecmd);
    CapHomeCmd_Del(homecmd);

    char line[1024];
    assert(PadFile_ReadLine(line, sizeof line, config->var_home_path));

#ifdef CAP_TESTS__WINDOWS
    assert(strstr(line, "tests_env\\home"));
#else
    assert(strstr(line, "tests_env/home"));
#endif

    CapConfig_Del(config);
}

static const struct testcase
CapHomeCmdests[] = {
    {"default", test_homecmd_default},
    {0},
};

/*************
* cd command *
*************/

static void
test_cdcmd_default(void) {
    CapConfig *config = CapConfig_New();
    int argc = 2;
    char *argv[] = {
        "cd",
        "tests_env/path/to/dir/",
        NULL,
    };

    assert(solve_path(config->home_path, sizeof config->home_path, "."));
    assert(solve_path(config->cd_path, sizeof config->cd_path, "."));
    assert(solve_path(config->var_cd_path, sizeof config->var_cd_path, "./tests_env/.cap/var/cd"));

    CapCdCmd *cdcmd = CapCdCmd_New(config, argc, argv);
    CapCdCmd_Run(cdcmd);
    CapCdCmd_Del(cdcmd);

    char line[1024];
    assert(PadFile_ReadLine(line, sizeof line, config->var_cd_path));
    assert(!strstr(line, "tests_env/.cap/var/cd"));

    CapConfig_Del(config);
}

static const struct testcase
cdcmd_tests[] = {
    {"default", test_cdcmd_default},
    {0},
};

/**************
* pwd command *
**************/

static void
test_pwdcmd_default(void) {
    CapConfig *config = CapConfig_New();
    int argc = 1;
    char *argv[] = {
        "pwd",
        NULL,
    };

    assert(solve_path(config->cd_path, sizeof config->cd_path, "./tests_env/path/to/dir"));

    char stdout_buf[1024];
    setbuf(stdout, stdout_buf);

    CapPwdCmd *pwdcmd = CapPwdCmd_New(config, argc, argv);
    CapPwdCmd_Run(pwdcmd);
    CapPwdCmd_Del(pwdcmd);

    setbuf(stdout, NULL);

    assert(strstr(stdout_buf, "/tests_env/path/to/dir"));

    CapConfig_Del(config);
}

static void
test_pwdcmd_nomalize_opt(void) {
    CapConfig *config = CapConfig_New();
    int argc = 2;
    char *argv[] = {
        "pwd",
        "-n",
        NULL,
    };

    assert(solve_path(config->cd_path, sizeof config->cd_path, "./tests_env/path/to/dir"));

    char stdout_buf[1024];
    setbuf(stdout, stdout_buf);

    CapPwdCmd *pwdcmd = CapPwdCmd_New(config, argc, argv);
    CapPwdCmd_Run(pwdcmd);
    CapPwdCmd_Del(pwdcmd);

    setbuf(stdout, NULL);

#ifdef CAP_TESTS__WINDOWS
    assert(strstr(stdout_buf, "\\tests_env\\path\\to\\dir"));
#else
    assert(strstr(stdout_buf, "/tests_env/path/to/dir"));
#endif

    CapConfig_Del(config);
}

static const struct testcase
CapPwdCmdests[] = {
    {"default", test_pwdcmd_default},
    {"normalize", test_pwdcmd_nomalize_opt},
    {0},
};

/*************
* ls command *
*************/

static void
test_lscmd_default(void) {

    return;  // TODO

    CapConfig *config = CapConfig_New();
    int argc = 1;
    char *argv[] = {
        "ls",
        NULL,
    };

    assert(solve_path(config->cd_path, sizeof config->cd_path, "./tests_env/ls"));

    char buf[1024] = {0};
    setvbuf(stdout, buf, _IOFBF, sizeof buf);

    CapLsCmd *lscmd = CapLsCmd_New(config, argc, argv);
    CapLsCmd_Run(lscmd);
    CapLsCmd_Del(lscmd);

    fflush(stdout);
    setvbuf(stdout, NULL, _IOLBF, BUFSIZ);

    // printf("stdout[%s]\n", buf);
    assert(!strcmp(buf, "a\nb\nc"));

    CapConfig_Del(config);
}

static const struct testcase
CapLsCmdests[] = {
    {"default", test_lscmd_default},
    {0},
};

/**************
* cat command *
**************/

static void
test_catcmd_default(void) {
    CapConfig *config = CapConfig_New();
    int argc = 2;
    char *argv[] = {
        "cat",
        "/tests_env/resources/hello.txt",
        NULL,
    };

    assert(solve_path(config->home_path, sizeof config->home_path, "."));

    char stdout_buf[1024] = {0};
    CapCatCmd *catcmd = CapCatCmd_New(config, argc, argv);

    setbuf(stdout, stdout_buf);
    CapCatCmd_Run(catcmd);
    assert(!strcmp(stdout_buf, "hello\n"));
    setbuf(stdout, NULL);

    CapCatCmd_Del(catcmd);
    CapConfig_Del(config);
}

static void
test_catcmd_indent_opt(void) {
    CapConfig *config = CapConfig_New();
    int argc = 4;
    char *argv[] = {
        "cat",
        "/tests_env/resources/hello.txt",
        "-i",
        "2",
        NULL,
    };

    assert(solve_path(config->home_path, sizeof config->home_path, "."));

    char stdout_buf[1024] = {0};
    CapCatCmd *catcmd = CapCatCmd_New(config, argc, argv);

    setbuf(stdout, stdout_buf);
    CapCatCmd_Run(catcmd);
    assert(!strcmp(stdout_buf, "        hello\n"));
    setbuf(stdout, NULL);

    CapCatCmd_Del(catcmd);
    CapConfig_Del(config);
}

static void
test_CapCatCmdab_opt(void) {
    CapConfig *config = CapConfig_New();
    int argc = 5;
    char *argv[] = {
        "cat",
        "/tests_env/resources/hello.txt",
        "-i",
        "2",
        "-t",
        NULL,
    };

    assert(solve_path(config->home_path, sizeof config->home_path, "."));

    char stdout_buf[1024] = {0};
    CapCatCmd *catcmd = CapCatCmd_New(config, argc, argv);

    setbuf(stdout, stdout_buf);
    CapCatCmd_Run(catcmd);
    assert(!strcmp(stdout_buf, "\t\thello\n"));
    setbuf(stdout, NULL);

    CapCatCmd_Del(catcmd);
    CapConfig_Del(config);
}

static void
test_CapCatCmdabspaces_opt(void) {
    CapConfig *config = CapConfig_New();
    int argc = 6;
    char *argv[] = {
        "cat",
        "/tests_env/resources/hello.txt",
        "-i",
        "2",
        "-T",
        "2",
        NULL,
    };

    assert(solve_path(config->home_path, sizeof config->home_path, "."));

    char stdout_buf[1024] = {0};
    CapCatCmd *catcmd = CapCatCmd_New(config, argc, argv);

    setbuf(stdout, stdout_buf);
    CapCatCmd_Run(catcmd);
    assert(!strcmp(stdout_buf, "    hello\n"));
    setbuf(stdout, NULL);

    CapCatCmd_Del(catcmd);
    CapConfig_Del(config);
}

static void
test_catcmd_make_opt(void) {
    CapConfig *config = CapConfig_New();
    int argc = 3;
    char *argv[] = {
        "cat",
        "-m",
        "/tests_env/resources/hello.cap",
        NULL,
    };

    assert(solve_path(config->home_path, sizeof config->home_path, "."));

    char stdout_buf[1024] = {0};
    CapCatCmd *catcmd = CapCatCmd_New(config, argc, argv);

    setbuf(stdout, stdout_buf);
    CapCatCmd_Run(catcmd);
    assert(!strcmp(stdout_buf, "hello"));
    setbuf(stdout, NULL);

    CapCatCmd_Del(catcmd);
    CapConfig_Del(config);
}

/**
 * TODO
 */
static void
test_catcmd_make_opt_1(void) {
    CapConfig *config = CapConfig_New();
    int argc = 4;
    char *argv[] = {
        "cat",
        "-m",
        "/tests_env/resources/hello.cap",
        "/tests_env/resources/hello.cap",
        NULL,
    };

    assert(solve_path(config->home_path, sizeof config->home_path, "."));

    char buf[1024] = {0};
    CapCatCmd *catcmd = CapCatCmd_New(config, argc, argv);

    // _IOFBF is full buffering mode
    // stdout use line buffer mode default
    setvbuf(stdout, buf, _IOFBF, sizeof buf);
    // CapCatCmd_Run(catcmd);
    // assert(!strcmp(buf, "hellohello"));
    // setvbuf(stdout, NULL, _IOLBF, 0);
    puts("ababa"); // <- missing
    puts("higege");
    fflush(stdout);
    setbuf(stdout, NULL);

    // why not write "ababa" at buffer?
    fprintf(stderr, "stdout[%s]\n", buf);

    CapCatCmd_Del(catcmd);
    CapConfig_Del(config);
}

static void
test_catcmd_all(void) {
    test_catcmd_default();
    test_catcmd_indent_opt();
    test_CapCatCmdab_opt();
    test_CapCatCmdabspaces_opt();
    test_catcmd_make_opt();
    // test_catcmd_make_opt_1();
}

static const struct testcase
CapCatCmdests[] = {
    {"default", test_catcmd_default},
    {"indent", test_catcmd_indent_opt},
    {"tab", test_CapCatCmdab_opt},
    {"tabspaces", test_CapCatCmdabspaces_opt},
    {"make", test_catcmd_make_opt},
    {"make_1", test_catcmd_make_opt_1},
    {"all", test_catcmd_all},
    {0},
};

/***************
* make command *
***************/

static void
test_makecmd_default(void) {
    CapConfig *config = CapConfig_New();
    int argc = 2;
    char *argv[] = {
        "make",
        "test.cap",
        NULL,
    };

    config->scope = CAP_SCOPE__LOCAL;
    assert(solve_path(config->home_path, sizeof config->home_path, "./tests_env/make"));
    assert(solve_path(config->cd_path, sizeof config->cd_path, "./tests_env/make"));

    char buf[1024] = {0};
    setvbuf(stdout, buf, _IOFBF, sizeof buf);

    CapMakeCmd *makecmd = CapMakeCmd_New(config, argc, argv);
    CapMakeCmd_Run(makecmd);
    CapMakeCmd_Del(makecmd);

    fflush(stdout);
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    assert(!strcmp(buf, "1"));

    CapConfig_Del(config);
}

static void
test_makecmd_options(void) {
    CapConfig *config = CapConfig_New();
    int argc = 5;
    char *argv[] = {
        "make",
        "test2.cap",
        "--name",
        "alice",
        "-h",
        NULL,
    };

    config->scope = CAP_SCOPE__LOCAL;
    assert(solve_path(config->home_path, sizeof config->home_path, "./tests_env/make"));
    assert(solve_path(config->cd_path, sizeof config->cd_path, "./tests_env/make"));

    char buf[1024] = {0};
    setvbuf(stdout, buf, _IOFBF, sizeof buf);

    CapMakeCmd *makecmd = CapMakeCmd_New(config, argc, argv);
    CapMakeCmd_Run(makecmd);
    CapMakeCmd_Del(makecmd);

    fflush(stdout);
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    assert(!strcmp(buf, "alice\ntrue"));

    CapConfig_Del(config);
}

static const struct testcase
CapMakeCmdests[] = {
    {"default", test_makecmd_default},
    {"options", test_makecmd_options},
    {0},
};

/**************
* run command *
**************/

static void
test_runcmd_default(void) {
    // using Pad_SafeSystem
}

static const struct testcase
CapRunCmdests[] = {
    {"default", test_runcmd_default},
    {0},
};

/***************
* exec command *
***************/

static void
test_execcmd_default(void) {
    // using Pad_SafeSystem or fork
}

static const struct testcase
CapExecCmdests[] = {
    {"default", test_execcmd_default},
    {0},
};

/****************
* alias command *
****************/

static void
test_alcmd_default(void) {

    return;  // TODO: buffering is not working

    CapConfig *config = CapConfig_New();
    int argc = 1;
    char *argv[] = {
        "alias",
        NULL,
    };

    assert(solve_path(config->home_path, sizeof config->home_path, "./tests_env/alias"));
    assert(solve_path(config->cd_path, sizeof config->cd_path, "./tests_env/alias"));

    char buf[1024] = {0};
    setvbuf(stdout, buf, _IOFBF, sizeof buf);

    CapAlCmd *alcmd = CapAlCmd_New(config, argc, argv);
    CapAlCmd_Run(alcmd);
    CapAlCmd_Del(alcmd);

    fflush(stdout);
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    assert(!strcmp(buf, "aaa    AAA\nbbb    BBB\n"));

    CapConfig_Del(config);
}

static const struct testcase
CapAlCmdests[] = {
    {"default", test_alcmd_default},
    {0},
};

/***************
* edit command *
***************/

static void
test_editcmd_default(void) {
    // using Pad_SafeSystem
}

static const struct testcase
CapEditCmdests[] = {
    {"default", test_editcmd_default},
    {0},
};

/*****************
* editor command *
*****************/

static void
test_editorcmd_default(void) {
    // using Pad_SafeSystem
    CapConfig *config = CapConfig_New();
    int argc = 2;
    char *argv[] = {
        "editor",
        "/path/to/editor",
        NULL,
    };

    assert(solve_path(config->var_editor_path, sizeof config->var_editor_path, "./tests_env/editor/editor"));

    CapEditorCmd *editorcmd = CapEditorCmd_New(config, argc, argv);
    CapEditorCmd_Run(editorcmd);
    CapEditorCmd_Del(editorcmd);

    char line[256];
    assert(PadFile_ReadLine(line, sizeof line, "./tests_env/editor/editor"));
    assert(!strcmp(line, "/path/to/editor"));

    PadFile_Remove("./tests_env/editor/editor");

    CapConfig_Del(config);
}

static const struct testcase
CapEditorCmdests[] = {
    {"default", test_editorcmd_default},
    {0},
};

/****************
* mkdir command *
****************/

static void
test_mkdircmd_default(void) {
    // using Pad_SafeSystem
    CapConfig *config = CapConfig_New();
    int argc = 2;
    char *argv[] = {
        "mkdir",
        "dir",
        NULL,
    };

    config->scope = CAP_SCOPE__LOCAL;
    assert(solve_path(config->home_path, sizeof config->home_path, "."));
    assert(solve_path(config->cd_path, sizeof config->cd_path, "./tests_env/mkdir"));

    PadFile_Remove("./tests_env/mkdir/dir");
    assert(!PadFile_IsExists("./tests_env/mkdir/dir"));

    CapMkdirCmd *mkdircmd = CapMkdirCmd_New(config, argc, argv);
    CapMkdirCmd_Run(mkdircmd);
    CapMkdirCmd_Del(mkdircmd);

    assert(PadFile_IsExists("./tests_env/mkdir/dir"));

    PadFile_Remove("./tests_env/mkdir/dir");

    CapConfig_Del(config);
}

static const struct testcase
CapMkdirCmdests[] = {
    {"default", test_mkdircmd_default},
    {0},
};

/*************
* rm command *
*************/

static void
test_rmcmd_default(void) {
#ifdef CAP_TESTS__WINDOWS
    return;  // rm command has permission denied error on Windows
#endif

    // using Pad_SafeSystem
    CapConfig *config = CapConfig_New();
    int argc = 2;
    char *argv[] = {
        "rm",
        "file1",
        NULL,
    };

    config->scope = CAP_SCOPE__LOCAL;
    assert(solve_path(config->home_path, sizeof config->home_path, "."));
    assert(solve_path(config->cd_path, sizeof config->cd_path, "./tests_env/rm"));

    PadFile_Trunc("./tests_env/rm/file1");
    assert(PadFile_IsExists("./tests_env/rm/file1"));

    CapRmCmd *rmcmd = CapRmCmd_New(config, argc, argv);
    CapRmCmd_Run(rmcmd);
    CapRmCmdErrno errn = CapRmCmd_Errno(rmcmd);
    switch (errn) {
    case CAP_RMCMD_ERR__NOERR:
        break;
    default:
        fprintf(stderr, "failed to run rm command. %s %s\n",
            CapRmCmd_What(rmcmd), strerror(errno));
        break;
    }
    CapRmCmd_Del(rmcmd);

    assert(!PadFile_IsExists("./tests_env/rm/file1"));

    CapConfig_Del(config);
}

static void
test_rmcmd_multi(void) {
#ifdef CAP_TESTS__WINDOWS
    return;  // rm command has permission denied error on Windows
#endif

    // using Pad_SafeSystem
    CapConfig *config = CapConfig_New();
    int argc = 3;
    char *argv[] = {
        "rm",
        "file1",
        "file2",
        NULL,
    };

    config->scope = CAP_SCOPE__LOCAL;
    assert(solve_path(config->home_path, sizeof config->home_path, "."));
    assert(solve_path(config->cd_path, sizeof config->cd_path, "./tests_env/rm"));

    PadFile_Trunc("./tests_env/rm/file1");
    PadFile_Trunc("./tests_env/rm/file2");
    assert(PadFile_IsExists("./tests_env/rm/file1"));
    assert(PadFile_IsExists("./tests_env/rm/file2"));

    CapRmCmd *rmcmd = CapRmCmd_New(config, argc, argv);
    CapRmCmd_Run(rmcmd);
    CapRmCmd_Del(rmcmd);

    assert(!PadFile_IsExists("./tests_env/rm/file1"));
    assert(!PadFile_IsExists("./tests_env/rm/file2"));

    CapConfig_Del(config);
}

static void
test_rmcmd_dir(void) {
    // using Pad_SafeSystem
    CapConfig *config = CapConfig_New();
    int argc = 3;
    char *argv[] = {
        "rm",
        "dir1",
        "-r",
        NULL,
    };

    config->scope = CAP_SCOPE__LOCAL;
    assert(solve_path(config->home_path, sizeof config->home_path, "."));
    assert(solve_path(config->cd_path, sizeof config->cd_path, "./tests_env/rm"));

    if (!PadFile_IsExists("./tests_env/rm/dir1")) {
        PadFile_MkdirQ("./tests_env/rm/dir1");
    }
    assert(PadFile_IsExists("./tests_env/rm/dir1"));

    CapRmCmd *rmcmd = CapRmCmd_New(config, argc, argv);
    CapRmCmd_Run(rmcmd);
    CapRmCmd_Del(rmcmd);

    assert(!PadFile_IsExists("./tests_env/rm/dir1"));

    CapConfig_Del(config);
}

static void
test_rmcmd_dir_multi(void) {
    // using Pad_SafeSystem
    CapConfig *config = CapConfig_New();
    int argc = 3;
    char *argv[] = {
        "rm",
        "dir1",
        "dir2",
        NULL,
    };

    config->scope = CAP_SCOPE__LOCAL;
    assert(solve_path(config->home_path, sizeof config->home_path, "."));
    assert(solve_path(config->cd_path, sizeof config->cd_path, "./tests_env/rm"));

    PadFile_MkdirQ("./tests_env/rm/dir1");
    PadFile_MkdirQ("./tests_env/rm/dir2");
    assert(PadFile_IsExists("./tests_env/rm/dir1"));
    assert(PadFile_IsExists("./tests_env/rm/dir2"));

    CapRmCmd *rmcmd = CapRmCmd_New(config, argc, argv);
    CapRmCmd_Run(rmcmd);
    CapRmCmd_Del(rmcmd);

    assert(!PadFile_IsExists("./tests_env/rm/dir1"));
    assert(!PadFile_IsExists("./tests_env/rm/dir2"));

    CapConfig_Del(config);
}

static void
test_rmcmd_dir_r(void) {
    // using Pad_SafeSystem
    CapConfig *config = CapConfig_New();
    int argc = 3;
    char *argv[] = {
        "rm",
        "dir1",
        "-r",
        NULL,
    };

    config->scope = CAP_SCOPE__LOCAL;
    assert(solve_path(config->home_path, sizeof config->home_path, "."));
    assert(solve_path(config->cd_path, sizeof config->cd_path, "./tests_env/rm"));

    if (!PadFile_IsExists("./tests_env/rm/dir1")) {
        PadFile_MkdirQ("./tests_env/rm/dir1");
    }
    if (!PadFile_IsExists("./tests_env/rm/dir1/file1")) {
        PadFile_Trunc("./tests_env/rm/dir1/file1");
    }
    if (!PadFile_IsExists("./tests_env/rm/dir1/file2")) {
        PadFile_Trunc("./tests_env/rm/dir1/file2");
    }
    assert(PadFile_IsExists("./tests_env/rm/dir1"));
    assert(PadFile_IsExists("./tests_env/rm/dir1/file1"));
    assert(PadFile_IsExists("./tests_env/rm/dir1/file2"));

    CapRmCmd *rmcmd = CapRmCmd_New(config, argc, argv);
    CapRmCmd_Run(rmcmd);
    CapRmCmd_Del(rmcmd);

    assert(!PadFile_IsExists("./tests_env/rm/dir1"));

    CapConfig_Del(config);
}

static void
test_rmcmd_dir_r_multi(void) {
    // using Pad_SafeSystem
    CapConfig *config = CapConfig_New();
    int argc = 4;
    char *argv[] = {
        "rm",
        "dir1",
        "dir2",
        "-r",
        NULL,
    };

    config->scope = CAP_SCOPE__LOCAL;
    assert(solve_path(config->home_path, sizeof config->home_path, "."));
    assert(solve_path(config->cd_path, sizeof config->cd_path, "./tests_env/rm"));

    if (!PadFile_IsExists("./tests_env/rm/dir1")) {
        PadFile_MkdirQ("./tests_env/rm/dir1");
    }
    if (!PadFile_IsExists("./tests_env/rm/dir1/file1")) {
        PadFile_Trunc("./tests_env/rm/dir1/file1");
    }
    if (!PadFile_IsExists("./tests_env/rm/dir1/file2")) {
        PadFile_Trunc("./tests_env/rm/dir1/file2");
    }
    assert(PadFile_IsExists("./tests_env/rm/dir1"));
    assert(PadFile_IsExists("./tests_env/rm/dir1/file1"));
    assert(PadFile_IsExists("./tests_env/rm/dir1/file2"));

    if (!PadFile_IsExists("./tests_env/rm/dir2")) {
        PadFile_MkdirQ("./tests_env/rm/dir2");
    }
    if (!PadFile_IsExists("./tests_env/rm/dir2/file1")) {
        PadFile_Trunc("./tests_env/rm/dir2/file1");
    }
    if (!PadFile_IsExists("./tests_env/rm/dir2/file2")) {
        PadFile_Trunc("./tests_env/rm/dir2/file2");
    }
    assert(PadFile_IsExists("./tests_env/rm/dir2"));
    assert(PadFile_IsExists("./tests_env/rm/dir2/file1"));
    assert(PadFile_IsExists("./tests_env/rm/dir2/file2"));

    CapRmCmd *rmcmd = CapRmCmd_New(config, argc, argv);
    CapRmCmd_Run(rmcmd);
    CapRmCmd_Del(rmcmd);

    assert(!PadFile_IsExists("./tests_env/rm/dir1"));
    assert(!PadFile_IsExists("./tests_env/rm/dir2"));

    CapConfig_Del(config);
}

static const struct testcase
CapRmCmdests[] = {
    {"default", test_rmcmd_default},
    {"multi", test_rmcmd_multi},
    {"dir", test_rmcmd_dir},
    {"dir_multi", test_rmcmd_dir_multi},
    {"dir_r", test_rmcmd_dir_r},
    {"dir_r_multi", test_rmcmd_dir_r_multi},
    {0},
};

/*************
* mv command *
*************/

static void
test_mvcmd_default(void) {
#ifdef CAP_TESTS__WINDOWS
    return;  // mv command has permission denied error on Windows
#endif
    // using Pad_SafeSystem
    CapConfig *config = CapConfig_New();
    int argc = 3;
    char *argv[] = {
        "mv",
        "file1",
        "file2",
        NULL,
    };

    config->scope = CAP_SCOPE__LOCAL;
    assert(solve_path(config->home_path, sizeof config->home_path, "."));
    assert(solve_path(config->cd_path, sizeof config->cd_path, "./tests_env/mv"));

    PadFile_Trunc("./tests_env/mv/file1");
    assert(PadFile_IsExists("./tests_env/mv/file1"));
    assert(!PadFile_IsExists("./tests_env/mv/file2"));

    CapMvCmd *mvcmd = CapMvCmd_New(config, argc, argv);
    CapMvCmd_Run(mvcmd);
    CapMvCmd_Del(mvcmd);

    // rename("./tests_env/mv/file1", "./tests_env/mv/file2");  // ok
    // rename("/mnt/d/src/cap/tests_env/mv/file1", "/mnt/d/src/cap/tests_env/mv/file2");  // ok

    // rename("/mnt/d/src/cap/tests_env/mv/file1", "/mnt/d/src/cap/tests_env/mv/file2");

    assert(!PadFile_IsExists("./tests_env/mv/file1"));
    assert(PadFile_IsExists("./tests_env/mv/file2")); 

    PadFile_Remove("./tests_env/mv/file2");

    CapConfig_Del(config);
}

static void
test_mvcmd_dir(void) {
#ifdef CAP_TESTS__WINDOWS
    return;  // mv command has permission denied error on Windows
#endif
    // using Pad_SafeSystem
    CapConfig *config = CapConfig_New();
    int argc = 3;
    char *argv[] = {
        "mv",
        "dir1",
        "dir2",
        NULL,
    };

    config->scope = CAP_SCOPE__LOCAL;
    assert(solve_path(config->home_path, sizeof config->home_path, "."));
    assert(solve_path(config->cd_path, sizeof config->cd_path, "./tests_env/mv"));

    PadFile_MkdirQ("./tests_env/mv/dir1");
    assert(PadFile_IsExists("./tests_env/mv/dir1"));

    CapMvCmd *mvcmd = CapMvCmd_New(config, argc, argv);
    CapMvCmd_Run(mvcmd);
    CapMvCmd_Del(mvcmd);

    assert(!PadFile_IsExists("./tests_env/mv/dir1"));
    assert(PadFile_IsExists("./tests_env/mv/dir2"));

    PadFile_Remove("./tests_env/mv/dir2");

    CapConfig_Del(config);
}

static void
test_mvcmd_file_to_dir(void) {
#ifdef CAP_TESTS__WINDOWS
    return;  // mv command has permission denied error on Windows
#endif
    // using Pad_SafeSystem
    CapConfig *config = CapConfig_New();
    int argc = 3;
    char *argv[] = {
        "mv",
        "file1",
        "dir1",
        NULL,
    };

    config->scope = CAP_SCOPE__LOCAL;
    assert(solve_path(config->home_path, sizeof config->home_path, "."));
    assert(solve_path(config->cd_path, sizeof config->cd_path, "./tests_env/mv"));

    PadFile_Trunc("./tests_env/mv/file1");
    PadFile_MkdirQ("./tests_env/mv/dir1");
    assert(PadFile_IsExists("./tests_env/mv/file1"));
    assert(PadFile_IsExists("./tests_env/mv/dir1"));

    CapMvCmd *mvcmd = CapMvCmd_New(config, argc, argv);
    CapMvCmd_Run(mvcmd);
    CapMvCmd_Del(mvcmd);

    assert(!PadFile_IsExists("./tests_env/mv/file1"));
    assert(PadFile_IsExists("./tests_env/mv/dir1"));
    assert(PadFile_IsExists("./tests_env/mv/dir1/file1"));

    PadFile_Remove("./tests_env/mv/dir1/file1");
    PadFile_Remove("./tests_env/mv/dir1");

    CapConfig_Del(config);
}

static void
test_mvcmd_files_to_dir(void) {
#ifdef CAP_TESTS__WINDOWS
    return;  // mv command has permission denied error on Windows
#endif
    // using Pad_SafeSystem
    CapConfig *config = CapConfig_New();
    int argc = 4;
    char *argv[] = {
        "mv",
        "file1",
        "file2",
        "dir1",
        NULL,
    };

    config->scope = CAP_SCOPE__LOCAL;
    assert(solve_path(config->home_path, sizeof config->home_path, "."));
    assert(solve_path(config->cd_path, sizeof config->cd_path, "./tests_env/mv"));

    PadFile_Trunc("./tests_env/mv/file1");
    PadFile_Trunc("./tests_env/mv/file2");
    PadFile_MkdirQ("./tests_env/mv/dir1");
    assert(PadFile_IsExists("./tests_env/mv/file1"));
    assert(PadFile_IsExists("./tests_env/mv/file2"));
    assert(PadFile_IsExists("./tests_env/mv/dir1"));

    CapMvCmd *mvcmd = CapMvCmd_New(config, argc, argv);
    CapMvCmd_Run(mvcmd);
    CapMvCmd_Del(mvcmd);

    assert(!PadFile_IsExists("./tests_env/mv/file1"));
    assert(!PadFile_IsExists("./tests_env/mv/file2"));
    assert(PadFile_IsExists("./tests_env/mv/dir1"));
    assert(PadFile_IsExists("./tests_env/mv/dir1/file1"));
    assert(PadFile_IsExists("./tests_env/mv/dir1/file2"));

    PadFile_Remove("./tests_env/mv/dir1/file1");
    PadFile_Remove("./tests_env/mv/dir1/file2");
    PadFile_Remove("./tests_env/mv/dir1");

    CapConfig_Del(config);
}

static void
test_mvcmd_err_1(void) {
#ifdef CAP_TESTS__WINDOWS
    return;  // mv command has permission denied error on Windows
#endif
    // using Pad_SafeSystem
    CapConfig *config = CapConfig_New();
    int argc = 3;
    char *argv[] = {
        "mv",
        "dir1",
        "file1",
        NULL,
    };

    config->scope = CAP_SCOPE__LOCAL;
    assert(solve_path(config->home_path, sizeof config->home_path, "."));
    assert(solve_path(config->cd_path, sizeof config->cd_path, "./tests_env/mv"));

    PadFile_MkdirQ("./tests_env/mv/dir1");
    PadFile_Trunc("./tests_env/mv/file1");
    assert(PadFile_IsExists("./tests_env/mv/dir1"));
    assert(PadFile_IsExists("./tests_env/mv/file1"));

    char buf[1024] = {0};
    setbuf(stderr, buf);

    CapMvCmd *mvcmd = CapMvCmd_New(config, argc, argv);
    CapMvCmd_Run(mvcmd);
    CapMvCmd_Del(mvcmd);

    setbuf(stderr, NULL);
    assert(strstr(buf, "Failed to rename"));

    PadFile_Remove("./tests_env/mv/dir1");
    PadFile_Remove("./tests_env/mv/file1");

    CapConfig_Del(config);
}

static const struct testcase
CapMvCmdests[] = {
    {"default", test_mvcmd_default},
    {"dir", test_mvcmd_dir},
    {"file_to_dir", test_mvcmd_file_to_dir},
    {"files_to_dir", test_mvcmd_files_to_dir},
    {"err_1", test_mvcmd_err_1},
    {0},
};

/*************
* cp command *
*************/

static void
test_cpcmd_default(void) {
    // using Pad_SafeSystem
    CapConfig *config = CapConfig_New();
    int argc = 3;
    char *argv[] = {
        "cp",
        "file1",
        "file2",
        NULL,
    };

    config->scope = CAP_SCOPE__LOCAL;
    assert(solve_path(config->home_path, sizeof config->home_path, "."));
    assert(solve_path(config->cd_path, sizeof config->cd_path, "./tests_env/cp"));

    PadFile_Trunc("./tests_env/cp/file1");
    assert(PadFile_IsExists("./tests_env/cp/file1"));

    CapCpCmd *cpcmd = CapCpCmd_New(config, argc, argv);
    CapCpCmd_Run(cpcmd);
    CapCpCmd_Del(cpcmd);

    assert(PadFile_IsExists("./tests_env/cp/file1"));
    assert(PadFile_IsExists("./tests_env/cp/file2"));

    PadFile_Remove("./tests_env/cp/file1");
    PadFile_Remove("./tests_env/cp/file2");

    CapConfig_Del(config);
}

static void
test_cpcmd_dir(void) {
    // using Pad_SafeSystem
    CapConfig *config = CapConfig_New();
    int argc = 3;
    char *argv[] = {
        "cp",
        "file1",
        "dir1",
        NULL,
    };

    config->scope = CAP_SCOPE__LOCAL;
    assert(solve_path(config->home_path, sizeof config->home_path, "."));
    assert(solve_path(config->cd_path, sizeof config->cd_path, "./tests_env/cp"));

    PadFile_Trunc("./tests_env/cp/file1");
    PadFile_MkdirQ("./tests_env/cp/dir1");
    assert(PadFile_IsExists("./tests_env/cp/file1"));
    assert(PadFile_IsExists("./tests_env/cp/dir1"));

    CapCpCmd *cpcmd = CapCpCmd_New(config, argc, argv);
    CapCpCmd_Run(cpcmd);
    CapCpCmd_Del(cpcmd);

    assert(PadFile_IsExists("./tests_env/cp/file1"));
    assert(PadFile_IsExists("./tests_env/cp/dir1/file1"));

    PadFile_Remove("./tests_env/cp/file1");
    PadFile_Remove("./tests_env/cp/dir1/file1");
    PadFile_Remove("./tests_env/cp/dir1");

    CapConfig_Del(config);
}

static void
test_cpcmd_files_to_dir(void) {
    // using Pad_SafeSystem
    CapConfig *config = CapConfig_New();
    int argc = 4;
    char *argv[] = {
        "cp",
        "file1",
        "file2",
        "dir1",
        NULL,
    };

    config->scope = CAP_SCOPE__LOCAL;
    assert(solve_path(config->home_path, sizeof config->home_path, "."));
    assert(solve_path(config->cd_path, sizeof config->cd_path, "./tests_env/cp"));

    PadFile_Trunc("./tests_env/cp/file1");
    PadFile_Trunc("./tests_env/cp/file2");
    PadFile_MkdirQ("./tests_env/cp/dir1");
    assert(PadFile_IsExists("./tests_env/cp/file1"));
    assert(PadFile_IsExists("./tests_env/cp/file2"));
    assert(PadFile_IsExists("./tests_env/cp/dir1"));

    CapCpCmd *cpcmd = CapCpCmd_New(config, argc, argv);
    CapCpCmd_Run(cpcmd);
    CapCpCmd_Del(cpcmd);

    assert(PadFile_IsExists("./tests_env/cp/file1"));
    assert(PadFile_IsExists("./tests_env/cp/file2"));
    assert(PadFile_IsExists("./tests_env/cp/dir1/file1"));
    assert(PadFile_IsExists("./tests_env/cp/dir1/file2"));

    PadFile_Remove("./tests_env/cp/file1");
    PadFile_Remove("./tests_env/cp/file2");
    PadFile_Remove("./tests_env/cp/dir1/file1");
    PadFile_Remove("./tests_env/cp/dir1/file2");
    PadFile_Remove("./tests_env/cp/dir1");

    CapConfig_Del(config);
}

static void
test_cpcmd_dir_r(void) {
    // using Pad_SafeSystem
    CapConfig *config = CapConfig_New();
    int argc = 4;
    char *argv[] = {
        "cp",
        "dir1",
        "dir2",
        "-r",
        NULL,
    };

    config->scope = CAP_SCOPE__LOCAL;
    assert(solve_path(config->home_path, sizeof config->home_path, "."));
    assert(solve_path(config->cd_path, sizeof config->cd_path, "./tests_env/cp"));

    PadFile_MkdirQ("./tests_env/cp/dir1");
    PadFile_Trunc("./tests_env/cp/dir1/file1");
    assert(PadFile_IsExists("./tests_env/cp/dir1/file1"));

    CapCpCmd *cpcmd = CapCpCmd_New(config, argc, argv);
    CapCpCmd_Run(cpcmd);
    CapCpCmd_Del(cpcmd);

    assert(PadFile_IsExists("./tests_env/cp/dir1/file1"));
    assert(PadFile_IsExists("./tests_env/cp/dir2/file1"));

    PadFile_Remove("./tests_env/cp/dir1/file1");
    PadFile_Remove("./tests_env/cp/dir1");
    PadFile_Remove("./tests_env/cp/dir2/file1");
    PadFile_Remove("./tests_env/cp/dir2");

    CapConfig_Del(config);
}

static void
test_cpcmd_dirs_r(void) {
    // using Pad_SafeSystem
    CapConfig *config = CapConfig_New();
    int argc = 5;
    char *argv[] = {
        "cp",
        "dir1",
        "dir2",
        "dir3",
        "-r",
        NULL,
    };

    config->scope = CAP_SCOPE__LOCAL;
    assert(solve_path(config->home_path, sizeof config->home_path, "."));
    assert(solve_path(config->cd_path, sizeof config->cd_path, "./tests_env/cp"));

    PadFile_MkdirQ("./tests_env/cp/dir1");
    PadFile_MkdirQ("./tests_env/cp/dir2");
    PadFile_Trunc("./tests_env/cp/dir1/file1");
    PadFile_Trunc("./tests_env/cp/dir2/file1");
    assert(PadFile_IsExists("./tests_env/cp/dir1/file1"));
    assert(PadFile_IsExists("./tests_env/cp/dir2/file1"));

    CapCpCmd *cpcmd = CapCpCmd_New(config, argc, argv);
    CapCpCmd_Run(cpcmd);
    CapCpCmd_Del(cpcmd);

    assert(PadFile_IsExists("./tests_env/cp/dir1/file1"));
    assert(PadFile_IsExists("./tests_env/cp/dir2/file1"));
    assert(PadFile_IsExists("./tests_env/cp/dir3/file1"));
    assert(PadFile_IsExists("./tests_env/cp/dir3/dir2/file1"));

    PadFile_Remove("./tests_env/cp/dir1/file1");
    PadFile_Remove("./tests_env/cp/dir1");
    PadFile_Remove("./tests_env/cp/dir2/file1");
    PadFile_Remove("./tests_env/cp/dir2");
    PadFile_Remove("./tests_env/cp/dir3/file1");
    PadFile_Remove("./tests_env/cp/dir3/dir2/file1");
    PadFile_Remove("./tests_env/cp/dir3/dir2");
    PadFile_Remove("./tests_env/cp/dir3");

    CapConfig_Del(config);
}

static const struct testcase
CapCpCmdests[] = {
    {"default", test_cpcmd_default},
    {"dir", test_cpcmd_dir},
    {"files_to_dir", test_cpcmd_files_to_dir},
    {"dir_r", test_cpcmd_dir_r},
    {"dirs_r", test_cpcmd_dirs_r},
    {0},
};

/****************
* touch command *
****************/

static void
test_touchcmd_default(void) {
    // using Pad_SafeSystem
    CapConfig *config = CapConfig_New();
    int argc = 2;
    char *argv[] = {
        "touch",
        "file1",
        NULL,
    };

    config->scope = CAP_SCOPE__LOCAL;
    assert(solve_path(config->home_path, sizeof config->home_path, "."));
    assert(solve_path(config->cd_path, sizeof config->cd_path, "./tests_env/touch"));

    assert(!PadFile_IsExists("./tests_env/touch/file1"));

    CapTouchCmd *touchcmd = CapTouchCmd_New(config, argc, argv);
    CapTouchCmd_Run(touchcmd);
    CapTouchCmd_Del(touchcmd);

    assert(PadFile_IsExists("./tests_env/touch/file1"));

    PadFile_Remove("./tests_env/touch/file1");

    CapConfig_Del(config);
}

static void
test_touchcmd_multi(void) {
    // using Pad_SafeSystem
    CapConfig *config = CapConfig_New();
    int argc = 3;
    char *argv[] = {
        "touch",
        "file1",
        "file2",
        NULL,
    };

    config->scope = CAP_SCOPE__LOCAL;
    assert(solve_path(config->home_path, sizeof config->home_path, "."));
    assert(solve_path(config->cd_path, sizeof config->cd_path, "./tests_env/touch"));

    assert(!PadFile_IsExists("./tests_env/touch/file1"));
    assert(!PadFile_IsExists("./tests_env/touch/file2"));

    CapTouchCmd *touchcmd = CapTouchCmd_New(config, argc, argv);
    CapTouchCmd_Run(touchcmd);
    CapTouchCmd_Del(touchcmd);

    assert(PadFile_IsExists("./tests_env/touch/file1"));
    assert(PadFile_IsExists("./tests_env/touch/file2"));

    PadFile_Remove("./tests_env/touch/file1");
    PadFile_Remove("./tests_env/touch/file2");

    CapConfig_Del(config);
}

static const struct testcase
CapTouchCmdests[] = {
    {"default", test_touchcmd_default},
    {"multi", test_touchcmd_multi},
    {0},
};

/******************
* snippet command *
******************/

static void
test_snippetcmd_default(void) {
    // using Pad_SafeSystem
    CapConfig *config = CapConfig_New();
    int argc = 1;
    char *argv[] = {
        "snippet",
        NULL,
    };

    CapSnptCmd *snptcmd = CapSnptCmd_New(config, argc, argv);
    int result = CapSnptCmd_Run(snptcmd);
    CapSnptCmd_Del(snptcmd);

    assert(result == 0);

    CapConfig_Del(config);
}

static void
test_snippetcmd_add(void) {
    // ==2226== Syscall param read(buf) points to unaddressable byte(s)
    // ==2226==    at 0x4F31260: __read_nocancel (syscall-template.S:84)
    // ==2226==    by 0x4EB45E7: _IO_file_underflow@@GLIBC_2.2.5 (fileops.c:592)
    // ==2226==    by 0x4EB560D: _IO_default_uflow (genops.c:413)
    // ==2226==    by 0x4EB0107: getc (getc.c:38)
    // ==2226==    by 0x46F893: snptcmd_add (snippet.c:106)
    return;

    // using Pad_SafeSystem
    CapConfig *config = CapConfig_New();
    int argc = 3;
    char *argv[] = {
        "snippet",
        "add",
        "mysnippet",
        NULL,
    };

    char buf[1024] = "test";
    setbuf(stdin, buf);

    assert(solve_path(config->codes_dir_path, sizeof config->codes_dir_path, "./tests_env/snippet"));
    CapSnptCmd *snptcmd = CapSnptCmd_New(config, argc, argv);
    int result = CapSnptCmd_Run(snptcmd);
    CapSnptCmd_Del(snptcmd);

    assert(result == 0);

    setbuf(stdin, NULL);
    CapConfig_Del(config);
}

static const struct testcase
snippetcmd_tests[] = {
    {"default", test_snippetcmd_default},
    {"add", test_snippetcmd_add},
    {0},
};

/***************
* link command *
***************/

static void
test_linkcmd_default(void) {
    // using Pad_SafeSystem
    CapConfig *config = CapConfig_New();
    int argc = 3;
    char *argv[] = {
        "link",
        "link-to-a",
        "a",
        NULL,
    };

    config->scope = CAP_SCOPE__LOCAL;
    assert(solve_path(config->home_path, sizeof config->home_path, "."));
    assert(solve_path(config->cd_path, sizeof config->cd_path, "./tests_env/link"));

    assert(!PadFile_IsExists("./tests_env/link/link-to-a"));

    CapLinkCmd *linkcmd = CapLinkCmd_New(config, argc, argv);
    int result = CapLinkCmd_Run(linkcmd);
    CapLinkCmd_Del(linkcmd);
    assert(result == 0);

    assert(PadFile_IsExists("./tests_env/link/link-to-a"));
    PadFile_Remove("./tests_env/link/link-to-a");

    CapConfig_Del(config);
}

static void
test_linkcmd_unlink(void) {
    // using Pad_SafeSystem
    CapConfig *config = CapConfig_New();
    int argc = 3;
    char *argv[] = {
        "link",
        "link-to-a",
        "a",
        NULL,
    };

    config->scope = CAP_SCOPE__LOCAL;
    assert(solve_path(config->home_path, sizeof config->home_path, "."));
    assert(solve_path(config->cd_path, sizeof config->cd_path, "./tests_env/link"));

    assert(!PadFile_IsExists("./tests_env/link/link-to-a"));

    CapLinkCmd *linkcmd = CapLinkCmd_New(config, argc, argv);
    int result = CapLinkCmd_Run(linkcmd);
    CapLinkCmd_Del(linkcmd);
    assert(result == 0);

    assert(PadFile_IsExists("./tests_env/link/link-to-a"));

    // unlink
    int argc2 = 3;
    char *argv2[] = {
        "link",
        "link-to-a",
        "-u",
        NULL,
    };

    linkcmd = CapLinkCmd_New(config, argc2, argv2);
    result = CapLinkCmd_Run(linkcmd);
    CapLinkCmd_Del(linkcmd);
    assert(result == 0);

    assert(!PadFile_IsExists("./tests_env/link/link-to-a"));

    CapConfig_Del(config);
}

static const struct testcase
CapLinkCmdests[] = {
    {"default", test_linkcmd_default},
    {"unlink", test_linkcmd_unlink},
    {0},
};

/***************
* bake command *
***************/

static void
test_bakecmd_1(void) {
    const char *bakefname = "tests_env/bake/target.cap";
    if (!PadFile_CopyPath(bakefname, "tests_env/bake/target.cap.org")) {
        PadErr_Err("failed to copy tests_env/bake/target.cap.org");
        return;
    }
    
    CapConfig *config = CapConfig_New();
    CapBakeCmd *cmd = NULL;
    int argc = 2;
    char *argv[] = {
        "bake",
        ":tests_env/bake/target.cap",
        NULL,
    };
    cmd = CapBakeCmd_New(config, argc, argv);
    CapBakeCmd_Run(cmd);
    CapBakeCmd_Del(cmd);

    char *s = PadFile_ReadCopyFromPath(bakefname);
    assert(strcmp(s, "1\n") == 0);
    free(s);

    PadFile_Remove(bakefname);
    CapConfig_Del(config);
}

static void
test_bakecmd_2(void) {
    const char *bakefname = "tests_env/bake/target.cap";
    if (!PadFile_CopyPath(bakefname, "tests_env/bake/target.cap.org.2")) {
        PadErr_Err("failed to copy tests_env/bake/target.cap.org");
        return;
    }
    
    CapConfig *config = CapConfig_New();
    CapBakeCmd *cmd = NULL;
    int argc = 5;
    char *argv[] = {
        "bake",
        ":tests_env/bake/target.cap",
        "abc",
        "--def",
        "ghi",
        NULL,
    };
    cmd = CapBakeCmd_New(config, argc, argv);
    CapBakeCmd_Run(cmd);
    CapBakeCmd_Del(cmd);

    char *s = PadFile_ReadCopyFromPath(bakefname);
    assert(strcmp(s, "abc,ghi") == 0);
    free(s);

    PadFile_Remove(bakefname);
    CapConfig_Del(config);
}

static const struct testcase
CapBakeCmdests[] = {
    {"1", test_bakecmd_1},
    {"2", test_bakecmd_2},
    {0},
};

static void
test_replacecmd_1(void) {
    PadFile_CopyPath("tests_env/replace/file1.txt", "tests_env/replace/file1.txt.org");

    CapConfig *config = CapConfig_New();
    int argc = 4;
    char *argv[] = {
        "replace",
        ":tests_env/replace/file1.txt",
        "ababa",
        "ABABABA",
    };
    CapReplaceCmd *cmd = CapReplaceCmd_New(config, argc, argv);
    CapReplaceCmd_Run(cmd);
    CapReplaceCmd_Del(cmd);
    CapConfig_Del(config);

    char *s = PadFile_ReadCopyFromPath("tests_env/replace/file1.txt");
    assert(strcmp(s, "abc ABABABA def\n") == 0);
    free(s);

    PadFile_Remove("tests_env/replace/file1.txt");
}

static void
test_replacecmd_2(void) {
    PadFile_CopyPath("tests_env/replace/file2.txt", "tests_env/replace/file2.txt.org");

    CapConfig *config = CapConfig_New();
    int argc = 4;
    char *argv[] = {
        "replace",
        ":tests_env/replace/file2.txt",
        "abcd",
        "ABCD",
    };
    CapReplaceCmd *cmd = CapReplaceCmd_New(config, argc, argv);
    CapReplaceCmd_Run(cmd);
    CapReplaceCmd_Del(cmd);
    CapConfig_Del(config);

    char *s = PadFile_ReadCopyFromPath("tests_env/replace/file2.txt");
    assert(strcmp(s, "ABCDABCD\n") == 0);
    free(s);

    PadFile_Remove("tests_env/replace/file2.txt");
}

static void
test_replacecmd_3(void) {
    PadFile_CopyPath("tests_env/replace/file3.txt", "tests_env/replace/file3.txt.org");

    CapConfig *config = CapConfig_New();
    int argc = 4;
    char *argv[] = {
        "replace",
        ":tests_env/replace/file3.txt",
        "abcd\nefgh",
        "ABABA",
    };
    CapReplaceCmd *cmd = CapReplaceCmd_New(config, argc, argv);
    CapReplaceCmd_Run(cmd);
    CapReplaceCmd_Del(cmd);
    CapConfig_Del(config);

    char *s = PadFile_ReadCopyFromPath("tests_env/replace/file3.txt");
    assert(strcmp(s, "hige\nABABA\nhige\n") == 0);
    free(s);

    PadFile_Remove("tests_env/replace/file3.txt");
}

static const struct testcase
CapReplaceCmdests[] = {
    {"1", test_replacecmd_1},
    {"2", test_replacecmd_2},
    {"3", test_replacecmd_3},
    {0},
};

/************
* lang/opts *
************/

static void
test_lang_CapOpts_New(void) {
    CapOpts *opts = CapOpts_New();
    assert(opts);
    CapOpts_Del(opts);
}

static void
test_lang_CapOpts_Parse(void) {
    CapOpts *opts = CapOpts_New();
    assert(opts);

    int argc = 7;
    char *argv[] = {
        "make",
        "arg1",
        "arg2",
        "-a",
        "aaa",
        "--bbb",
        "bbb",
        NULL,
    };
    assert(CapOpts_Parse(opts, argc, argv));

    assert(CapOpts_ArgsLen(opts) == 3);
    assert(CapOpts_GetcArgs(opts, -1) == NULL);
    assert(CapOpts_GetcArgs(opts, 0));
    assert(CapOpts_GetcArgs(opts, 1));
    assert(CapOpts_GetcArgs(opts, 2));
    assert(CapOpts_GetcArgs(opts, 3) == NULL);
    assert(!strcmp(CapOpts_GetcArgs(opts, 0), "make"));
    assert(!strcmp(CapOpts_GetcArgs(opts, 1), "arg1"));
    assert(!strcmp(CapOpts_GetcArgs(opts, 2), "arg2"));
    assert(CapOpts_Getc(opts, "a"));
    assert(!strcmp(CapOpts_Getc(opts, "a"), "aaa"));
    assert(CapOpts_Getc(opts, "bbb"));
    assert(!strcmp(CapOpts_Getc(opts, "bbb"), "bbb"));
    assert(CapOpts_Has(opts, "a"));
    assert(CapOpts_Has(opts, "bbb"));
    CapOpts_Del(opts);
}

static void
test_lang_CapOpts_Parse_0(void) {
    CapOpts *opts = CapOpts_New();
    assert(opts);

    int argc = 1;
    char *argv[] = {
        "make",
        NULL,
    };
    assert(CapOpts_Parse(opts, argc, argv));
    CapOpts_Del(opts);
}

static void
test_lang_CapOpts_GetcArgs_0(void) {
    CapOpts *opts = CapOpts_New();
    assert(opts);

    int argc = 3;
    char *argv[] = {
        "cmd",
        "arg1",
        "arg2",
        NULL,
    };

    assert(CapOpts_Parse(opts, argc, argv));
    assert(!strcmp(CapOpts_GetcArgs(opts, 0), "cmd"));
    assert(!strcmp(CapOpts_GetcArgs(opts, 1), "arg1"));
    assert(!strcmp(CapOpts_GetcArgs(opts, 2), "arg2"));
    CapOpts_Del(opts);
}

static void
test_lang_CapOpts_GetcArgs_1(void) {
    CapOpts *opts = CapOpts_New();
    assert(opts);

    int argc = 7;
    char *argv[] = {
        "cmd",
        "-a",
        "optarg1",
        "-b",
        "optarg2",
        "arg1",
        "arg2",
        NULL,
    };

    assert(CapOpts_Parse(opts, argc, argv));
    assert(!strcmp(CapOpts_Getc(opts, "a"), "optarg1"));
    assert(!strcmp(CapOpts_Getc(opts, "b"), "optarg2"));
    assert(CapOpts_GetcArgs(opts, 0));
    assert(!strcmp(CapOpts_GetcArgs(opts, 0), "cmd"));
    assert(CapOpts_GetcArgs(opts, 1));
    assert(!strcmp(CapOpts_GetcArgs(opts, 1), "arg1"));
    assert(CapOpts_GetcArgs(opts, 2));
    assert(!strcmp(CapOpts_GetcArgs(opts, 2), "arg2"));
    CapOpts_Del(opts);
}

static void
test_lang_CapOpts_Clear(void) {
    int argc = 1;
    char *argv[] = {"abc", NULL};

    CapOpts *opts = CapOpts_New();

    assert(CapOpts_Parse(opts, argc, argv));
    assert(CapOpts_ArgsLen(opts) == 1);
    CapOpts_Clear(opts);
    assert(CapOpts_ArgsLen(opts) == 0);

    CapOpts_Del(opts);
}

static void
test_lang_CapOpts_Getc(void) {
    int argc = 5;
    char *argv[] = {
        "cmd",
        "-a",
        "aaa",
        "-b",
        "bbb",
        NULL,
    };
    CapOpts *opts = CapOpts_New();

    assert(CapOpts_Parse(opts, argc, argv));
    assert(!strcmp(CapOpts_Getc(opts, "a"), "aaa"));
    assert(!strcmp(CapOpts_Getc(opts, "b"), "bbb"));

    CapOpts_Del(opts);
}

static void
test_lang_CapOpts_Has(void) {
    int argc = 3;
    char *argv[] = {
        "cmd",
        "-a",
        "aaa",
        NULL,
    };
    CapOpts *opts = CapOpts_New();

    assert(CapOpts_Parse(opts, argc, argv));
    assert(CapOpts_Has(opts, "a"));

    CapOpts_Del(opts);
}

static void
test_lang_CapOpts_ArgsLen(void) {
    int argc = 3;
    char *argv[] = {
        "cmd",
        "arg1",
        "arg2",
        NULL,
    };
    CapOpts *opts = CapOpts_New();

    assert(CapOpts_Parse(opts, argc, argv));
    assert(CapOpts_ArgsLen(opts) == 3);

    CapOpts_Del(opts);
}

static const struct testcase
opts_tests[] = {
    {"CapOpts_New", test_lang_CapOpts_New},
    {"CapOpts_Parse", test_lang_CapOpts_Parse},
    {"CapOpts_Parse_0", test_lang_CapOpts_Parse_0},
    {"CapOpts_GetcArgs_0", test_lang_CapOpts_GetcArgs_0},
    {"CapOpts_GetcArgs_1", test_lang_CapOpts_GetcArgs_1},
    {"CapOpts_Clear", test_lang_CapOpts_Clear},
    {"CapOpts_Getc", test_lang_CapOpts_Getc},
    {"CapOpts_Has", test_lang_CapOpts_Has},
    {"CapOpts_ArgsLen", test_lang_CapOpts_ArgsLen},
    {0},
};

/***********
* lang/trv *
***********/

static void
test_lang_blt_mods(void) {
    PadConfig *config = PadConfig_New();
    PadTkrOpt *opt = PadTkrOpt_New();
    PadTkr *tkr = PadTkr_New(PadMem_Move(opt));
    PadAST *ast = PadAST_New(config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);

    // success
    PadTkr_Parse(tkr, "{@ a = alias.set(\"\", \"\") @}{: a :}");
    {
        PadAST_Clear(ast);
        (PadCC_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);

        PadObj *alias_mod = CapBltAliasMod_NewMod(config, gc);
        PadObjDict *varmap = PadCtx_GetVarmap(ctx);
        PadObjDict_Move(varmap, alias_mod->module.name, PadMem_Move(alias_mod));

        PadTrv_Trav(ast, ctx);
        assert(!PadAST_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "nil"));
    }


    PadTkr_Parse(tkr, "{@ a = alias.set(\"\", \"\")\n b = alias.set(\"\", \"\") @}{: a :},{: b :}");
    {
        PadAST_Clear(ast);
        (PadCC_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);
        
        PadObj *alias_mod = CapBltAliasMod_NewMod(config, gc);
        PadObjDict *varmap = PadCtx_GetVarmap(ctx);
        PadObjDict_Move(varmap, alias_mod->module.name, PadMem_Move(alias_mod));

        PadTrv_Trav(ast, ctx);
        assert(!PadAST_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "nil,nil"));
    }

    PadTkr_Parse(tkr, "{@ a = opts.get(\"abc\") @}{: a :}");
    {
        char *argv[] = {
            "make",
            "-abc",
            "def",
            NULL,
        };
        PadAST_Clear(ast);
        (PadCC_Compile(ast, PadTkr_GetToks(tkr)));
        PadCtx_Clear(ctx);

        CapOpts *opts = CapOpts_New();
        assert(CapOpts_Parse(opts, 3, argv));
        CapBltOptsMod_MoveOpts(ctx, opts);

        PadObj *opts_mod = CapBltOptsMod_NewMod(config, gc);
        PadObjDict *varmap = PadCtx_GetVarmap(ctx);
        PadObjDict_Move(varmap, opts_mod->module.name, PadMem_Move(opts_mod));

        PadTrv_Trav(ast, ctx);
        PadAST_MoveOpts(ast, NULL);
        PadAST_TraceErr(ast, stderr);
        assert(!PadAST_HasErrs(ast));
        assert(!strcmp(PadCtx_GetcStdoutBuf(ctx), "def"));
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);
    PadAST_Del(ast);
    PadTkr_Del(tkr);
    PadConfig_Del(config);
}

static const struct testcase
lang_tests[] = {
    {"blt_mods", test_lang_blt_mods},
    {0},
};

/*******
* main *
*******/

static const struct testmodule
test_modules[] = {
    // commands
    {"home", CapHomeCmdests},
    {"cd", cdcmd_tests},
    {"pwd", CapPwdCmdests},
    {"ls", CapLsCmdests},
    {"cat", CapCatCmdests},
    {"make", CapMakeCmdests},
    {"run", CapRunCmdests},
    {"exec", CapExecCmdests},
    {"alias", CapAlCmdests},
    {"edit", CapEditCmdests},
    {"editor", CapEditorCmdests},
    {"mkdir", CapMkdirCmdests},
    {"rm", CapRmCmdests},
    {"mv", CapMvCmdests},
    {"cp", CapCpCmdests},
    {"touch", CapTouchCmdests},
    {"snippet", snippetcmd_tests},
    {"link", CapLinkCmdests},
    {"bake", CapBakeCmdests},
    {"replace", CapReplaceCmdests},

    {"util", utiltests},
    {"symlink", symlink_tests},
    {"opts", opts_tests},

    {"lang", lang_tests},
    {0},
};

struct Opts {
    bool ishelp;
    int32_t argc;
    char **argv;
    int32_t optind;
};

static int32_t
parseopts(struct Opts *opts, int argc, char *argv[]) {
    // Init opts
    *opts = (struct Opts) {0};
    optind = 0;
    opterr = 0;

    // Parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {0},
    };

    for (;;) {
        int optsindex;
        int cur = getopt_long(argc, argv, "h", longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 0: /* Long option only */ break;
        case 'h': opts->ishelp = true; break;
        case '?':
        default: die("unknown option"); break;
        }
    }

    if (argc < optind) {
        die("failed to parse option");
    }

    opts->argc = argc;
    opts->optind = optind;
    opts->argv = argv;

    return 0;
}

static int32_t
modtest(const char *modname) {
    int32_t ntest = 0;
    const struct testmodule *fndmod = NULL;

    for (const struct testmodule *m = test_modules; m->name; ++m) {
        if (strcmp(modname, m->name) == 0) {
            fndmod = m;
        }
    }
    if (!fndmod) {
        return 0;
    }

    printf("\n* module '%s'\n", fndmod->name);

    for (const struct testcase *t = fndmod->tests; t->name; ++t) {
        printf("- testing '%s'\n", t->name);
        t->test();
        ++ntest;
    }

    return ntest;
}

static int32_t
methtest(const char *modname, const char *methname) {
    const struct testmodule *fndmod = NULL;

    for (const struct testmodule *m = test_modules; m->name; ++m) {
        if (strcmp(modname, m->name) == 0) {
            fndmod = m;
        }
    }
    if (!fndmod) {
        return 0;
    }

    printf("\n* module '%s'\n", fndmod->name);

    const struct testcase *fndt = NULL;
    for (const struct testcase *t = fndmod->tests; t->name; ++t) {
        if (!strcmp(t->name, methname)) {
            fndt = t;
            break;
        }
    }
    if (!fndt) {
        return 0;
    }

    printf("* method '%s'\n", fndt->name);
    fndt->test();

    return 1;
}

static int32_t
fulltests(void) {
    int32_t ntest = 0;

    for (const struct testmodule *m = test_modules; m->name; ++m) {
        printf("\n* module '%s'\n", m->name);
        for (const struct testcase *t = m->tests; t->name; ++t) {
            printf("- testing '%s'\n", t->name);
            t->test();
            ++ntest;
        }
    }

    return ntest;
}

static void
run(const struct Opts *opts) {
    int32_t ntest = 0;
    clock_t start;
    clock_t end;

    if (opts->argc - opts->optind == 1) {
        start = clock();
        ntest = modtest(opts->argv[opts->optind]);
        end = clock();
    } else if (opts->argc - opts->optind >= 2) {
        start = clock();
        ntest = methtest(opts->argv[opts->optind], opts->argv[opts->optind+1]);
        end = clock();
    } else {
        start = clock();
        ntest = fulltests();
        end = clock();
    }

    fflush(stdout);
    fprintf(stderr, "\nRun %d test in %0.3lfs.\n", ntest, (double)(end-start)/CLOCKS_PER_SEC);
    fprintf(stderr, "\n");
    fprintf(stderr, "OK\n");

    fflush(stderr);
}

static void
cleanup(void) {
}

int
main(int argc, char *argv[]) {
    setlocale(LC_CTYPE, "");

    struct Opts opts;
    if (parseopts(&opts, argc, argv) != 0) {
        die("failed to parse options");
    }

    run(&opts);
    cleanup();

    return 0;
}
