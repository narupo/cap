#include <cap/find/find.h>

enum {
    FINDCMD_DEF_MAX_RECURSION = 8,
};

/**
 * Structure of options
 */
struct Opts {
    bool is_help;
    bool is_normalize;
    bool is_alias;
    int max_recursion;
    char origin[PAD_FILE__NPATH];
};

/**
 * Structure of command
 */
struct CapFindCmd {
    const CapConfig *config;
    int argc;
    int optind;
    char **argv;
    struct Opts opts;
    CapArgsMgr *argsmgr;
    CapAliasMgr *almgr;  
};

/**
 * Show usage of command
 *
 * @param[in] self pointer to CapFindCmd
 */
static void
usage(CapFindCmd *self) {
    fflush(stdout);
    fflush(stderr);
    fprintf(stderr, "Usage:\n"
        "\n"
        "    cap find [options]... [arguments]...\n"
        "\n"
        "The options are:\n"
        "\n"
        "    -h, --help             show usage\n"
        "    -n, --normalize        normalize path\n"
        "    -o, --origin           origin path\n"
        "    -a, --alias            find aliases\n"
        "    -m, --max-recursion    max recursion depth (default to 8)\n"
        "\n"
    );
    fflush(stderr);
    exit(0);
}

/**
 * Parse options
 *
 * @param[in] self pointer to CapFindCmd 
 *
 * @return success to true
 * @return failed to false
 */
