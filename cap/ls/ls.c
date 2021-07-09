#include <ls/ls.h>

struct Opts {
    bool is_help;
    bool is_all;
};

struct lscmd {
    const CapConfig *config;
    int argc;
    char **argv;
    struct Opts opts;
};

void
lscmd_del(lscmd_t *self) {
    if (self) {
        return;
    }

    free(self);
}

static bool
lscmd_parse_opts(lscmd_t *self) {
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
lscmd_usage(const lscmd_t *self) {
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

lscmd_t *
lscmd_new(const CapConfig *config, int argc, char **argv) {
    lscmd_t *self = PadMem_ECalloc(1, sizeof(*self));

    self->config = config;
    self->argc = argc;
    self->argv = argv;

    if (!lscmd_parse_opts(self)) {
        lscmd_del(self);
        return NULL;
    }

    return self;
}

static void
print_fname(const lscmd_t *self, FILE *fout, bool print_color, const char *path, const char *name) {
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
        term_cfprintf(fout, TERM_WHITE, TERM_GREEN, TERM_BRIGHT, "%s", name);
    } else if (CapSymlink_IsLinkFile(fpath)) {
        term_cfprintf(fout, TERM_CYAN, TERM_BLACK, TERM_BRIGHT, "%s", name);
    } else {
        term_cfprintf(fout, TERM_GREEN, TERM_BLACK, TERM_BRIGHT, "%s", name);
    }
    fputc('\n', fout);
}

static void
lscmd_arrdump(const lscmd_t *self, FILE *fout, const char *path, const cstring_array_t *arr) {
    bool print_color = isatty(file_get_no(fout));

    for (int i = 0; i < cstrarr_len(arr); ++i) {
        const char *name = cstrarr_getc(arr, i);
        print_fname(self, fout, print_color, path, name);
    }

    fflush(fout);
}

static bool
lscmd_isdotfile(const lscmd_t *_, const char *fname) {
    if (strcmp(fname, "..") == 0 ||
        fname[0] == '.') {
        return true;
    }

    return false;
}

static cstring_array_t *
lscmd_dir2arr(const lscmd_t *self, file_dir_t *dir) {
    cstring_array_t *arr = cstrarr_new();
    if (!arr) {
        return NULL;
    }

    for (PadDirNode *nd; (nd = PadDir_Read(dir)); ) {
        const char *name = PadDirNode_Name(nd);
        if (lscmd_isdotfile(self, name) && !self->opts.is_all) {
            continue;
        }
        cstrarr_push(arr, name);
        PadDirNode_Del(nd);
    }

    return arr;
}

static int
lscmd_ls(const lscmd_t *self, const char *path) {
    if (Cap_IsOutOfHome(self->config->home_path, path)) {
        PadErr_Err("\"%s\" is out of home", path);
        return 1;
    }

    file_dir_t *dir = PadDir_Open(path);
    if (!dir) {
        PadErr_Err("failed to open directory \"%s\"", path);
        return 2;
    }

    cstring_array_t *arr = lscmd_dir2arr(self, dir);
    if (!arr) {
        PadErr_Err("failed to read directory \"%s\"", path);
        return 3;
    }

    cstrarr_sort(arr);
    lscmd_arrdump(self, stdout, path, arr);
    cstrarr_del(arr);

    if (PadDir_Close(dir) < 0) {
        PadErr_Err("failed to close directory \"%s\"", path);
        return 4;
    }

    return 0;
}

int
lscmd_run(lscmd_t *self) {
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
        return lscmd_ls(self, realpath);
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
            lscmd_ls(self, realpath);
        }
    }

    return 0;
}