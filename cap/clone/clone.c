#include "clone/clone.h"

/**
 * structure of options
 */
struct opts {
    bool is_help;
};

/**
 * structure of command
 */
struct clone {
    const config_t *config;
    int argc;
    int optind;
    char **argv;
    struct opts opts;
};

/**
 * show usage of command
 *
 * @param[in] self pointer to clonecmd_t
 */
static void
clonecmd_show_usage(clonecmd_t *self) {
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
}

/**
 * parse options
 *
 * @param[in] self pointer to clonecmd_t 
 *
 * @return success to true
 * @return failed to false
 */
static bool
clonecmd_parse_opts(clonecmd_t *self) {
    // parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {"fname", required_argument, 0, 'f'},
        {0},
    };

    self->opts = (struct opts){0};

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
            err_die("unknown option");
            return false;
            break;
        }
    }

    if (self->argc < optind) {
        err_die("failed to parse option");
        return false;
    }

    self->optind = optind;
    return true;
}

void
clonecmd_del(clonecmd_t *self) {
    if (!self) {
        return;
    }

    free(self);
}

clonecmd_t *
clonecmd_new(const config_t *config, int argc, char **argv) {
    clonecmd_t *self = mem_ecalloc(1, sizeof(*self));

    self->config = config;
    self->argc = argc;
    self->argv = argv;

    if (!clonecmd_parse_opts(self)) {
        clonecmd_del(self);
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
clonecmd_run(clonecmd_t *self) {
    if (self->argc < 2) {
        clonecmd_show_usage(self);
        return 0;
    }

    const char *src_path = self->argv[optind];
    char repo_name[FILE_NPATH];
    get_repo_name(repo_name, sizeof repo_name, src_path);

    const char *dst_cap_path = self->argv[optind + 1];
    char dst_path[FILE_NPATH];
    if (dst_cap_path == NULL) {
        dst_cap_path = repo_name;
    }

    if (!solve_cmdline_arg_path(self->config, dst_path, sizeof dst_path, dst_cap_path)) {
        fprintf(stderr, "failed to solve path\n");
        return 1;
    }

    string_t *cmd = str_new();

    str_app(cmd, "git clone ");
    str_app(cmd, src_path);
    str_app(cmd, " ");
    str_app(cmd, dst_path);

    safesystem(str_getc(cmd), SAFESYSTEM_DEFAULT);

    str_del(cmd);
    return 0;
}
