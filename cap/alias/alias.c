#include <alias/alias.h>

struct Opts {
    bool is_help;
    bool is_global;
    bool is_desc;
};

struct CapAlCmd {
    const CapConfig *config;
    int argc;
    int optind;
    char **argv;
    struct Opts opts;
    CapAliasMgr *almgr;
    int32_t key_colors[3];
    int32_t value_colors[3];
    int32_t desc_colors[3];
};

static CapAlCmd *
parse_opts(CapAlCmd *self) {
    // Parse options
    static const struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {"global", required_argument, 0, 'g'},
        {"description", no_argument, 0, 'd'},
        {0},
    };
    static const char *shortopts = "hgd";

    self->opts = (struct Opts){0};

    extern int opterr;
    extern int optind;
    opterr = 0; // ignore error messages
    optind = 0; // init index of parse

    for (;;) {
        int optsindex;
        int cur = getopt_long(self->argc, self->argv, shortopts, longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 0: /* long option only */ break;
        case 'h': self->opts.is_help = true; break;
        case 'g': self->opts.is_global = true; break;
        case 'd': self->opts.is_desc = true; break;
        case '?':
        default: PadErr_Die("unknown option"); break;
        }
    }

    if (self->argc < optind) {
        PadErr_Die("failed to parse option");
        return NULL;
    }

    self->optind = optind;

    return self;
}

static void
show_usage(const CapAlCmd *self) {
    fprintf(stdout, "Usage:\n"
        "\n"
        "    cap alias [name] [options]\n"
        "\n"
        "The options are:\n"
        "\n"
        "    -h, --help        show usage.\n"
        "    -g, --global      scope to global.\n"
        "    -d, --description show description of alias.\n"
        "\n"
    );
    fflush(stdout);
}

void
CapAlCmd_Del(CapAlCmd *self) {
    if (!self) {
        return;
    }

    CapAliasMgr_Del(self->almgr);
    free(self);
}

CapAlCmd *
CapAlCmd_New(const CapConfig *config, int argc, char **argv) {
    CapAlCmd *self = PadMem_Calloc(1, sizeof(*self));
    if (self == NULL) {
        return NULL;
    }

    self->config = config;
    self->argc = argc;
    self->argv = argv;
    self->almgr = CapAliasMgr_New(self->config);
    self->key_colors[0] = TERM_GREEN;
    self->key_colors[1] = TERM_BLACK;
    self->key_colors[2] = TERM_BRIGHT;
    self->value_colors[0] = TERM_CYAN;
    self->value_colors[1] = TERM_BLACK;
    self->value_colors[2] = TERM_BRIGHT;
    self->desc_colors[0] = TERM_RED;
    self->desc_colors[1] = TERM_BLACK;
    self->desc_colors[2] = TERM_BRIGHT;

    if (!parse_opts(self)) {
        PadErr_Die("failed to parse options");
        return NULL;
    }

    return self;
}

static CapAlCmd *
load_alias_list_by_opts(CapAlCmd* self) {
    if (self->opts.is_global) {
        if (!CapAliasMgr_LoadAliasList(self->almgr, CAP_SCOPE__GLOBAL)) {
            if (CapAliasMgr_HasErr(self->almgr)) {
                PadErr_Err(CapAliasMgr_GetErrDetail(self->almgr));
            }
            return NULL;
        }
    } else {
        if (!CapAliasMgr_LoadAliasList(self->almgr, CAP_SCOPE__LOCAL)) {
            if (CapAliasMgr_HasErr(self->almgr)) {
                PadErr_Err(CapAliasMgr_GetErrDetail(self->almgr));
            }
            return NULL;
        }
    }
    return self;
}

static const char *
getc_value(CapAlCmd *self, const char *key) {
    const PadCtx *ctx = CapAliasMgr_GetcCtx(self->almgr);
    const PadAliasInfo *alinfo = PadCtx_GetcAliasInfo(ctx);
    return PadAlInfo_GetcValue(alinfo, key);
}

static void
padline(FILE *fout, int32_t len) {
    for (int32_t i = 0; i < len; ++i) {
        fputc(' ', fout);
    }
}

