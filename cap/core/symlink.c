#include <core/symlink.h>

static const char *
skip_drive_letter(const char *path) {
    const char *found = strchr(path, ':');
    if (!found) {
        return NULL;
    }
    return ++found;
}

static bool
is_contain_header(char *data, uint32_t datasz) {
    size_t len1 = strlen(data);
    size_t len2 = strlen(SYMLINK_HEADER);
    size_t size = len1 < len2 ? len1 : len2;
    return memcmp(data, SYMLINK_HEADER, size) == 0;
}

static const char *
read_sympath(const CapConfig *config, char *sympath, uint32_t sympathsz, const char *path) {
    FILE *fin = fopen(path, "r");
    if (!fin) {
        return NULL;
    }

    uint32_t symheaderlen = strlen(SYMLINK_HEADER);
    char line[FILE_NPATH + symheaderlen + 1];
    int32_t linelen = file_getline(line, sizeof line, fin);
    if (linelen == EOF) {
        fclose(fin);
        return NULL;
    }
    if (fclose(fin) < 0) {
        return NULL;
    }

    if (!is_contain_header(line, linelen)) {
        return NULL;
    }

    const char *p = strchr(line, ':');
    if (!p) {
        return NULL;
    }
    ++p;
    for (; *p == ' '; ++p) ;

    char cappath[FILE_NPATH];
    for (int i = 0; i < sizeof(cappath)-1 && *p; ++i, ++p) {
        cappath[i] = *p;
        cappath[i+1] = '\0';
    }

    // origin is home path (symlink path is always absolute path)
    const char *org = config->home_path;
    if (!PadFile_Solvefmt(sympath, sympathsz, "%s/%s", org, cappath)) {
        return NULL;
    }

    return sympath;
}

static void
fix_path_seps(char *dst, uint32_t dstsz, const char *src) {
#ifdef CAP__WINDOWS
    char replace_sep = '/';
#else
    char replace_sep = '\\';
#endif
    char *dp = dst;
    char *dpend = dst + dstsz;
    const char *p = src;
    for (; *p && dp < dpend; ++p, ++dp) {
        char ch = *p;
        if (ch == replace_sep) {
            ch = FILE_SEP;
        }
        *dp = ch;
    }
    *dp = '\0';
}

static const char *
find_path_head(const char *path) {
#ifdef CAP__WINDOWS
    const char *p = skip_drive_letter(path);
    if (!p) {
        return path;
    }
    return p;
#else
    return path;
#endif
}

static char *
__CapSymlink_FollowPath(const CapConfig *config, char *dst, uint32_t dstsz, const char *abspath, int dep) {
    if (dep >= 8) {
        return NULL;
    }

    char normpath[FILE_NPATH];
    fix_path_seps(normpath, sizeof normpath, abspath);

    const char *p = find_path_head(normpath);
    if (!p) {
        return NULL;
    }

    char **toks = cstr_split_ignore_empty(p, FILE_SEP);
    if (!toks) {
        return NULL;
    }

    char **save_toks = NULL;
    char sympath[FILE_NPATH];

    string_t *path = PadStr_New();
#ifdef CAP__WINDOWS
    // append drive letter
    str_pushb(path, normpath[0]);
    PadStr_App(path, ":\\");
#endif
    for (char **toksp = toks; *toksp; ++toksp) {
        str_pushb(path, FILE_SEP);
        PadStr_App(path, *toksp);
        // printf("path[%s] toksp[%s]\n", PadStr_Getc(path), *toksp);
        if (PadFile_IsDir(PadStr_Getc(path))) {
            continue;
        }

        sympath[0] = '\0';
        if (!read_sympath(config, sympath, sizeof sympath, PadStr_Getc(path))) {
            continue;
        }
        // printf("sympath[%s]\n", sympath);

        ++toksp;
        save_toks = toksp;
        break;
    }

    if (!save_toks) {
        if (!PadFile_Solve(dst, dstsz, normpath)) {
            return NULL;
        }
        goto done;
    }

    str_set(path, sympath);
    for (char **toksp = save_toks; *toksp; ++toksp) {
        str_pushb(path, FILE_SEP);
        PadStr_App(path, *toksp);
    }

    if (!__CapSymlink_FollowPath(config, dst, dstsz, PadStr_Getc(path), dep+1)) {
        goto fail;
    }

#define cleanup() { \
        for (char **toksp = toks; *toksp; ++toksp) { \
            free(*toksp); \
        } \
        free(toks); \
        PadStr_Del(path); \
    }

