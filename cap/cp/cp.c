#include <cp/cp.h>

/**
 * Numbers
 */
typedef enum {
    CPCMD_ERR__NOERR = 0,
    CPCMD_ERR__SOLVEPATH,
    CPCMD_ERR__OUTOFHOME,
    CPCMD_ERR__COPY,
    CPCMD_ERR__OPENFILE,
    CPCMD_ERR__BASENAME,
    CPCMD_ERR__OPENDIR,
    CPCMD_ERR__MKDIR,
} Errno;

/**
 * Structure of options
 */
struct Opts {
    bool is_help;
    bool is_recursive;
};

/**
 * Structure of command
 */
struct CapCpCmd {
    const CapConfig *config;
    int argc;
    int optind;
    Errno errno_;
    char **argv;
    struct Opts opts;
    char what[2048];
};

/**
 * Show usage of command
 *
 * @param[in] self pointer to CapCpCmd
 */
static void
usage(CapCpCmd *self) {
    fflush(stdout);
    fflush(stderr);
    fprintf(stderr, "Copy files.\n"
        "\n"
        "Usage:\n"
        "\n"
        "    cap cp [src] [dst] [options...]\n"
        "    cap cp [src] [dst dir] [options...]\n"
        "    cap cp [src] [src] [dst dir] [options...]\n"
        "\n"
        "The options are:\n"
        "\n"
        "    -h, --help         show usage\n"
        "    -r, --recursive    copy directory recursively\n"
        "\n"
        "Examples:\n"
        "\n"
        "    Copy from Cap's environment to user's environment.\n"
        "\n"
        "        $ cap cp path/to/src.txt :path/to/dst.txt\n"
        "\n"
        "    Copy from user's environment to Cap's environment.\n"
        "\n"
        "        $ cap cp :path/to/src.txt path/to/dst.txt\n"
        "\n"
    );
    fflush(stderr);
    exit(0);
}

/**
 * Parse options
 *
 * @param[in] self pointer to CapCpCmd
 *
 * @return success to true
 * @return failed to false
 */