static bool
parse_opts(CapFindCmd *self) {
    // parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {"normalize", no_argument, 0, 'n'},
        {"alias", no_argument, 0, 'a'},
        {"origin", required_argument, 0, 'o'},
        {"max-recursion", required_argument, 0, 'm'},
        {0},
    };

    self->opts = (struct Opts){0};
    self->opts.max_recursion = FINDCMD_DEF_MAX_RECURSION;

    extern int opterr;
    extern int optind;
    opterr = 0; // ignore error messages
    optind = 0; // init index of parse

    for (;;) {
        int optsindex;
        int cur = getopt_long(self->argc, self->argv, "hnao:m:", longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 0: /* long option only */ break;
        case 'h': self->opts.is_help = true; break;
        case 'n': self->opts.is_normalize = true; break;
        case 'a': self->opts.is_alias = true; break;
        case 'o': snprintf(self->opts.origin, sizeof self->opts.origin, "%s", optarg); break;
        case 'm': self->opts.max_recursion = atoi(optarg); break;
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
CapFindCmd_Del(CapFindCmd *self) {
    if (!self) {
        return;
    }

    CapArgsMgr_Del(self->argsmgr);
    almgr_del(self->almgr);
    free(self);
}

CapFindCmd *
CapFindCmd_New(const CapConfig *config, int argc, char **argv) {
    CapFindCmd *self = PadMem_Calloc(1, sizeof(*self));
    if (self == NULL) {
        return NULL;
    }

    self->config = config;
    self->argc = argc;
    self->argv = argv;
    strcpy(self->opts.origin, ".");
    self->almgr = CapAliasMgr_New(self->config);

    if (!parse_opts(self)) {
        CapFindCmd_Del(self);
        return NULL;
    }

    self->argsmgr = CapArgsMgr_New(self->argv + self->optind);

    return self;
}

static void
join_cap_path(char *dst, int32_t dstsz, const char *lhs, const char *rhs) {
    for (; *rhs == '/' ; ++rhs);
    
    int32_t lhslen = strlen(lhs);

    if (lhslen) {
        bool sep = lhs[lhslen-1] == '/' ? false : true;
        if (sep) {
            snprintf(dst, dstsz, "%s/%s", lhs, rhs);
        } else {
            snprintf(dst, dstsz, "%s%s", lhs, rhs);
        }
    } else {
            snprintf(dst, dstsz, "%s", rhs);
    }
}

static int
find_files_r(const CapFindCmd *self, const char *dirpath, const char *cap_dirpath, int dep) {
    if (dep >= self->opts.max_recursion) {
        return 1;
    }

    PadDir *dir = PadDir_Open(dirpath);
    if (!dir) {
        PadErr_Err("failed to open directory \"%s\"", dirpath);
        return 1;
    }

    int ret = 0;

    for (;;) {
        PadDirNode *node = PadDir_Read(dir);
        if (!node) {
            break;
        }

        const char *name = PadDirNode_Name(node);
        if (PadCStr_Eq(name, ".") || PadCStr_Eq(name, "..")) {
            PadDirNode_Del(node);
            continue;
        }

        char cap_path[PAD_FILE__NPATH];
        join_cap_path(cap_path, sizeof cap_path, cap_dirpath, name);

        char tmp_path[PAD_FILE__NPATH];
        snprintf(tmp_path, sizeof tmp_path, "%s/%s", dirpath, name);

        char path[PAD_FILE__NPATH];
        if (!CapSymlink_FollowPath(self->config, path, sizeof path, tmp_path)) {
            PadErr_Err("failed to follow path on find file recursive");
            PadDirNode_Del(node);
            continue;
        }

        if (CapArgsMgr_ContainsAll(self->argsmgr, name)) {
            if (self->opts.is_normalize) {
                puts(path);
            } else {
                puts(cap_path);
            }
        }

        if (CapSymlink_IsLinkFile(path)) {
            // pass
        } else if (PadFile_IsDir(path)) {
            ret = find_files_r(self, path, cap_path, dep+1);
        }

        PadDirNode_Del(node);
    }

    PadDir_Close(dir);

    return ret;
}

static bool
has_contents(const CapFindCmd *self, const PadDict *alias_kvmap, int32_t *maxkeylen, int32_t *maxvallen) {
    bool has = false;
    for (int32_t i = 0; i < PadDict_Len(alias_kvmap); ++i) {
        const PadDictItem *item = PadDict_GetcIndex(alias_kvmap, i);
        if (CapArgsMgr_ContainsAll(self->argsmgr, item->key)) {
            int32_t keylen = strlen(item->key);
            int32_t vallen = strlen(item->value);
            *maxkeylen = keylen > *maxkeylen ? keylen : *maxkeylen;
            *maxvallen = vallen > *maxvallen ? vallen : *maxvallen;
            has = true;
        }
    }

    return has;
}

static int
find_aliases_r(const CapFindCmd *self, const char *dirpath, const char *cap_dirpath, int dep) {
    char alpath[PAD_FILE__NPATH];
    if (dep >= self->opts.max_recursion) {
        return 1;
    }

    if (!PadFile_Solvefmt(alpath, sizeof alpath, "%s/.caprc", dirpath)) {
        // not error
    }

    CapAliasMgr_Clear(self->almgr);
    if (PadFile_IsExists(alpath)) {
        if (!CapAliasMgr_LoadPath(self->almgr, alpath)) {
            PadErr_Err("failed to load resource file \"%s\" for alias", alpath);
            return 1;
        }
    }

    const PadCtx *ctx = CapAliasMgr_GetcContext(self->almgr);
    const PadAliasInfo *alinfo = PadCtx_GetcAliasInfo(ctx);
    const PadDict *alias_kvmap = PadAliasInfo_GetcKeyValueMap(alinfo);
    int32_t maxkeylen = 0;
    int32_t maxvallen = 0;
    bool hascontents = has_contents(self, alias_kvmap, &maxkeylen, &maxvallen);
    const char *disppath = self->opts.is_normalize ? dirpath : cap_dirpath;
    disppath = strlen(disppath) ? disppath : ".";

    if (PadDict_Len(alias_kvmap) && hascontents) {
        printf("%s\n\n", disppath);
    }

    for (int32_t i = 0; i < PadDict_Len(alias_kvmap); ++i) {
        const PadDictItem *item = PadDict_GetcIndex(alias_kvmap, i);
        if (CapArgsMgr_ContainsAll(self->argsmgr, item->key)) {
            printf("    %-*s    %-*s\n", maxkeylen, item->key, maxvallen, item->value);
        }
    }

    if (hascontents) {
        printf("\n");
    }

    PadDir *dir = PadDir_Open(dirpath);
    if (!dir) {
        PadErr_Err("failed to open directory \"%s\"", dirpath);
        return 1;
    }

    int ret = 0;

    for (;;) {
        PadDirNode *node = PadDir_Read(dir);
        if (!node) {
            break;
        }

        const char *name = PadDirNode_Name(node);
        if (PadCStr_Eq(name, ".") || PadCStr_Eq(name, "..")) {
            PadDirNode_Del(node);
            continue;
        }

        char cap_path[PAD_FILE__NPATH];
        join_cap_path(cap_path, sizeof cap_path, cap_dirpath, name);

        char tmp_path[PAD_FILE__NPATH];
        snprintf(tmp_path, sizeof tmp_path, "%s/%s", dirpath, name);

        char path[PAD_FILE__NPATH];
        if (!CapSymlink_FollowPath(self->config, path, sizeof path, tmp_path)) {
            PadErr_Err("failed to follow path on find file recursive");
            PadDirNode_Del(node);
            continue;
        }

        if (PadFile_IsDir(path)) {
            ret = find_aliases_r(self, path, cap_path, dep+1);
        }

        PadDirNode_Del(node);
    }

    return ret;
}

static int
find_start(const CapFindCmd *self) {
    const char *origin = Cap_GetOrigin(self->config, self->opts.origin);
    char tmppath[PAD_FILE__NPATH*2];
    snprintf(tmppath, sizeof tmppath, "%s/%s", origin, self->opts.origin);

    char path[PAD_FILE__NPATH];
    if (!CapSymlink_FollowPath(self->config, path, sizeof path, tmppath)) {
        PadErr_Err("failed to follow path in find files");
        return 1;
    }
    
    if (self->opts.is_alias) {
        return find_aliases_r(self, path, self->opts.origin, 0);
    }

    return find_files_r(self, path, self->opts.origin, 0);
}

int
CapFindCmd_Run(CapFindCmd *self) {
    int nargs = self->argc - self->optind;

    if (nargs == 0 || self->opts.is_help) {
        usage(self);
    }

    return find_start(self);
}