static void
print_key_val_desc(
    const CapAlCmd *self,
    FILE *fout,
    bool print_color,
    int32_t keymaxlen,
    int32_t valmaxlen,
    const char *key,
    const char *val,
    const char *desc
) {
    if (!print_color) {
        printf("%-*s    %-*s    %s\n",
            keymaxlen, key, valmaxlen, val, desc);
        return;
    }

    int32_t difkeylen = keymaxlen - strlen(key);
    int32_t difvallen = valmaxlen - strlen(val);

    PadTerm_CFPrintf(
        fout,
        self->key_colors[0],
        self->key_colors[1],
        self->key_colors[2],
        "%s", key
    );
    padline(fout, difkeylen + 4);
    PadTerm_CFPrintf(
        fout,
        self->value_colors[0],
        self->value_colors[1],
        self->value_colors[2],
        "%s", val
    );
    padline(fout, difvallen + 4);
    PadTerm_CFPrintf(
        fout,
        self->desc_colors[0],
        self->desc_colors[1],
        self->desc_colors[2],
        "%s\n", desc
    );
}

static void
print_key_val(
    const CapAlCmd *self,
    FILE *fout,
    bool print_color,
    int32_t keymaxlen,
    const char *key,
    const char *val
) {
    if (!print_color) {
        printf("%-*s    %s\n", keymaxlen, key, val);
        return;
    }

    int32_t difkeylen = keymaxlen - strlen(key);

    PadTerm_CFPrintf(fout, self->key_colors[0], self->key_colors[1], self->key_colors[2], "%s", key);
    padline(fout, difkeylen + 4);
    PadTerm_CFPrintf(fout, self->value_colors[0], self->value_colors[1], self->value_colors[2], "%s\n", val);
}

static int
show_list(CapAlCmd *self) {
    const PadCtx *ctx = CapAliasMgr_GetcCtx(self->almgr);
    const PadAliasInfo *alinfo = PadCtx_GetcAliasInfo(ctx);
    const PadDict *key_val_map = PadAliasInfo_GetcKeyValueMap(alinfo);
    int keymaxlen = 0;
    int valmaxlen = 0;

#undef max
#define max(a, b) (a > b ? a : b);

    for (int i = 0; i < PadDict_Len(key_val_map); ++i) {
        const PadDictItem *item = PadDict_GetcIndex(key_val_map, i);
        if (!item) {
            continue;
        }
        keymaxlen = max(strlen(item->key), keymaxlen);
        valmaxlen = max(strlen(item->value), valmaxlen);
    }

    FILE *fout = stdout;
    bool print_color = isatty(PadFile_GetNum(fout));

    for (int i = 0; i < PadDict_Len(key_val_map); ++i) {
        const PadDictItem *kv_item = PadDict_GetcIndex(key_val_map, i);
        if (!kv_item) {
            continue;
        }

        const char *desc = PadAliasInfo_GetcDesc(alinfo, kv_item->key);
        if (self->opts.is_desc && desc) {
            char disp_desc[128] = {0};
            trim_first_line(disp_desc, sizeof disp_desc, desc);

            print_key_val_desc(
                self,
                fout,
                print_color,
                keymaxlen,
                valmaxlen,
                kv_item->key,
                kv_item->value,
                disp_desc
            );
        } else {
            print_key_val(
                self,
                fout,
                print_color,
                keymaxlen,
                kv_item->key,
                kv_item->value
            );
        }
    }
    fflush(stdout);

    return 0;
}

static int
show_alias_value(CapAlCmd *self) {
    const char *key = self->argv[self->optind];
    const char *value = getc_value(self, key);
    if (!value) {
        PadErr_Err("not found alias \"%s\"", key);
        return 1;
    }

    puts(value);
    fflush(stdout);
    
    return 0;
}

int
CapAlCmd_ShowDescOfAlias(CapAlCmd *self) {
    const PadCtx *ctx = CapAliasMgr_GetcCtx(self->almgr);
    const PadAliasInfo *alinfo = PadCtx_GetcAliasInfo(ctx);
    const char *key = self->argv[self->optind];
    const char *desc = PadAliasInfo_GetcDesc(alinfo, key);
    if (!desc) {
        return 0;
    }

    puts(desc);
    fflush(stdout);

    return 0;
}

int
alcmd_run(CapAlCmd *self) {
    if (self->opts.is_help) {
        show_usage(self);
        return 0;
    }

    if (!load_alias_list_by_opts(self)) {
        return 1;
    }

    if (self->argc - self->optind == 0) {
        return show_list(self);
    }

    if (self->opts.is_desc) {
        return CapAlCmd_ShowDescOfAlias(self);
    }

    return show_alias_value(self);
}
