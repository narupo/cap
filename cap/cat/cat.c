#include <cap/cat/cat.h>

/**
 * Structure of options
 */
struct Opts {
    bool is_help;
    bool is_tab;
    bool is_make;
    int indent;
    int tabspaces;
};

/**
 * Structure of command
 */
struct CapCatCmd {
    const CapConfig *config;
    PadErrStack *errstack;
    int argc;
    char **argv;
    struct Opts opts;
    int optind;
    bool is_debug;
};

/**
 * Parse options
 *
 * @param[in] *self
 * @param[in] argc
 * @param[in] *argv[]
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
static CapCatCmd *
parse_opts(CapCatCmd *self) {
    // Parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {"indent", required_argument, 0, 'i'},
        {"tabspaces", required_argument, 0, 'T'},
        {"tab", no_argument, 0, 't'},
        {"make", no_argument, 0, 'm'},
        {0},
    };

    self->opts = (struct Opts){
        .is_help = false,
        .is_tab = false,
        .is_make = false,
        .indent = 0,
        .tabspaces = 4,
    };
    opterr = 0;
    optind = 0;

    for (;;) {
        int optsindex;
        int cur = getopt_long(self->argc, self->argv, "hi:T:tm", longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 0: /* Long option only */ break;
        case 'h': self->opts.is_help = true; break;
        case 'i': self->opts.indent = atoi(optarg); break;
        case 'T': self->opts.tabspaces = atoi(optarg); break;
        case 't': self->opts.is_tab = true; break;
        case 'm': self->opts.is_make = true; break;
        case '?':
        default:
            PadErr_Die("unsupported option");
            return NULL;
            break;
        }
    }

    if (self->argc < optind) {
        return NULL;
    }

    self->optind = optind;

    return self;
}

/**
 * Destruct command
 *
 * @param[in] pointer to allocate memory of command
 */
void
CapCatCmd_Del(CapCatCmd *self) {
    if (!self) {
        return;
    }
    PadErrStack_Del(self->errstack);
    Pad_SafeFree(self);
}

/**
 * Construct command
 *
 * @param[in] config pointer to config with move semantics
 * @param[in] argc   number of arguments
 * @param[in] argv   pointer to string array with move semantics
 * @return pointer to allocate memory of command
 */
CapCatCmd *
CapCatCmd_New(const CapConfig *config, int argc, char **argv) {
    CapCatCmd *self = PadMem_Calloc(1, sizeof(*self));
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
        PadErr_Err("failed to parse options");
        goto error;
    }

    return self;
error:
    CapCatCmd_Del(self);
    return NULL;
}

/**
 * Make file path for concatenate
 *
 * @param[in] *dst destination buffer
 * @param[in] dstsz size of destination buffer
 * @param[in] *cap_path string of cap_path
 *
 * @return success to pointer to destination buffer
 * @return failed to NULL
 */
static char *
make_path(CapCatCmd *self, char *dst, size_t dstsz, const char *cap_path) {
    if (!Cap_SolveCmdlineArgPath(self->config, dst, dstsz, cap_path)) {
        return NULL;
    }

    if (PadFile_IsDir(dst)) {
        return NULL;
    }

    return dst;
}

/**
 * Set indent characters.
 *
 * @param[in] *opts options
 * @param[out] *buf buffer
 * @param[in] bufsize buffer size
 *
 * @return success to true
 * @return failed to false
 */
static bool
set_indent(CapCatCmd *self, char *buf, size_t bufsize) {
    if (self->opts.is_tab) {
        buf[0] = '\t';
        buf[1] = '\0';
    } else {
        if (self->opts.tabspaces >= bufsize - 1) {
            return false;
        }

        memset(buf, ' ', self->opts.tabspaces);
        buf[self->opts.tabspaces] = '\0';
    }

    return true;
}

/**
 * Show usage and exit from proccess
 *
 * @param[in] self
 */
static int
usage(const CapCatCmd *self) {
    fprintf(stderr,
        "Usage:\n"
        "\n"
        "    cap cat [options] [files]\n"
        "\n"
        "The options are:\n"
        "\n"
        "    -h, --help       show usage.\n"
        "    -i, --indent     indent spaces.\n"
        "    -T, --tabspaces  number of tab spaces.\n"
        "    -t, --tab        tab indent mode.\n"
        "    -m, --make       with make.\n"
        "\n"
        "Examples:\n"
        "\n"
        "    $ cap cat f - g\n"
        "    $ cap cat\n"
        "\n"
    );
    return 0;
}

