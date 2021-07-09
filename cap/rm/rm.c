#include <rm/rm.h>

extern int opterr;
extern int optind;

struct Opts {
    bool is_help;
    bool is_recursive;
};

struct CapRmCmd {
    const CapConfig *config;
    int argc;
    int optind;
    CapRmCmdErrno errno_;
    char **argv;
    struct Opts opts;
    char what[1024];
};

static bool
parse_opts(CapRmCmd *self) {
    // parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {"recursive", no_argument, 0, 'r'},
        {0},
    };

    opterr = 0; // ignore error messages
    optind = 0; // init index of parse

    for (;;) {
        int optsindex;
        int cur = getopt_long(self->argc, self->argv, "hr", longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 0: /* long option only */ break;
        case 'h': self->opts.is_help = true; break;
        case 'r': self->opts.is_recursive = true; break;
        case '?':
        default:
            PadCStr_AppFmt(self->what, sizeof self->what, "unknown option.");
            self->errno_ = CAP_RMCMD_ERR__UNKNOWN_OPTS;
            return false;
        }
    }

    if (self->argc < optind) {
        PadCStr_AppFmt(self->what, sizeof self->what, "failed to parse option.");
        self->errno_ = CAP_RMCMD_ERR__PARSE_OPTS;
        return false;
    }

    self->optind = optind;
    return true;
}

void
CapRmCmd_Del(CapRmCmd *self) {
    if (!self) {
        return;
    }
    Pad_SafeFree(self);
}

CapRmCmd *
CapRmCmd_New(const CapConfig *config, int argc, char **argv) {
    CapRmCmd *self = PadMem_Calloc(1, sizeof(*self));
    if (self == NULL) {
        goto error;
    }

    self->config = config;
    self->argc = argc;
    self->argv = argv;

    if (!parse_opts(self)) {
        goto error;
    }

    return self;
error:
    CapRmCmd_Del(self);
    return NULL;
}

static void
usage(CapRmCmd *self) {
    fflush(stdout);
    fflush(stderr);
    fprintf(stderr, "Remove files or directories from environment.\n"
        "\n"
        "Usage:\n"
        "\n"
        "   cap rm [path]... [options]...\n"
        "\n"
        "The options are:\n"
        "\n"
        "   -h, --help         show usage\n"
        "   -r, --recursive    remove directories and their contents recursively\n"
        "\n"
    );
    fflush(stderr);
    exit(0);
}

CapRmCmdErrno
CapRmCmd_Errno(const CapRmCmd *self) {
    return self->errno_;
}

const char *
CapRmCmd_What(const CapRmCmd *self) {
    return self->what;
}

static bool
remove_re(CapRmCmd *self, const char *dirpath) {
    if (Cap_IsOutOfHome(self->config->home_path, dirpath)) {
        PadCStr_AppFmt(self->what, sizeof self->what, "\"%s\" is out of home.", dirpath);
        self->errno_ = CAP_RMCMD_ERR__OUTOFHOME;
        return false;
    }

    if (!PadFile_IsDir(dirpath)) {
        PadCStr_AppFmt(self->what, sizeof self->what, "\"%s\" is not a directory.", dirpath);
        self->errno_ = CAP_RMCMD_ERR__OPENDIR;
        return false;
    }

    file_dir_t *dir = PadDir_Open(dirpath);
    if (!dir) {
        PadCStr_AppFmt(self->what, sizeof self->what, "failed to open directory \"%s\".", dirpath);
        self->errno_ = CAP_RMCMD_ERR__OPENDIR;
        return false;
    }

    for (PadDirNode *node; (node = PadDir_Read(dir)); ) {
        const char *dirname = PadDirNode_Name(node);
        if (!strcmp(dirname, ".") || !strcmp(dirname, "..")) {
            continue;
        }

        char path[FILE_NPATH];
        if (!PadFile_Solvefmt(path, sizeof path, "%s/%s", dirpath, dirname)) {
            PadCStr_AppFmt(self->what, sizeof self->what, "failed to solve path by \"%s\".", dirname);
            self->errno_ = CAP_RMCMD_ERR__SOLVEPATH;
        }

        if (Cap_IsOutOfHome(self->config->home_path, path)) {
            PadCStr_AppFmt(self->what, sizeof self->what, "\"%s\" is out of home.", path);
            self->errno_ = CAP_RMCMD_ERR__OUTOFHOME;
            return false;
        }

        if (PadFile_IsDir(path)) {
            if (!remove_re(self, path)) {
                return false;
            }
            // directory is empty, remove directory
            if (PadFile_Remove(path) != 0) {
                PadCStr_AppFmt(self->what, sizeof self->what, "failed to remove directory \"%s\".", path);
                self->errno_ = CAP_RMCMD_ERR__REMOVE_FILE;
                return false;
            }
        } else {
            if (PadFile_Remove(path) != 0) {
                PadCStr_AppFmt(self->what, sizeof self->what, "failed to remove file \"%s\".", path);
                self->errno_ = CAP_RMCMD_ERR__REMOVE_FILE;
                return false;
            }
        }
    }

    if (PadDir_Close(dir) != 0) {
        PadCStr_AppFmt(self->what, sizeof self->what, "failed to close directory \"%s\".", dirpath);
        self->errno_ = CAP_RMCMD_ERR__CLOSEDIR;
        return false;
    }

    return true;
}

