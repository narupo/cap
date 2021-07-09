#include <cap/mv/mv.h>

struct Opts {
    bool is_help;
};

struct CapMvCmd {
    const CapConfig *config;
    int argc;
    char **argv;
    int optind;
    struct Opts opts;
};

static void
parse_opts(CapMvCmd *self) {
    // parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {0},
    };

    extern int opterr;
    extern int optind;
    opterr = 0; // ignore error messages
    optind = 0; // init index of parse

    for (;;) {
        int optsindex;
        int cur = getopt_long(self->argc, self->argv, "h", longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 0: /* long option only */ break;
        case 'h': self->opts.is_help = true; break;
        case '?':
        default: PadErr_Die("Unknown option"); break;
        }
    }

    if (self->argc < optind) {
        PadErr_Die("Failed to parse option");
    }

    self->optind = optind;
}

void
CapMvCmd_Del(CapMvCmd *self) {
    if (!self) {
        return;
    }
    Pad_SafeFree(self);
}

CapMvCmd *
CapMvCmd_New(CapConfig *config, int argc, char **argv) {
    CapMvCmd *self = PadMem_Calloc(1, sizeof(*self));
    if (self == NULL) {
        return NULL;
    }

    self->config = config;
    self->argc = argc;
    self->argv = argv;

    parse_opts(self);

    return self;
}

static void
usage(CapMvCmd *self) {
    fflush(stdout);
    fflush(stderr);
    fprintf(stderr, "Usage:\n"
        "\n"
        "    cap mv [old file] [new file] [options...]\n"
        "    cap mv [file] [dst dir] [options...]\n"
        "    cap mv [file] [file] [dst dir] [options...]\n"
        "\n"
        "The options are:\n"
        "\n"
        "    -h, --help    show usage\n"
        "\n"
    );
    fflush(stderr);
    exit(0);
}

static bool
mv_file_to_dir(CapMvCmd *self, const char *cap_path, const char *dirname) {
    char srcpath[PAD_FILE__NPATH];
    char dstpath[PAD_FILE__NPATH];
    char tmppath[PAD_FILE__NPATH*3];

    if (!Cap_SolveCmdlineArgPath(self->config, srcpath, sizeof srcpath, cap_path)) {
        PadErr_Err("failed to solve path for source file name");
        return false;
    }

    if (!PadFile_IsExists(srcpath)) {
        PadErr_Err("\"%s\" is not exists", cap_path);
        return false;
    }

    char basename[PAD_FILE__NPATH];
    if (!PadFile_BaseName(basename, sizeof basename, cap_path)) {
        PadErr_Err("failed to get basename from file name");
        return false;
    }

    snprintf(tmppath, sizeof tmppath, "%s/%s", dirname, basename);
    if (!Cap_SolveCmdlineArgPath(self->config, dstpath, sizeof dstpath, tmppath)) {
        PadErr_Err("failed to solve path for destination file name");
        return false;
    }

    if (PadFile_Rename(srcpath, dstpath) != 0) {
        PadErr_Err("failed to rename file \"%s\" to \"%s\" directory", srcpath, dstpath);
        return false;
    }

    return true;
}

static int
mv_files_to_dir(CapMvCmd *self) {
    const char *lastfname = self->argv[self->argc-1];

    for (int i = self->optind; i < self->argc-1; ++i) {
        const char *fname = self->argv[i];
        if (!mv_file_to_dir(self, fname, lastfname)) {
            PadErr_Err("failed to move file %s to %s", fname, lastfname);
            return 1;
        }
    }

    return 0;
}

static int
mv_file_to_other(CapMvCmd *self) {
    const char *src_cap_path = self->argv[self->optind];
    const char *dst_cap_path = self->argv[self->optind + 1];

    char srcpath[PAD_FILE__NPATH];

    if (!Cap_SolveCmdlineArgPath(self->config, srcpath, sizeof srcpath, src_cap_path)) {
        PadErr_Err("failed to follow path for source file name");
        return 1;
    }

    if (!PadFile_IsExists(srcpath)) {
        PadErr_Err("\"%s\" is not exists. can not move to other", src_cap_path);
        return 1;
    }

    char dstpath[PAD_FILE__NPATH * 2];

    if (!Cap_SolveCmdlineArgPath(self->config, dstpath, sizeof dstpath, dst_cap_path)) {
        PadErr_Err("failed to solve path for destination file name");
        return 1;
    }

    // remove last separate for stat
    const int dstpathlen = strlen(dstpath);
    if (dstpath[dstpathlen - 1] == PAD_FILE__SEP) {
        dstpath[dstpathlen - 1] = '\0';
    }

    // if dst path is directory then switch to process of directory
    if (PadFile_IsDir(dstpath)) {
        char basename[PAD_FILE__NPATH];

        if (src_cap_path[0] == ':') {
            src_cap_path += 1;
        }
        if (!PadFile_BaseName(basename, sizeof basename, src_cap_path)) {
            PadErr_Err("failed to get basename in file to other");
            return 1;
        }

        char dstpath2[PAD_FILE__NPATH * 3];
        char tmppath[PAD_FILE__NPATH * 3];
        snprintf(tmppath, sizeof tmppath, "%s/%s", dstpath, basename);
        if (!PadFile_Solve(dstpath2, sizeof dstpath2, tmppath)) {
            PadErr_Err("failed to follow path for second destination path in file to other");
            return 1;
        }

        if (PadFile_Rename(srcpath, dstpath2) != 0) {
            PadErr_Err("failed to rename \"%s\" to \"%s\"", srcpath, dstpath2);
            return 1;
        }
    } else {
        if (PadFile_Rename(srcpath, dstpath) != 0) {
            PadErr_Err("failed to rename \"%s\" to \"%s\" (2)", srcpath, dstpath);
            return 1;
        }
    }

    return 0;
}

static int
_mv(CapMvCmd *self) {
    const int nargs = self->argc-self->optind;
    if (nargs >= 3) {
        return mv_files_to_dir(self);
    } else if (nargs == 2) {
        return mv_file_to_other(self);
    } else {
        PadErr_Err("not found destination");
        return 1;
    }
}

int
CapMvCmd_Run(CapMvCmd *self) {
    if (self->argc < self->optind+2) {
        usage(self);
    }

    if (self->opts.is_help) {
        usage(self);
    }

    return _mv(self);
}
