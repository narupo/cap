/**
 * Cap
 *
 * License: MIT
 *  Author: narupo
 *   Since: 2016
 */
#include <cap/core/util.h>

static char *
read_path_var_from_resource(const CapConfig *config, const char *rcpath) {
    char *src = PadFile_ReadCopyFromPath(rcpath);

    PadTkr *tkr = PadTkr_New(PadTkrOpt_New());
    PadAST *ast = PadAST_New(config->pad_config);
    PadGC *gc = PadGC_New();
    PadCtx *ctx = PadCtx_New(gc);
    PadOpts *opts = PadOpts_New();

    PadTkr_Parse(tkr, src);
    free(src);
    src = NULL;
    if (PadTkr_HasErrStack(tkr)) {
        PadErr_Err("%s", PadTkr_GetcFirstErrMsg(tkr));
        return NULL;
    }

    PadAST_Clear(ast);
    PadAST_MoveOpts(ast, opts);
    opts = NULL;

    PadCC_Compile(ast, PadTkr_GetToks(tkr));
    if (PadAST_HasErrs(ast)) {
        PadErr_Err("%s", PadAST_GetcFirstErrMsg(ast));
        return NULL;
    }

    PadTrv_Trav(ast, ctx);
    if (PadAST_HasErrs(ast)) {
        PadErr_Err("%s", PadAST_GetcFirstErrMsg(ast));
        return NULL;
    }

    PadTkr_Del(tkr);
    PadAST_Del(ast);

    PadObjDict *varmap = PadCtx_GetVarmapAtGlobal(ctx);
    const PadObjDictItem *item = PadObjDict_Getc(varmap, "PATH");
    if (!item) {
        PadCtx_Del(ctx);
        PadGC_Del(gc);
        return NULL;
    }

    PadCtx_PopNewlineOfStdoutBuf(ctx);
    printf("%s", PadCtx_GetcStdoutBuf(ctx));
    fflush(stdout);

    const char *s = PadUni_GetcMB(item->value->unicode);
    char *path = PadCStr_Dup(s);
    if (!path) {
        PadCtx_Del(ctx);
        PadGC_Del(gc);
        return NULL;        
    }

    PadCtx_Del(ctx);
    PadGC_Del(gc);

    return path;
}

static PadCStrAry *
split_path_var(const char *path) {
    return Pad_SplitToArray(path, ',');
}

bool
Cap_IsOutOfHome(const char *homepath, const char *argpath) {
    if (!homepath || !argpath) {
        return false;
    }

    char home[PAD_FILE__NPATH];
    char path[PAD_FILE__NPATH];

    if (!PadFile_Solve(home, sizeof home, homepath) ||
        !PadFile_Solve(path, sizeof path, argpath)) {
        return true;
    }

    PadPath_PopTailSlash(home);
    PadPath_PopTailSlash(path);

    size_t homelen = strlen(home);
    if (strncmp(home, path, homelen)) {
        return true;
    }

    return false;
}

char *
Cap_SolveCmdlineArgPath(const CapConfig *config, char *dst, int32_t dstsz, const char *caps_arg_path) {
    if (caps_arg_path[0] == ':') {
        if (!PadFile_Solve(dst, dstsz, caps_arg_path+1)) {
            return NULL;
        }
    } else {
        char tmp[PAD_FILE__NPATH*2];
        const char *org = Cap_GetOrigin(config, caps_arg_path);

        const char *path = caps_arg_path;
        if (caps_arg_path[0] == '/') {
            path = caps_arg_path + 1;
        }

        snprintf(tmp, sizeof tmp, "%s/%s", org, path);
        if (!CapSymlink_FollowPath(config, dst, dstsz, tmp)) {
            return NULL;
        }
    }

    return dst;
}

const char *
Cap_GetOrigin(const CapConfig *config, const char *cap_path) {
    if (!config || !cap_path) {
        return NULL;
    }

    if (cap_path[0] == '/') {
        return config->home_path;
    } else if (config->scope == CAP_SCOPE__LOCAL) {
        return config->cd_path;
    } else if (config->scope == CAP_SCOPE__GLOBAL) {
        return config->home_path;
    }

    PadErr_Die("impossible. invalid state in get origin");
    return NULL;
}

/**
 * Show snippet code by fname
 *
 * @param[in] *config reference to config
 * @param[in] *fname  snippet file name
 * @param[in] argc    number of arguments
 * @param[in] **argv  arguments
 *
 * @return success to true
 * @return failed to false
 */
static bool
show_snippet(const CapConfig *config, const char *fname, int argc, char **argv) {
    if (!config || !fname || !argv) {
        return false;
    }

    char path[PAD_FILE__NPATH];
    if (!PadFile_SolveFmt(path, sizeof path, "%s/%s", config->codes_dir_path, fname)) {
        PadErr_Err("failed to solve path for snippet file");
        return false;
    }

    char *content = PadFile_ReadCopyFromPath(path);
    if (!content) {
        PadErr_Err("failed to read from snippet \"%s\"", fname);
        return false;
    }

    PadErrStack *errstack = PadErrStack_New();
    char *compiled = Cap_MakeArgv(config, errstack, fname, content, argc, argv);
    if (!compiled) {
        PadErrStack_TraceSimple(errstack, stderr);
        fflush(stderr);
        free(content);
        PadErrStack_Del(errstack);
        return false;
    }

    printf("%s", compiled);
    fflush(stdout);

    free(content);
    free(compiled);

    PadErrStack_Del(errstack);
    return true;
}

