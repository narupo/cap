#include "replace/replace.h"

/**
 * structure of options
 */
struct opts {
    bool is_help;
};

/**
 * structure of command
 */
struct replace {
    const config_t *config;
    int argc;
    int optind;
    char **argv;
    struct opts opts;
    errstack_t *errstack;
};

/**
 * show usage of command
 *
 * @param[in] self pointer to replacecmd_t
 */
static void
replacecmd_show_usage(replacecmd_t *self) {
    fflush(stdout);
    fflush(stderr);
    fprintf(stderr, "Replace text of file\n"
        "\n"
        "Usage:\n"
        "\n"
        "    cap replace [cap-path] [target] [replaced] [options]...\n"
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
 * @param[in] self pointer to replacecmd_t 
 *
 * @return success to true
 * @return failed to false
 */
static bool
replacecmd_parse_opts(replacecmd_t *self) {
    // parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {0},
    };

    self->opts = (struct opts){0};

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
replacecmd_del(replacecmd_t *self) {
    if (!self) {
        return;
    }

    errstack_del(self->errstack);
    free(self);
}

replacecmd_t *
replacecmd_new(const config_t *config, int argc, char **argv) {
    replacecmd_t *self = mem_ecalloc(1, sizeof(*self));

    self->config = config;
    self->argc = argc;
    self->argv = argv;
    self->errstack = errstack_new();

    if (!replacecmd_parse_opts(self)) {
        replacecmd_del(self);
        return NULL;
    }

    return self;
}

static void
swrite(FILE *fout, const char *text) {
    for (const char *p = text; *p; p += 1) {
        fputc(*p, fout);
    }
}

static int
sreplace(FILE *fout, const char *text, const char *target, const char *replaced) {
    int32_t tarlen = strlen(target);

    for (const char *p = text; *p; p += 1) {
        if (strncmp(p, target, tarlen) == 0) {
            p += tarlen - 1;
            swrite(fout, replaced);
        } else {
            fputc(*p, fout);
        }
    }

    return 0;
}

static int
replace(replacecmd_t *self) {
    if (self->argc < 4) {
        replacecmd_show_usage(self);
        return 0;
    }

    const char *cap_fname = self->argv[optind];
    const char *target_ = self->argv[optind + 1];
    const char *replaced = self->argv[optind + 2];
    assert(cap_fname && target_ && replaced);

    string_t *target = str_new();
    char *content = NULL;
    FILE *fout = NULL;

    char path[FILE_NPATH];
    if (!solve_cmdline_arg_path(self->config, path, sizeof path, cap_fname)) {
        blush("failed to solve path %s", cap_fname);
        goto error;
    }

    content = file_readcp_from_path(path);
    if (content == NULL) {
        blush("failed to read content from %s", path);
        goto error;
    }

    fout = file_open(path, "wb");
    if (fout == NULL) {
        blush("failed to open file %s", path);
        goto error;
    }

    unescape_text(target, target_, NULL);
    int result = sreplace(fout, content, str_getc(target), replaced);

    fclose(fout);
    free(content);
    str_del(target);
    return result;
error:
    str_del(target);
    free(content);
    if (fout) {
        fclose(fout);
    }
    return 1;
}

int
replacecmd_run(replacecmd_t *self) {
    int result = replace(self);
    if (result != 0) {
        errstack_trace_simple(self->errstack, stderr);
    }
    return result;
}
