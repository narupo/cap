#include <cap/insert/insert.h>

typedef enum {
    AFTER,
    BEFORE,
    CENTER,
} Pos;

/**
 * structure of options
 */
struct Opts {
    bool is_help;
    char after[1024];
    char before[1024];
};

/**
 * structure of command
 */
struct CapInsertCmd {
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
 * @param[in] self pointer to CapInsertCmd
 */
static int
usage(CapInsertCmd *self) {
    fflush(stdout);
    fflush(stderr);
    fprintf(stderr, "Usage:\n"
        "\n"
        "    cap insert [file] [elem] [options]...\n"
        "\n"
        "The options are:\n"
        "\n"
        "    -h, --help      Show usage\n"
        "    -a, --after     Insert after position of label or line no\n"
        "    -b, --before    Insert before position of label or line no\n"
        "\n"
    );
    fflush(stderr);
    return 0;
}

/**
 * parse options
 *
 * @param[in] self pointer to CapInsertCmd 
 *
 * @return success to true
 * @return failed to false
 */
static bool
parse_opts(CapInsertCmd *self) {
    // parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {"after", required_argument, 0, 'a'},
        {"before", required_argument, 0, 'b'},
        {0},
    };

    self->opts = (struct Opts){0};

    extern int opterr;
    extern int optind;
    opterr = 0; // ignore error messages
    optind = 0; // init index of parse