int
Cap_ExecSnippet(const CapConfig *config, bool *found, int argc, char **argv, const char *name) {
    if (!config || !found || !argv || !name) {
        PadErr_Warn("util:Cap_ExecSnippet: invalid arguments");
        return 1;
    }

    PadDir *dir = PadDir_Open(config->codes_dir_path);
    if (!dir) {
        PadErr_Err("failed to open directory \"%s\"", config->codes_dir_path);
        return 1;
    }

    *found = false;
    for (PadDirNode *node; (node = PadDir_Read(dir)); ) {
        const char *fname = PadDirNode_Name(node);
        if (Pad_IsDotFile(fname)) {
            continue;
        }

        if (PadCStr_Eq(fname, name)) {
            *found = true;
            if (!show_snippet(config, fname, argc, argv)) {
                PadDir_Close(dir);
                return 1;
            }
        }
    }

    PadDir_Close(dir);
    return *found ? 0 : -1;
}

int
Cap_ExecRun(const CapConfig *config, int argc, char *argv[]) {
    CapRunCmd *cmd = CapRunCmd_New(config, argc, argv);
    if (!cmd) {
        return 1;
    }

    int result = CapRunCmd_Run(cmd);
    CapRunCmd_Del(cmd);

    return result;
}

static int
exec_prog_by_dirname(const CapConfig *config, bool *found, int cmd_argc, char *cmd_argv[], const char *cap_dirname) {
    *found = false;

    const char *cmdname = cmd_argv[0];
    char cap_fpath[PAD_FILE__NPATH];
    snprintf(cap_fpath, sizeof cap_fpath, "%s/%s", cap_dirname, cmdname);

    const char *org = Cap_GetOrigin(config, cap_fpath);
    char real_path[PAD_FILE__NPATH];
    if (!PadFile_SolveFmt(real_path, sizeof real_path, "%s/%s", org, cap_fpath)) {
        PadErr_Err("failed to solve in execute program in directory");
        return 1;
    }
    if (!PadFile_IsExists(real_path)) {
        return 1;
    }
    *found = true;

    PadCStrAry *args = PadCStrAry_New();
    PadCStrAry_PushBack(args, "run");
    PadCStrAry_PushBack(args, cap_fpath);
    for (int32_t i = 1; i < cmd_argc; ++i) {
        PadCStrAry_PushBack(args, cmd_argv[i]);
    }

    int argc = PadCStrAry_Len(args);
    char **argv = PadCStrAry_EscDel(args);
    return Cap_ExecRun(config, argc, argv);
}

static int
exec_prog_by_caprc(const CapConfig *config, bool *found, int cmd_argc, char *cmd_argv[], const char *org) {
    *found = false;
    char rcpath[PAD_FILE__NPATH];

    if (!PadFile_SolveFmt(rcpath, sizeof rcpath, "%s/.caprc", org)) {
        return 1;
    }

    if (!PadFile_IsExists(rcpath)) {
        return 1;
    }

    char *path = read_path_var_from_resource(config, rcpath);
    if (!path) {
        return 1;
    }
    
    PadCStrAry *dirs = split_path_var(path);
    free(path);
    if (!PadCStrAry_Len(dirs)) {
        return 1;
    }

    for (int32_t i = 0; i < PadCStrAry_Len(dirs); ++i) {
        const char *cap_dirname = PadCStrAry_Getc(dirs, i);

        *found = false;
        int result = exec_prog_by_dirname(
            config,
            found,
            cmd_argc,
            cmd_argv,
            cap_dirname
        );
        if (*found) {
            return result;
        }
    }

    return 1;
}

int
Cap_ExecProg(const CapConfig *config, bool *found, int cmd_argc, char *cmd_argv[], const char *cmdname) {
    if (cmdname[0] == '.') {
        *found = false;
        return 1;
    }

    int result;

    *found = false;
    result = exec_prog_by_caprc(config, found, cmd_argc, cmd_argv, config->cd_path);
    if (*found) {
        return result;
    }

    *found = false;
    result = exec_prog_by_caprc(config, found, cmd_argc, cmd_argv, config->home_path);
    if (*found) {
        return result;
    }

    return 1;
}

char *
Cap_MakeArgv(
    const CapConfig *config,
    PadErrStack *errstack,
    const char *program_filename,
    const char *src,
    int argc,
    char *argv[]
) {
    CapKit *kit = CapKit_New(config);
    if (kit == NULL) {
        PadErrStack_Add(errstack, "failed to create kit");
        goto error;
    }

    if (CapKit_CompileFromStrArgs(
        kit,
        program_filename,
        src,
        argc,
        argv
    ) == NULL) {
        PadErrStack_Add(errstack, "failed to compile");
        goto error;
    }

    const char *stdout_buf = CapKit_GetcStdoutBuf(kit);
    char *maked = PadCStr_Dup(stdout_buf);
    if (maked == NULL) {
        PadErrStack_Add(errstack, "failed to dup");
        goto error;
    }

    CapKit_Del(kit);
    return maked;
error:
    CapKit_Del(kit);
    return NULL;
}
