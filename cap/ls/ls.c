#include <ls/ls.h>

struct Opts {
    bool is_help;
    bool is_all;
};

struct CapLsCmd {
    const CapConfig *config;
    int argc;
    char **argv;
    struct Opts opts;
};

void
CapLsCmd_Del(CapLsCmd *self) {
    if (self) {
        return;
    }
    free(self);
}

static bool
parse_opts(CapLsCmd *self) {
    // Parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {"all", no_argument, 0, 'a'},
        {0},
    };

    self->opts = (struct Opts){0};
    opterr = 0; // ignore error messages
    optind = 0; // init index of parse

    for (;;) {
        int optsindex;
        int cur = getopt_long(self->argc, self->argv, "ha", longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 'h': self->opts.is_help = true; break;
        case 'a': self->opts.is_all = true; break;
        case '?':
        default:
            PadErr_Err("unknown option");
            return false;
            break;
        }
    }

    if (self->argc < optind) {
        return false;
    }

    return true;
}

static void
lscmd_usage(const CapLsCmd *self) {
    fprintf(stderr,
        "Usage:\n"
        "\n"
        "    cap ls [options]\n"
        "\n"
        "The options are:\n"
        "\n"
        "    -h, --help    show usage.\n"
        "    -a, --all     show all files.\n"
        "\n"
    );
}

CapLsCmd *
CapLsCmd_New(const CapConfig *config, int argc, char **argv) {
    CapLsCmd *self = PadMem_ECalloc(1, sizeof(*self));

    self->config = config;
    self->argc = argc;
    self->argv = argv;

    if (!parse_opts(self)) {
        CapLsCmd_Del(self);
        return NULL;
    }

    return self;
}

static void
print_fname(const CapLsCmd *self, FILE *fout, bool print_color, const char *path, const char *name) {
    if (!print_color) {
        fprintf(fout, "%s\n", name);
        return;
    }

    char fpath[FILE_NPATH];
    if (!PadFile_Solvefmt(fpath, sizeof fpath, "%s/%s", path, name)) {
        PadErr_Err("failed to solve path by name \"%s\"", name);
        return;
    }

    if (PadFile_IsDir(fpath)) {
        PadTerm_CFPrintf(fout, TERM_WHITE, TERM_GREEN, TERM_BRIGHT, "%s", name);
    } else if (CapSymlink_IsLinkFile(fpath)) {
        PadTerm_CFPrintf(fout, TERM_CYAN, TERM_BLACK, TERM_BRIGHT, "%s", name);
    } else {
        PadTerm_CFPrintf(fout, TERM_GREEN, TERM_BLACK, TERM_BRIGHT, "%s", name);
    }
    fputc('\n', fout);
}

static void
dump_ary(const CapLsCmd *self, FILE *fout, const char *path, const PadCStrAry *arr) {
    bool print_color = isatty(PadFile_GetNum(fout));

    for (int i = 0; i < PadCStrAry_Len(arr); ++i) {
        const char *name = PadCStrAry_Getc(arr, i);
        print_fname(self, fout, print_color, path, name);
    }

    fflush(fout);
}

static bool
is_dot_file(const CapLsCmd *_, const char *fname) {
    if (strcmp(fname, "..") == 0 ||
        fname[0] == '.') {
        return true;
    }

    return false;
}

static PadCStrAry *
dir_to_ary(const CapLsCmd *self, file_dir_t *dir) {
    PadCStrAry *ary = PadCStrAry_New();
    if (!ary) {
        return NULL;
    }

    for (PadDirNode *nd; (nd = PadDir_Read(dir)); ) {
        const char *name = PadDirNode_Name(nd);
        if (is_dot_file(self, name) && !self->opts.is_all) {
            continue;
        }
        PadCStrAry_PushBack(ary, name);
        PadDirNode_Del(nd);
    }

    return ary;
}

static int
_ls(const CapLsCmd *self, const char *path) {
    if (Cap_IsOutOfHome(self->config->home_path, path)) {
        PadErr_Err("\"%s\" is out of home", path);
        return 1;
    }

    file_dir_t *dir = PadDir_Open(path);
    if (!dir) {
        PadErr_Err("failed to open directory \"%s\"", path);
        return 2;
    }

    PadCStrAry *arr = dir_to_ary(self, dir);
    if (!arr) {
        PadErr_Err("failed to read directory \"%s\"", path);
        return 3;
    }

    PadCStrAry_Sort(arr);
    dump_ary(self, stdout, path, arr);
    PadCStrAry_Del(arr);

    if (PadDir_Close(dir) < 0) {
        PadErr_Err("failed to close directory \"%s\"", path);
        return 4;
    }

    return 0;
}

int
CapLsCmd_Run(CapLsCmd *self) {
    if (self->opts.is_help) {
        lscmd_usage(self);
        return 0;
    }

    char realpath[FILE_NPATH];

    if (optind - self->argc == 0) {
        if (!CapSymlink_FollowPath(self->config, realpath, sizeof realpath, self->config->cd_path)) {
            PadErr_Err("failed to follow path");
            return 1;
        }
        return _ls(self, realpath);
    } else {
        for (int i = optind; i < self->argc; ++i) {
            const char *arg = self->argv[i];
            const char *org = (arg[0] == '/' ? self->config->home_path : self->config->cd_path);
            if (!strcmp(arg, "/")) {
                char tmppath[FILE_NPATH];
                snprintf(tmppath, sizeof tmppath, "%s", org);
                if (!CapSymlink_FollowPath(self->config, realpath, sizeof realpath, tmppath)) {
                    continue;
                }
            } else {
                char tmppath[FILE_NPATH*2];
                snprintf(tmppath, sizeof tmppath, "%s/%s", org, arg);
                if (!CapSymlink_FollowPath(self->config, realpath, sizeof realpath, tmppath)) {
                    continue;
                }
            }
            _ls(self, realpath);
        }
    }

    return 0;
}