static bool
parse_opts(CapCpCmd *self) {
    // parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {"recusive", required_argument, 0, 'r'},
        {0},
    };

    self->opts = (struct Opts){0};

    extern int opterr;
    extern int optind;
    opterr = 0; // ignore error messages
    optind = 0; // init index of parse

    for (;;) {
        int optsindex;
        int cur = getopt_long(self->argc, self->argv, "hr", longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 0: /* long option only */ break;
        case 'h': self->opts.is_help = true; break;
        case 'r': self->opts.is_recursive = true; break;
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
CapCpCmd_Del(CapCpCmd *self) {
    if (!self) {
        return;
    }
    free(self);
}

CapCpCmd *
CapCpCmd_New(const CapConfig *config, int argc, char **argv) {
    CapCpCmd *self = PadMem_Calloc(1, sizeof(*self));
    if (self == NULL) {
        return NULL;
    }

    self->config = config;
    self->argc = argc;
    self->argv = argv;

    if (!parse_opts(self)) {
        CapCpCmd_Del(self);
        return NULL;
    }

    return self;
}

static void
set_err(CapCpCmd *self, Errno errno_, const char *fmt, ...) {
    self->errno_ = errno_;
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(self->what, sizeof self->what, fmt, ap);
    va_end(ap);
}

static char *
solve_path(CapCpCmd *self, char *dst, size_t dstsz, const char *path) {
    char tmppath[FILE_NPATH*2];

    if (path[0] == ':') {
        // the path on the user's file system
        const char *pth = path+1;
        if (!PadFile_Solve(dst, dstsz, pth)) {
            set_err(self, CPCMD_ERR__SOLVEPATH, "failed to solve path from \"%s\"", pth);
            return NULL;
        }
        return dst;
    }

    // the path on the Cap's environment
    const char *src_path = path;
    const char *org = NULL;

    if (src_path[0] == '/') {
        org = self->config->home_path;
    } else if (self->config->scope == CAP_SCOPE_LOCAL) {
        org = self->config->cd_path;
    } else if (self->config->scope == CAP_SCOPE_GLOBAL) {
        org = self->config->home_path;
    } else {
        PadErr_Die("impossible. invalid state in solve path");
    }

    snprintf(tmppath, sizeof tmppath, "%s/%s", org, src_path);
    if (!CapSymlink_FollowPath(self->config, dst, dstsz, tmppath)) {
        set_err(self, CPCMD_ERR__SOLVEPATH, "failed to solve path from \"%s\"", path);
        return NULL;
    }

    return dst;
}

static bool
copy_file(CapCpCmd *self, const char *dst_path, const char *src_path) {
    FILE *dstfp = PadFile_Open(dst_path, "wb");
    if (!dstfp) {
        set_err(self, CPCMD_ERR__OPENFILE, "failed to open destination file \"%s\"", dst_path);
        return false;
    }

    FILE *srcfp = PadFile_Open(src_path, "rb");
    if (!srcfp) {
        set_err(self, CPCMD_ERR__OPENFILE, "failed to open source file \"%s\"", src_path);
        return false;
    }

    if (!PadFile_Copy(dstfp, srcfp)) {
        set_err(self, CPCMD_ERR__COPY, "failed to copy file from \"%s\" to \"%s\"", src_path, dst_path);
        return false;
    }

    PadFile_Close(dstfp);
    PadFile_Close(srcfp);
    return true;
}

static bool
copy_re(CapCpCmd *self, const char *dst_path, const char *src_path) {
    file_dir_t *dir = PadFileDir_Open(src_path);
    if (!dir) {
        set_err(self, CPCMD_ERR__OPENDIR, "failed to open directory \"%s\"", src_path);
        return false;
    }

    for (PadDirNode *node; (node = PadFileDir_Read(dir)); ) {
        const char *fname = PadDirNode_Name(node);
        if (!strcmp(fname, ".") || !strcmp(fname, "..")) {
            continue;
        }

        char norm_src_path[FILE_NPATH];
        char tmppath[FILE_NPATH];

        snprintf(tmppath, sizeof tmppath, "%s/%s", src_path, fname);
        if (!CapSymlink_FollowPath(self->config, norm_src_path, sizeof norm_src_path, tmppath)) {
            set_err(self, CPCMD_ERR__SOLVEPATH, "failed to solve source path by \"%s\"", fname);
            goto fail;
        }

        char norm_dst_path[FILE_NPATH];

        snprintf(tmppath, sizeof tmppath, "%s/%s", dst_path, fname);
        if (!CapSymlink_FollowPath(self->config, norm_dst_path, sizeof norm_dst_path, tmppath)) {
            set_err(self, CPCMD_ERR__SOLVEPATH, "failed to solve destination path by \"%s\"", fname);
            goto fail;
        }

        if (PadFile_IsDir(norm_src_path)) {
            if (!PadFile_IsDir(norm_dst_path) && PadFile_MkdirQ(norm_dst_path) != 0) {
                set_err(self, CPCMD_ERR__MKDIR, "failed to make directory \"%s\"", norm_dst_path);
                goto fail;
            }
            if (!copy_re(self, norm_dst_path, norm_src_path)) {
                goto fail;
            }
        } else {
            copy_file(self, norm_dst_path, norm_src_path);
        }
    }

    PadFileDir_Close(dir);
    return true;
fail:
    PadFileDir_Close(dir);
    return false;
}

static bool
cp_src2dst_r(CapCpCmd *self, const char *dst_path, const char *src_path) {
    // dst_path が存在する場合
    //     dst_path がディレクトリの場合
    //         dst_path 以下に src の親ディレクトリごとコピーする
    //     dst_path がファイルの場合
    //         エラーを出す
    // dst_path が存在しない場合
    //     dst_path を mkdir
    //         作成したディレクトリ以下に src の親ディレクトリ以下をコピーする
    if (!PadFile_IsDir(src_path)) {
        set_err(self, CPCMD_ERR__COPY, "\"%s\" is not a directory (1)", src_path);
        return false;
    }

    if (PadFile_IsExists(dst_path)) {
        if (PadFile_IsDir(dst_path)) {
            char basename[FILE_NPATH];
            file_basename(basename, sizeof basename, src_path);
            char dstdirpath[FILE_NPATH];
            PadFile_Solvefmt(dstdirpath, sizeof dstdirpath, "%s/%s", dst_path, basename);
            if (!PadFile_IsExists(dstdirpath) && PadFile_MkdirQ(dstdirpath) != 0) {
                set_err(self, CPCMD_ERR__MKDIR, "failed to make directory \"%s\"", dstdirpath);
                return false;
            }
            return copy_re(self, dstdirpath, src_path);
        } else {
            set_err(self, CPCMD_ERR__COPY, "\"%s\" is not a directory (2)", dst_path);
            return false;
        }
    } else {
        if (PadFile_MkdirQ(dst_path) != 0) {
            set_err(self, CPCMD_ERR__MKDIR, "failed to make directory \"%s\"", dst_path);
            return false;
        }
        return copy_re(self, dst_path, src_path);
    }
}

static bool
cp_src2dst(CapCpCmd *self, const char *dst_path, const char *src_path) {
    if (PadFile_IsDir(src_path)) {
        if (self->opts.is_recursive) {
            return cp_src2dst_r(self, dst_path, src_path);
        } else {
            set_err(self, CPCMD_ERR__COPY, "omitting directory \"%s\"", src_path);
            return false;
        }
    }

    if (PadFile_IsDir(dst_path)) {
        char basename[FILE_NPATH];
        if (!file_basename(basename, sizeof basename, src_path)) {
            set_err(self, CPCMD_ERR__BASENAME, "failed to get basename from \"%s\"", src_path);
            return false;
        }

        char newdstpath[FILE_NPATH];
        char tmppath[FILE_NPATH*2];

        snprintf(tmppath, sizeof tmppath, "%s/%s", dst_path, basename);
        if (!CapSymlink_FollowPath(self->config, newdstpath, sizeof newdstpath, tmppath)) {
            set_err(self, CPCMD_ERR__SOLVEPATH, "failed to solve path from \"%s\"", basename);
            return false;
        }

        if (!copy_file(self, newdstpath, src_path)) {
            return false;
        }
    } else {
        if (!copy_file(self, dst_path, src_path)) {
            return false;
        }
    }

    return true;
}

static bool
cp_to(CapCpCmd *self, const char *to, const char *from) {
    char src_path[FILE_NPATH];
    if (!solve_path(self, src_path, sizeof src_path, from)) {
        return false;
    }
    if (from[0] != ':' && Cap_IsOutOfHome(self->config->home_path, src_path)) {
        set_err(self, CPCMD_ERR__OUTOFHOME, "\"%s\" is out of home", from);
        return false;
    }

    char dst_path[FILE_NPATH];
    if (!solve_path(self, dst_path, sizeof dst_path, to)) {
        return false;
    }

    if (to[0] != ':' && Cap_IsOutOfHome(self->config->home_path, dst_path)) {
        set_err(self, CPCMD_ERR__OUTOFHOME, "\"%s\" is out of home (2)", to);
        return false;
    }

    if (!cp_src2dst(self, dst_path, src_path)) {
        return false;
    }

    return true;
}

static int
cp(CapCpCmd *self) {
    int nargs = self->argc - self->optind;
    if (nargs == 2) {
        const char *from = self->argv[self->optind];
        const char *to = self->argv[self->optind+1];
        if (!cp_to(self, to, from)) {
            return 1;
        }
    } else {
        const char *to = self->argv[self->argc-1];

        for (int i = self->optind; i < self->argc-1; ++i) {
            const char *from = self->argv[i];
            if (!cp_to(self, to, from)) {
                return 1;
            }
        }
    }
    return 0;
}

int
CapCpCmd_Run(CapCpCmd *self) {
    if (self->argc < self->optind+2) {
        usage(self);
        return 1;
    }

    if (self->opts.is_help) {
        usage(self);
        return 1;
    }

    int ret = cp(self);
    if (ret != 0) {
        PadErr_Error(self->what);
    }
    return ret;
}