static int
_rm_re(CapRmCmd *self) {
    for (int i = self->optind; i < self->argc; ++i) {
        const char *argpath = self->argv[i];
        if (!strcmp(argpath, ".") || !strcmp(argpath, "..")) {
            PadCStr_AppFmt(self->what, sizeof self->what, "refusing to remove '.' or '..'");
            self->errno_ = CAP_RMCMD_ERR__REMOVE_FILE;
            return 1;
        }

        const char *org = Cap_GetOrigin(self->config, argpath);
        char path[FILE_NPATH];
        char drtpath[FILE_NPATH];

        snprintf(drtpath, sizeof drtpath, "%s/%s", org, argpath);

        if (!CapSymlink_FollowPath(self->config, path, sizeof path, drtpath)) {
            PadCStr_AppFmt(self->what, sizeof self->what, "failed to solve path from \"%s\".", argpath);
            self->errno_ = CAP_RMCMD_ERR__SOLVEPATH;
            return 1;
        }

        if (Cap_IsOutOfHome(self->config->home_path, path)) {
            PadCStr_AppFmt(self->what, sizeof self->what, "\"%s\" is out of home.", path);
            self->errno_ = CAP_RMCMD_ERR__OUTOFHOME;
            return 1;
        }

        if (!remove_re(self, path)) {
            PadCStr_AppFmt(self->what, sizeof self->what, "could not delete recusively.");
            return 1;
        }

        if (PadFile_Remove(path) != 0) {
            PadCStr_AppFmt(self->what, sizeof self->what, "failed to remove directory \"%s\".", path);
            self->errno_ = CAP_RMCMD_ERR__REMOVE_FILE;
            return false;
        }
    }

    return 0;
}

static int
_rm(CapRmCmd *self, const char *argpath) {
    char path[FILE_NPATH];
    const char *org = Cap_GetOrigin(self->config, argpath);

    char drtpath[FILE_NPATH];
    snprintf(drtpath, sizeof drtpath, "%s/%s", org, argpath);

    if (!CapSymlink_FollowPath(self->config, path, sizeof path, drtpath)) {
        PadCStr_AppFmt(self->what, sizeof self->what, "failed to solve path.");
        self->errno_ = CAP_RMCMD_ERR__SOLVEPATH;
        return 1;
    }

    if (Cap_IsOutOfHome(self->config->home_path, path)) {
        PadCStr_AppFmt(self->what, sizeof self->what, "\"%s\" is out of home.", path);
        self->errno_ = CAP_RMCMD_ERR__OUTOFHOME;
        return 1;
    }

    if (PadFile_Remove(path) != 0) {
        PadCStr_AppFmt(self->what, sizeof self->what, "failed to remove \"%s\".", path);
        self->errno_ = CAP_RMCMD_ERR__REMOVE_FILE;
        return 1;
    }

    return 0;
}

static int
_rm_all(CapRmCmd *self) {
    int ret = 0;

    for (int i = self->optind; i < self->argc; ++i) {
        const char *argpath = self->argv[i];
        ret += _rm(self, argpath);
    }

    return ret;
}

int
CapRmCmd_Run(CapRmCmd *self) {
    if (self->argc < self->optind+1) {
        usage(self);
    }

    if (self->opts.is_help) {
        usage(self);
    }

    if (self->opts.is_recursive) {
        return _rm_re(self);
    }

    return _rm_all(self);
}
