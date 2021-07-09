#include <cap/clone/clone.h>

/**
 * structure of options
 */
struct Opts {
    bool is_help;
};

/**
 * structure of command
 */
struct CapCloneCmd {
    const CapConfig *config;
    int argc;
    int optind;
    char **argv;
    struct Opts opts;
};

/**
 * show usage of command
 *
 * @param[in] self pointer to CapCloneCmd
 */
static int
usage(CapCloneCmd *self) {
    fflush(stdout);
    fflush(stderr);
    fprintf(stderr, "Usage:\n"
        "\n"
        "    cap clone [url|path] [dst-cap-path] [options]...\n"
        "\n"
        "The options are:\n"
        "\n"
        "    -h, --help    Show usage\n"
        "\n"
    );
    fflush(stderr);
    return 0;
}

/**
 * parse options
 *
 * @param[in] self pointer to CapCloneCmd 
 *
 * @return success to true
 * @return failed to false
 */
static bool
parse_opts(CapCloneCmd *self) {
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
CapCloneCmd_Del(CapCloneCmd *self) {
    if (!self) {
        return;
    }

    free(self);
}

CapCloneCmd *
CapCloneCmd_New(const CapConfig *config, int argc, char **argv) {
    CapCloneCmd *self = PadMem_Calloc(1, sizeof(*self));
    if (self == NULL) {
        return NULL;
    }

    self->config = config;
    self->argc = argc;
    self->argv = argv;

    if (!parse_opts(self)) {
        CapCloneCmd_Del(self);
        return NULL;
    }

    return self;
}

static void get_repo_name(char *dst, int32_t dstsz, const char *path) {
    int32_t len = strlen(path);
    const char *p = path + len - 1;

    for (; p >= path; p -= 1) {
        if (*p == '/' || *p == '\\' || *p == ':') {
            p += 1;
            break;
        }
    }

    char *dp = dst;
    char *dend = dst + dstsz - 1;

    for (; dp < dend; dp += 1, p += 1) {
        *dp = *p;
    }

    *dp = '\0';
}

int
CapCloneCmd_Run(CapCloneCmd *self) {
    if (self->argc < 2) {
        return usage(self);
    }

    const char *src_path = self->argv[optind];
    char repo_name[PAD_FILE__NPATH];
    get_repo_name(repo_name, sizeof repo_name, src_path);

    const char *dst_cap_path = self->argv[optind + 1];
    char dst_path[PAD_FILE__NPATH];
    if (dst_cap_path == NULL) {
        dst_cap_path = repo_name;
    }

    if (!Cap_SolveCmdlineArgPath(self->config, dst_path, sizeof dst_path, dst_cap_path)) {
        fprintf(stderr, "failed to solve path\n");
        return 1;
    }

    PadStr *cmd = PadStr_New();

    PadStr_App(cmd, "git clone ");
    PadStr_App(cmd, src_path);
    PadStr_App(cmd, " ");
    PadStr_App(cmd, dst_path);

    Pad_SafeSystem(PadStr_Getc(cmd), PAD_SAFESYSTEM__DEFAULT);

    PadStr_Del(cmd);
    return 0;
}
