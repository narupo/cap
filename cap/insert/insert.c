#include "insert/insert.h"

#define error(fmt, ...) errstack_add(self->errstack, fmt, ##__VA_ARGS__)

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
struct insert {
    const CapConfig *config;
    int argc;
    int optind;
    char **argv;
    struct Opts opts;
    errstack_t *errstack;
};

/**
 * show usage of command
 *
 * @param[in] self pointer to insertcmd_t
 */
static void
insertcmd_show_usage(insertcmd_t *self) {
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
}

/**
 * parse options
 *
 * @param[in] self pointer to insertcmd_t 
 *
 * @return success to true
 * @return failed to false
 */
static bool
insertcmd_parse_opts(insertcmd_t *self) {
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
insertcmd_del(insertcmd_t *self) {
    if (!self) {
        return;
    }

    errstack_del(self->errstack);
    free(self);
}

insertcmd_t *
insertcmd_new(const CapConfig *config, int argc, char **argv) {
    insertcmd_t *self = PadMem_ECalloc(1, sizeof(*self));

    self->config = config;
    self->argc = argc;
    self->argv = argv;
    self->errstack = errstack_new();

    if (!insertcmd_parse_opts(self)) {
        insertcmd_del(self);
        return NULL;
    }

    return self;
}

static int32_t 
find_insert_lieno(insertcmd_t *self, FILE *fp, int32_t lineno, Pos insert_pos) {
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
find_insert_label(insertcmd_t *self, FILE *fp, const char *label, Pos insert_pos) {
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
find_insert_pos(insertcmd_t *self, const char *path) {
    char *label = NULL;
    int32_t lineno = -1;
    Pos insert_pos = AFTER;
    int32_t pos = 0;

    FILE *fp = fopen(path, "rb");
    if (fp == NULL) {
        error("failed to open file %s", path);
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
insert_tail(insertcmd_t *self, const char *path, const char *elem) {
    puts("tail!");
    char *s = file_readcp_from_path(path);
    if (s == NULL) {
        error("failed to read");
        return 1;
    }

    FILE *fout = fopen(path, "wb");
    if (fout == NULL) {
        error("failed to open file (2) %s", path);
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
insert_at(insertcmd_t *self, const char *path, int32_t pos, const char *elem) {
    FILE *fin = fopen(path, "rb");
    if (fin == NULL) {
        error("failed to open file %s", path);
        return 1;
    }

    char *s = file_readcp(fin);
    if (s == NULL) {
        error("failed to read");
        return 1;
    }

    fclose(fin);

    FILE *fout = fopen(path, "wb");
    if (fout == NULL) {
        error("failed to open file (2) %s", path);
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
fix_elem(insertcmd_t *self, const char *raw) {
    string_t *s = PadStr_New();

    for (const char *p = raw; *p; p += 1) {
        switch (*p) {
        default:
            str_pushb(s, *p);
            break;
        case '\\':
            unescape(s, &p, NULL);
            break;
        }
    }

    return str_esc_del(s);
}

static int
insert(insertcmd_t *self) {
    const char *cap_path = NULL;
    const char *raw_elem = NULL;
    char *elem = NULL;
    char path[FILE_NPATH];
    int result = 0;

    if (self->argc < 3) {
        insertcmd_show_usage(self);
        return 0;
    } else {
        cap_path = self->argv[optind];
        if (!Cap_SolveCmdlineArgPath(self->config, path, sizeof path, cap_path)) {
            error("failed to solve path");
            goto error;
        }

        raw_elem = self->argv[optind + 1];
        elem = fix_elem(self, raw_elem);
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
insertcmd_run(insertcmd_t *self) {
    int result = insert(self);
    if (errstack_len(self->errstack)) {
        errstack_trace(self->errstack, stderr);
        return result;
    }
    return result;
}
