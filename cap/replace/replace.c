#include <cap/replace/replace.h>

/**
 * structure of options
 */
struct Opts {
    bool is_help;
};

/**
 * structure of command
 */
struct CapReplaceCmd {
    const CapConfig *config;
    int argc;
    int optind;
    char **argv;
    struct Opts opts;
    PadErrStack *errstack;
};

/**
 * show usage of command
 *
 * @param[in] self pointer to CapReplaceCmd
 */
static void
usage(CapReplaceCmd *self) {
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
    exit(0);
}

/**
 * parse options
 *
 * @param[in] self pointer to CapReplaceCmd 
 *
 * @return success to true
 * @return failed to false
 */
static bool
parse_opts(CapReplaceCmd *self) {
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
CapReplaceCmd_Del(CapReplaceCmd *self) {
    if (!self) {
        return;
    }
    PadErrStack_Del(self->errstack);
    Pad_SafeFree(self);
}

CapReplaceCmd *
CapReplaceCmd_New(const CapConfig *config, int argc, char **argv) {
    CapReplaceCmd *self = PadMem_Calloc(1, sizeof(*self));
    if (self == NULL) {
        goto error;
    }

    self->config = config;
    self->argc = argc;
    self->argv = argv;
    self->errstack = PadErrStack_New();
    if (self->errstack == NULL) {
        goto error;
    }

    if (!parse_opts(self)) {
        goto error;
    }

    return self;
error:
    CapReplaceCmd_Del(self);
    return NULL;
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
replace(CapReplaceCmd *self) {
    if (self->argc < 4) {
        usage(self);
    }

    const char *cap_fname = self->argv[optind];
    const char *target_ = self->argv[optind + 1];
    const char *replaced = self->argv[optind + 2];
    assert(cap_fname && target_ && replaced);

    string_t *target = PadStr_New();
    char *content = NULL;
    FILE *fout = NULL;

    char path[PAD_FILE__NPATH];
    if (!Cap_SolveCmdlineArgPath(self->config, path, sizeof path, cap_fname)) {
        blush("failed to solve path %s", cap_fname);
        goto error;
    }

    content = PadFile_ReadCopyFromPath(path);
    if (content == NULL) {
        blush("failed to read content from %s", path);
        goto error;
    }

    fout = PadFile_Open(path, "wb");
    if (fout == NULL) {
        blush("failed to open file %s", path);
        goto error;
    }

    Pad_UnescapeText(target, target_, NULL);
    int result = sreplace(fout, content, PadStr_Getc(target), replaced);

    fclose(fout);
    Pad_SafeFree(content);
    PadStr_Del(target);
    return result;
error:
    PadStr_Del(target);
    Pad_SafeFree(content);
    if (fout) {
        fclose(fout);
    }
    return 1;
}

int
CapReplaceCmd_Run(CapReplaceCmd *self) {
    int result = replace(self);
    if (result != 0) {
        PadErrStack_TraceSimple(self->errstack, stderr);
    }
    return result;
}