    for (;;) {
        int optsindex;
        int cur = getopt_long(self->argc, self->argv, "ha:b:", longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 0: /* long option only */ break;
        case 'h': self->opts.is_help = true; break;
        case 'a': snprintf(self->opts.after, sizeof self->opts.after, "%s", optarg); break;
        case 'b': snprintf(self->opts.before, sizeof self->opts.before, "%s", optarg); break;
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
CapInsertCmd_Del(CapInsertCmd *self) {
    if (!self) {
        return;
    }

    PadErrStack_Del(self->errstack);
    free(self);
}

CapInsertCmd *
CapInsertCmd_New(const CapConfig *config, int argc, char **argv) {
    CapInsertCmd *self = PadMem_Calloc(1, sizeof(*self));
    if (self == NULL) {
        return NULL;
    }

    self->config = config;
    self->argc = argc;
    self->argv = argv;
    self->errstack = PadErrStack_New();

    if (!parse_opts(self)) {
        CapInsertCmd_Del(self);
        return NULL;
    }

    return self;
}

static int32_t 
find_insert_lieno(CapInsertCmd *self, FILE *fp, int32_t lineno, Pos insert_pos) {
    int32_t pos = 0;
    int32_t curlineno = 1;

    switch (insert_pos) {
    default: break;
    case BEFORE: lineno -= 1; break;
    case AFTER: lineno += 1; break;
    }

    for (;;) {
        pos = ftell(fp);

        int c = fgetc(fp);
        if (c == EOF) {
            break;
        }

        if (curlineno == lineno) {
            break;
        }

        if (c == '\n') {
            curlineno += 1;
        }
    }

    return pos;
}

static bool
read_line(char *dst, int32_t dstsz, FILE *fin) {
    char *dp = dst;
    char *dend = dst + dstsz - 1;

    for (; dp < dend; ) {
        int32_t c = fgetc(fin);
        if (c == EOF) {
            goto end;
        } else if (c == '\n') {
            goto ok;
        }

        *dp++ = c;
    }

ok:
    *dp = '\0';
    return true;
end:
    *dp = '\0';
    return false;
}

static int32_t
find_insert_label(CapInsertCmd *self, FILE *fp, const char *label, Pos insert_pos) {
    char lbl[1024];
    char line[1024];
    int32_t pos = 0;
    int32_t before_pos = 0;

    snprintf(lbl, sizeof lbl, "@cap.label=%s", label);

    for (;;) {
        if (!read_line(line, sizeof line, fp)) {
            break;
        }

        before_pos = pos;
        pos = ftell(fp);

        if (strstr(line, lbl)) {
            switch (insert_pos) {
            default: break;
            case BEFORE:
                pos = before_pos;
                break;
            case AFTER:
                // nothing todo
                break;
            }
            break;
        }
    }

    return pos;
}

static int32_t
find_insert_pos(CapInsertCmd *self, const char *path) {
    char *label = NULL;
    int32_t lineno = -1;
    Pos insert_pos = AFTER;
    int32_t pos = 0;

    FILE *fp = fopen(path, "rb");
    if (fp == NULL) {
        Pad_PushErr("failed to open file %s", path);
        return 1;
    }

    if (strlen(self->opts.after)) {
        label = self->opts.after;
        insert_pos = AFTER;
    } else if (strlen(self->opts.before)) {
        label = self->opts.before;
        insert_pos = BEFORE;
    } else {
        fseek(fp, 0, SEEK_END);
        pos = ftell(fp);
        fclose(fp);
        return pos;
    }

    if (isdigit(label[0])) {
        lineno = atoi(label);
    }

    if (lineno >= 0) {
        pos = find_insert_lieno(self, fp, lineno, insert_pos);
    } else {
        pos = find_insert_label(self, fp, label, insert_pos);
    }

    fclose(fp);
    return pos;
}

static int32_t
insert_tail(CapInsertCmd *self, const char *path, const char *elem) {
    char *s = PadFile_ReadCopyFromPath(path);
    if (s == NULL) {
        Pad_PushErr("failed to read");
        return 1;
    }

    FILE *fout = fopen(path, "wb");
    if (fout == NULL) {
        Pad_PushErr("failed to open file (2) %s", path);
        return 1;
    }

    for (char *p = s; *p; p += 1) {
        fputc(*p, fout);
    }
    for (const char *p = elem; *p; p += 1) {
        fputc(*p, fout);
    }

    fclose(fout);
    free(s);
    return 0;
}

static int32_t
insert_at(CapInsertCmd *self, const char *path, int32_t pos, const char *elem) {
    FILE *fin = fopen(path, "rb");
    if (fin == NULL) {
        Pad_PushErr("failed to open file %s", path);
        return 1;
    }

    char *s = PadFile_ReadCopy(fin);
    if (s == NULL) {
        Pad_PushErr("failed to read");
        return 1;
    }

    fclose(fin);

    FILE *fout = fopen(path, "wb");
    if (fout == NULL) {
        Pad_PushErr("failed to open file (2) %s", path);
        return 1;
    }

    char *p = s;
    for (int32_t curpos = 0; ; p += 1) {
        if (curpos == pos) {
            for (const char *q = elem; *q; q += 1) {
                fputc(*q, fout);
            }
        }

        if (*p == '\0') {
            break;
        }

        fputc(*p, fout);
        curpos = ftell(fout);
    }

    fclose(fout);
    free(s);
    return 0;
}

static char *
unescape(CapInsertCmd *self, const char *raw) {
    PadStr *s = PadStr_New();

    for (const char *p = raw; *p; p += 1) {
        switch (*p) {
        default:
            PadStr_PushBack(s, *p);
            break;
        case '\\':
            Pad_Unescape(s, &p, NULL);
            break;
        }
    }

    return PadStr_EscDel(s);
}

static int
insert(CapInsertCmd *self) {
    const char *cap_path = NULL;
    const char *raw_elem = NULL;
    char *elem = NULL;
    char path[PAD_FILE__NPATH];
    int result = 0;

    if (self->argc < 3) {
        return usage(self);
    } else {
        cap_path = self->argv[optind];
        if (!Cap_SolveCmdlineArgPath(self->config, path, sizeof path, cap_path)) {
            Pad_PushErr("failed to solve path");
            goto error;
        }

        raw_elem = self->argv[optind + 1];
        elem = unescape(self, raw_elem);
    }

    int32_t pos = find_insert_pos(self, path);
    if (pos < 0) {
        result = insert_tail(self, path, elem);
    } else {
        result = insert_at(self, path, pos, elem);
    }

    free(elem);
    return result;
error:
    free(elem);
    return 1;
}

int
CapInsertCmd_Run(CapInsertCmd *self) {
    Pad_ShowArgv(self->argc, self->argv);
    int result = insert(self);
    if (PadErrStack_Len(self->errstack)) {
        PadErrStack_TraceSimple(self->errstack, stderr);
        return result;
    }
    return result;
}
