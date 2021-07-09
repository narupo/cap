#include <mkdir/mkdir.h>

extern int opterr;
extern int optind;

struct Opts {
    bool is_help;
    bool is_parents;
};

struct CapMkdirCmd {
    const CapConfig *config;
    int argc;
    char **argv;
    struct Opts opts;
    int optind;
};

void
parse_opts(CapMkdirCmd *self) {
    // parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {"parents", no_argument, 0, 'p'},
        {0},
    };

    opterr = 0; // ignore error messages
    optind = 0; // init index of parse
    self->opts = (struct Opts){0};

    for (;;) {
        int optsindex;
        int cur = getopt_long(self->argc, self->argv, "hp", longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 0: /* long option only */ break;
        case 'h': self->opts.is_help = true; break;
        case 'p': self->opts.is_parents = true; break;
        case '?':
        default:
            PadErr_Die("unsupported option");
            break;
        }
    }

    if (self->argc < optind) {
        PadErr_Die("Failed to parse option");
    }

    self->optind = optind;
}

void
CapMkdirCmd_Del(CapMkdirCmd *self) {
    if (!self) {
        return;
    }
    Pad_SafeFree(self);
}

CapMkdirCmd *
CapMkdirCmd_New(const CapConfig *config, int argc, char **argv) {
    CapMkdirCmd *self = PadMem_Calloc(1, sizeof(*self));
    if (self == NULL) {
        return NULL;
    }

    self->config = config;
    self->argc = argc;
    self->argv = argv;

    parse_opts(self);

    return self;
}

void
usage(CapMkdirCmd *self) {
    fflush(stdout);
    fflush(stderr);
    fprintf(stderr, "Usage:\n"
        "\n"
        "    cap mkdir [path] [options...]\n"
        "\n"
        "The options are:\n"
        "\n"
        "    -h, --help       show usage\n"
        "    -p, --parents    not error if existing, make parent directories as needed\n"
        "\n"
    );
    fflush(stderr);
    exit(0);
}

static int
_mkdirp(CapMkdirCmd *self) {
    const char *argpath = self->argv[self->optind];
    char path[PAD_FILE__NPATH];

    if (argpath[0] == ':') {
        if (!PadFile_Solve(path, sizeof path, argpath+1)) {
            PadErr_Err("failed to solve path");
            return 1;
        }
    } else {
        const char* org = Cap_GetOrigin(self->config, argpath);
        char tmppath[PAD_FILE__NPATH];

        snprintf(tmppath, sizeof tmppath, "%s/%s", org, argpath);

        if (!CapSymlink_FollowPath(self->config, path, sizeof path, tmppath)) {
            PadErr_Err("failed to follow path");
            return 1;
        }
    }

    if (PadFile_MkdirsQ(path) != 0) {
        PadErr_Err("failed to create directory \"%s\"", path);
        return 1;
    }

    return 0;
}

static int
_mkdir(CapMkdirCmd *self) {
    const char *argpath = self->argv[self->optind];
    char path[PAD_FILE__NPATH];

    if (argpath[0] == ':') {
        if (!PadFile_Solve(path, sizeof path, argpath+1)) {
            PadErr_Err("failed to solve path");
            return 1;
        }
    } else {
        const char *org = Cap_GetOrigin(self->config, argpath);
        char tmppath[PAD_FILE__NPATH];

        snprintf(tmppath, sizeof tmppath, "%s/%s", org, argpath);

        if (!CapSymlink_FollowPath(self->config, path, sizeof path, tmppath)) {
            PadErr_Err("failed to follow path");
            return 1;
        }
    }

    if (PadFile_IsExists(path)) {
        PadErr_Err("failed to create directory. \"%s\" is exists", path);
        return 1;
    }

    if (PadFile_MkdirQ(path) != 0) {
        PadErr_Err("failed to create directory \"%s\"", path);
        return 1;
    }

    return 0;
}

int
CapMkdirCmd_Run(CapMkdirCmd *self) {
    if (self->opts.is_help) {
        usage(self);
    }

    if (self->argc < self->optind+1) {
        usage(self);
    }

    if (self->opts.is_parents) {
        return _mkdirp(self);
    } else {
        return _mkdir(self);
    }

    return 0;  // impossible
}
