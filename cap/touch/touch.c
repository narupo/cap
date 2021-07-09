#include <cap/touch/touch.h>

/**
 * Structure of options
 */
struct Opts {
    bool is_help;
};

/**
 * Structure of command
 */
struct CapTouchCmd {
    const CapConfig *config;
    int argc;
    int optind;
    char **argv;
    struct Opts opts;
};

/**
 * Show usage of command
 *
 * @param[in] self pointer to CapTouchCmd
 */
static void
usage(CapTouchCmd *self) {
    fflush(stdout);
    fflush(stderr);
    fprintf(stderr, "Usage:\n"
        "\n"
        "    cap touch [options]... [file]...\n"
        "\n"
        "The options are:\n"
        "\n"
        "    -h, --help    show usage\n"
        "\n"
    );
    fflush(stderr);
    exit(0);
}

/**
 * Parse options
 *
 * @param[in] self pointer to CapTouchCmd
 *
 * @return success to true
 * @return failed to false
 */
static bool
parse_opts(CapTouchCmd *self) {
    // parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {0},
    };

    self->opts = (struct Opts){0};

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
CapTouchCmd_Del(CapTouchCmd *self) {
    if (!self) {
        return;
    }

    free(self);
}

CapTouchCmd *
CapTouchCmd_New(const CapConfig *config, int argc, char **argv) {
    CapTouchCmd *self = PadMem_Calloc(1, sizeof(*self));
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
    CapTouchCmd_Del(self);
    return NULL;
}

static int
_touch(CapTouchCmd *self, const char *argpath) {
    char path[FILE_NPATH];
    char tmppath[FILE_NPATH];
    const char *org = Cap_GetOrigin(self->config, argpath);

    snprintf(tmppath, sizeof tmppath, "%s/%s", org, argpath);
    if (!CapSymlink_FollowPath(self->config, path, sizeof path, tmppath)) {
        PadErr_Err("failed to solve path by \"%s\"", argpath);
        return 1;
    }

    if (PadFile_IsExists(path)) {
        return 0;  // do not truncate file
    }

    if (!PadFile_Trunc(path)) {
        PadErr_Err("failed to truncate file");
        return 1;
    }

    return 0;
}

static int
_all_touch(CapTouchCmd *self) {
    for (int i = self->optind; i < self->argc; ++i) {
        const char *argpath = self->argv[i];
        _touch(self, argpath);
    }

    return 0;
}

int
CapTouchCmd_Run(CapTouchCmd *self) {
    if (self->argc < self->optind+1) {
        usage(self);
    }

    if (self->opts.is_help) {
        usage(self);
    }

    return _all_touch(self);
}