done:
    cleanup();
    return dst;
fail:
    cleanup();
    return NULL;
}

char *
CapSymlink_FollowPath(const CapConfig *config, char *dst, uint32_t dstsz, const char *abspath) {
    if (!dst || !dstsz || !abspath) {
        return NULL;
    }

    dst[0] = '\0';
    if (!__CapSymlink_FollowPath(config, dst, dstsz, abspath, 0)) {
        return NULL;
    }

    return dst;
}

static cstring_array_t *
split_ignore_empty(const char *p, char sep) {
    char **toks = cstr_split_ignore_empty(p, FILE_SEP);
    if (!toks) {
        return NULL;
    }

    cstring_array_t *arr = cstrarr_new();

    for (char **toksp = toks; *toksp; ++toksp) {
        cstrarr_move(arr, *toksp);
    }

    free(toks);
    return arr;
}

char *
symlink_norm_path(const CapConfig *config, char *dst, uint32_t dstsz, const char *drtpath) {
    if (!config || !dst || !dstsz || !drtpath) {
        return NULL;
    }

    char cleanpath[FILE_NPATH];
    fix_path_seps(cleanpath, sizeof cleanpath, drtpath);
    const char *pathhead = find_path_head(cleanpath);
    if (!pathhead) {
        return NULL;
    }

#ifdef CAP__WINDOWS
    const char *hasdriveletter = strchr(drtpath, ':');
#endif

    // save tokens from srctoks to dsttoks by ".." token
    cstring_array_t *srctoks = split_ignore_empty(pathhead, FILE_SEP);
    cstring_array_t *dsttoks = cstrarr_new();

    for (int32_t i = 0; i < cstrarr_len(srctoks); ++i) {
        const char *tok = cstrarr_getc(srctoks, i);
        if (PadCStr_Eq(tok, "..")) {
            char *el = cstrarr_pop_move(dsttoks);
            free(el);
        } else {
            cstrarr_push(dsttoks, tok);
        }
    }

    // save normalized path by tokens
    dst[0] = '\0';

#ifdef CAP__WINDOWS
    // append drive letter of Windows
    if (hasdriveletter) {
        PadCStr_App_fmt(dst, dstsz, "%c:", drtpath[0]);
    }
#endif

    if (pathhead[0] == FILE_SEP) {
        PadCStr_App_fmt(dst, dstsz, "%c", FILE_SEP);
    }

    for (int32_t i = 0; i < cstrarr_len(dsttoks)-1; ++i) {
        const char *tok = cstrarr_getc(dsttoks, i);
        PadCStr_App(dst, dstsz, tok);
        PadCStr_App_fmt(dst, dstsz, "%c", FILE_SEP);
    }
    if (cstrarr_len(dsttoks)) {
        const char *tok = cstrarr_getc(dsttoks, cstrarr_len(dsttoks)-1);
        PadCStr_App(dst, dstsz, tok);
    }

    cstrarr_del(srctoks);
    cstrarr_del(dsttoks);

    return dst;
}

bool
CapSymlink_IsLinkFile(const char *path) {
    char line[FILE_NPATH + 100];
    if (!PadFile_ReadLine(line, sizeof line, path)) {
        return false;
    }

    return is_contain_header(line, strlen(line));
}