/**
 * Read content from stream
 *
 * @param[in] *self pointer to CapCatCmd
 * @param[in] *fin pointer to FILE of source
 *
 * @return pointer to content
 */
static PadStr *
read_stream(CapCatCmd *self, FILE *fin) {
    PadStr *buf = PadStr_New();

    for (;;) {
        int c = fgetc(fin);
        if (c == EOF) {
            break;
        }
        PadStr_PushBack(buf, c);
    }

    return buf;
}

/**
 * Write buffer at stream
 *
 * @param[in] *self pointer to CapCatCmd
 * @param[in] *fout pointer to FILE of destination
 * @param[in] *buf pointer to buffer
 *
 * @return success to true, failed to false
 */
static bool
write_stream(CapCatCmd *self, const char *fname, FILE *fout, const PadStr *buf) {
    bool ret = true;
    CapKit *kit = NULL;
    PadStr *out = NULL;
    const char *p = PadStr_Getc(buf);
    int m = 0;

    if (self->opts.is_make) {
        CapKit *kit = CapKit_New(self->config);
        if (kit == NULL) {
            Pad_PushErr("failed to create kit");
            goto error;
        }

        if (!CapKit_CompileFromStrArgs(
            kit,
            fname,
            PadStr_Getc(buf),
            0,
            NULL
        )) {
            const PadErrStack *es = CapKit_GetcErrStack(kit);
            PadErrStack_ExtendBackOther(self->errstack, es);
            Pad_PushErr("failed to compile");
            goto error;
        }

        p = CapKit_GetcStdoutBuf(kit);
    }

    // set indent
    out = PadStr_New();

    for (; *p; ) {
        char c = *p++;

        switch (m) {
        case 0: { // Indent mode
            char indent[100] = {0};
            if (!set_indent(self, indent, sizeof indent)) {
                return false;
            }

            for (int i = 0; i < self->opts.indent; ++i) {
                PadStr_App(out, indent);
            }

            PadStr_PushBack(out, c);
            if (c != '\n') {
                m = 1;
            }
        } break;
        case 1: { // Stream mode
            PadStr_PushBack(out, c);
            if (c == '\n') {
                m = 0;
            }
        } break;
        }
    }

    printf("%s", PadStr_Getc(out));
    fflush(fout);

error:
    CapKit_Del(kit);
    PadStr_Del(out);
    return ret;
}

/**
 * Read content of file of path
 *
 * @param[in] *self pointer to CapCatCmd
 * @param[in] *path path of file
 *
 * @return pointer to PadStr
 */
static PadStr *
read_file(CapCatCmd *self, const char *path) {
    FILE *fin = fopen(path, "rb");
    if (fin == NULL) {
        return NULL;
    }

    PadStr *buf = read_stream(self, fin);

    if (fclose(fin) < 0) {
        PadStr_Del(buf);
        return NULL;
    }

    return buf;
}

void
CapCatCmd_SetDebug(CapCatCmd *self, bool debug) {
    self->is_debug = debug;
}

/**
 * Execute command
 *
 * @param[in] self
 * @return success to number of 0
 * @return failed to number of other of 0
 */
int
CapCatCmd_Run(CapCatCmd *self) {
    if (self->opts.is_help) {
        return usage(self);
    }

    if (self->argc - self->optind + 1 < 2) {
        PadStr *stdinbuf = read_stream(self, stdin);
        write_stream(self, NULL, stdout, stdinbuf);
        PadStr_Del(stdinbuf);
        return 0;
    }

    int ret = 0;
    for (int i = self->optind; i < self->argc; ++i) {
        const char *name = self->argv[i];

        if (PadCStr_Eq(name, "-")) {
            PadStr *stdinbuf = read_stream(self, stdin);
            write_stream(self, NULL, stdout, stdinbuf);
            PadStr_Del(stdinbuf);
            continue;
        }

        char path[PAD_FILE__NPATH];
        if (!make_path(self, path, sizeof path, name)) {
            ++ret;
            PadErr_Err("failed to make path by \"%s\"", name);
            continue;
        }

        PadStr *filebuf = read_file(self, path);
        if (!filebuf) {
            ++ret;
            PadErr_Err("failed to read file from \"%s\"", path);
            continue;
        }

        write_stream(self, path, stdout, filebuf);
        PadStr_Del(filebuf);
    }

    fflush(stdout);

    if (PadErrStack_Len(self->errstack)) {
        PadErrStack_TraceSimple(self->errstack, stderr);
        fflush(stderr);
    }

    return ret;
}
