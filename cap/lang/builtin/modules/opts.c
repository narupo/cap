#include <cap/lang/builtin/modules/opts.h>

#define push_error(fmt, ...) \
    Pad_PushBackErrNode(ref_ast->error_stack, fargs->ref_node, fmt, ##__VA_ARGS__)

/* optsを機能させるには複数のコンテキストに対応させる必要がある
   つまりコンテキストのアドレスごとにoptsを用意する必要がある */

static PadVoidDict *_opts_map;

bool
CapBltOptsMod_MoveOpts(void *pkey, CapOpts *move_opts) {
    if (!pkey || !move_opts) {
        return false;
    }

    if (_opts_map == NULL) {
        _opts_map = PadVoidDict_New();
        if (_opts_map == NULL) {
            return false;
        }
    }

    char key[PAD_VOID_DICT_ITEM__KEY_SIZE];
    snprintf(key, sizeof key, "%p", pkey);

    return !!PadVoidDict_Move(_opts_map, key, PadMem_Move(move_opts));
}

static CapOpts *
get_item(void *pkey) {
    if (_opts_map == NULL) {
        return NULL;
    }

    char key[PAD_VOID_DICT_ITEM__KEY_SIZE];
    snprintf(key, sizeof key, "%p", pkey);

    const PadVoidDictItem *item = PadVoidDict_Getc(_opts_map, key);
    if (item == NULL) {
        return NULL;
    }

    CapOpts *opts = item->value;
    return opts;
}

static PadObj *
builtin_opts_get(PadBltFuncArgs *fargs) {
    PadAST *ref_ast = fargs->ref_ast;
    assert(ref_ast);
    PadObj *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == PAD_OBJ_TYPE__ARRAY);

    PadObjAry *args = actual_args->objarr;

    CapOpts *opts = get_item(ref_ast->ref_context);
    if (opts == NULL) {
        push_error("nothing opts");
        return NULL;
    }

    if (PadObjAry_Len(args) != 1) {
        push_error("can't invoke opts.get. need one argument");
        return NULL;
    }

    const PadObj *objname = PadObjAry_Getc(args, 0);
    assert(objname);

    if (objname->type != PAD_OBJ_TYPE__UNICODE) {
        push_error("can't invoke opts.get. argument is not string");
        return NULL;
    }

    PadUni *optname = objname->unicode;
    const char *optval = CapOpts_Getc(opts, PadUni_GetcMB(optname));
    if (!optval) {
        return PadObj_NewNil(ref_ast->ref_gc);
    }

    return PadObj_NewUnicodeCStr(ref_ast->ref_gc, optval);
}

static PadObj *
builtin_opts_has(PadBltFuncArgs *fargs) {
    PadAST *ref_ast = fargs->ref_ast;
    assert(ref_ast);
    PadObj *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == PAD_OBJ_TYPE__ARRAY);

    PadObjAry *args = actual_args->objarr;

    CapOpts *opts = get_item(ref_ast->ref_context);
    if (opts == NULL) {
        push_error("nothing opts");
        return NULL;
    }

    if (PadObjAry_Len(args) != 1) {
        push_error("can't invoke opts.get. need one argument");
        return NULL;
    }

    const PadObj *objname = PadObjAry_Getc(args, 0);
    assert(objname);

    if (objname->type != PAD_OBJ_TYPE__UNICODE) {
        push_error("can't invoke opts.get. argument is not string");
        return NULL;
    }

    PadUni *optname = objname->unicode;
    bool has = CapOpts_Has(opts, PadUni_GetcMB(optname));
    return PadObj_NewBool(ref_ast->ref_gc, has);
}

static PadObj *
builtin_opts_args(PadBltFuncArgs *fargs) {
    PadAST *ref_ast = fargs->ref_ast;
    assert(ref_ast);
    PadObj *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == PAD_OBJ_TYPE__ARRAY);
    PadObjAry *args = actual_args->objarr;

    CapOpts *opts = get_item(ref_ast->ref_context);
    if (opts == NULL) {
        push_error("nothing opts");
        return NULL;
    }

    if (PadObjAry_Len(args) != 1) {
        push_error("can't invoke opts.args. need one argument");
        return NULL;
    }

    const PadObj *arg = PadObjAry_Getc(args, 0);
    if (arg->type != PAD_OBJ_TYPE__INT) {
        push_error("invalid argument type. argument is not int");
        return NULL;
    }

    int32_t idx = arg->lvalue;
    const char *value = CapOpts_GetcArgs(opts, idx);
    if (!value) {
        return PadObj_NewNil(ref_ast->ref_gc);
    }

    return PadObj_NewUnicodeCStr(ref_ast->ref_gc, value);
}

static PadBltFuncInfo
builtin_func_infos[] = {
    {"get", builtin_opts_get},
    {"has", builtin_opts_has},
    {"args", builtin_opts_args},
    {0},
};

PadObj *
CapBltOptsMod_NewMod(const PadConfig *ref_config, PadGC *ref_gc) {
    PadTkr *tkr = PadTkr_New(PadMem_Move(PadTkrOpt_New()));
    PadAST *ast = PadAST_New(ref_config);
    PadCtx *ctx = PadCtx_New(ref_gc);
    ast->ref_context = ctx;

    PadBltFuncInfoAry *info_ary = PadBltFuncInfoAry_New();
    PadBltFuncInfoAry_ExtendBackAry(info_ary, builtin_func_infos);

    return PadObj_NewModBy(
        ref_gc,
        "opts",
        NULL,
        NULL,
        PadMem_Move(tkr),
        PadMem_Move(ast),
        PadMem_Move(ctx),
        PadMem_Move(info_ary)
    );
}
