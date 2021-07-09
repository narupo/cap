#include <mkdir/mkdir.h>

extern int opterr;
extern int optind;

struct Opts {
    bool is_help;
    bool is_parents;
};

struct mkdircmd {
    const CapConfig *config;
    int argc;
    char **argv;
    struct Opts opts;
    int optind;
};

void
mkdircmd_parse_opts(mkdircmd_t *self) {
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
mkdircmd_del(mkdircmd_t *self) {
    if (!self) {
        return;
    }

    free(self);
}

mkdircmd_t *
mkdircmd_new(const CapConfig *config, int argc, char **argv) {
    mkdircmd_t *self = PadMem_ECalloc(1, sizeof(*self));

    self->config = config;
    self->argc = argc;
    self->argv = argv;

    mkdircmd_parse_opts(self);

    return self;
}

void
mkdircmd_show_usage(mkdircmd_t *self) {
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
}

int
mkdircmd_mkdirp(mkdircmd_t *self) {
    const char *argpath = self->argv[self->optind];
    char path[FILE_NPATH];

    if (argpath[0] == ':') {
        if (!PadFile_Solve(path, sizeof path, argpath+1)) {
            PadErr_Err("failed to solve path");
            return 1;
        }
    } else {
        const char* org = get_origin(self->config, argpath);
        char tmppath[FILE_NPATH];

        snprintf(tmppath, sizeof tmppath, "%s/%s", org, argpath);

        if (!CapSymlink_FollowPath(self->config, path, sizeof path, tmppath)) {
            PadErr_Err("failed to follow path");
            return 1;
        }
    }

    if (file_mkdirsq(path) != 0) {
        PadErr_Err("failed to create directory \"%s\"", path);
        return 1;
    }

    return 0;
}

int
mkdircmd_mkdir(mkdircmd_t *self) {
    const char *argpath = self->argv[self->optind];
    char path[FILE_NPATH];

    if (argpath[0] == ':') {
        if (!PadFile_Solve(path, sizeof path, argpath+1)) {
            PadErr_Err("failed to solve path");
            return 1;
        }
    } else {
        const char *org = get_origin(self->config, argpath);
        char tmppath[FILE_NPATH];

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
mkdircmd_run(mkdircmd_t *self) {
    if (self->opts.is_help) {
        mkdircmd_show_usage(self);
        return 0;        
    }

    if (self->argc < self->optind+1) {
        mkdircmd_show_usage(self);
        return 0;
    }

    if (self->opts.is_parents) {
        return mkdircmd_mkdirp(self);
    } else {
        return mkdircmd_mkdir(self);
    }

    return 0; // impossible
}
