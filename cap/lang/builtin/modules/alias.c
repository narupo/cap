#include <cap/lang/builtin/modules/alias.h>

#define push_error(fmt, ...) \
    Pad_PushBackErrNode(ref_ast->error_stack, fargs->ref_node, fmt, ##__VA_ARGS__)

static PadVoidDict *_alias_info_map;

static void
_construct(void) {
    if (_alias_info_map == NULL) {
        _alias_info_map = PadVoidDict_New();
    }
}

static CapAliasInfo *
get_item(const PadCtx *ctx) {
    char key[PAD_VOID_DICT_ITEM__KEY_SIZE];
    snprintf(key, sizeof key, "%p", ctx);

    const PadVoidDictItem *i = PadVoidDict_Getc(_alias_info_map, key);
    if (i == NULL) {
        return NULL;
    }

    return i->value;
}

static PadVoidDict *
set_item(const PadCtx *ctx, CapAliasInfo *alinfo) {
    char key[PAD_VOID_DICT_ITEM__KEY_SIZE];
    snprintf(key, sizeof key, "%p", ctx);

    return PadVoidDict_Move(_alias_info_map, key, alinfo);
}

const CapAliasInfo *
CapBltAliasMod_GetAliasInfo(const PadCtx *ctx) {
    _construct();
    return get_item(ctx);
}

static PadObj *
builtin_alias_set(PadBltFuncArgs *fargs) {
    _construct();
    PadAST *ref_ast = fargs->ref_ast;
    assert(ref_ast);
    PadObj *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == PAD_OBJ_TYPE__ARRAY);

    PadObjAry *args = actual_args->objarr;

    if (PadObjAry_Len(args) < 2) {
        push_error("can't invoke alias.set. too few arguments");
        return NULL;
    } else if (PadObjAry_Len(args) >= 4) {
        push_error("can't invoke alias.set. too many arguments");
        return NULL;
    }

    const PadObj *keyobj = PadObjAry_Getc(args, 0);
    if (keyobj->type != PAD_OBJ_TYPE__UNICODE) {
        push_error("can't invoke alias.set. key is not string");
        return NULL;
    }

    const PadObj *valobj = PadObjAry_Getc(args, 1);
    if (valobj->type != PAD_OBJ_TYPE__UNICODE) {
        push_error("can't invoke alias.set. value is not string");
        return NULL;
    }

    const PadObj *descobj = NULL;
    if (PadObjAry_Len(args) == 3) {
        descobj = PadObjAry_Getc(args, 2);
        if (descobj->type != PAD_OBJ_TYPE__UNICODE) {
            push_error("can't invoke alias.set. description is not unicode");
            return NULL;
        }
    }

    const char *key = PadUni_GetcMB(keyobj->unicode);
    const char *val = PadUni_GetcMB(valobj->unicode);
    const char *desc = descobj ? PadUni_GetcMB(descobj->unicode) : NULL;

    CapAliasInfo *alinfo = get_item(ref_ast->ref_context);
    if (alinfo == NULL) {
        alinfo = CapAliasInfo_New();
    }

    CapAliasInfo_SetValue(alinfo, key, val);    
    if (desc) {
        CapAliasInfo_SetDesc(alinfo, key, desc);    
    }

    set_item(ref_ast->ref_context, alinfo);

    return PadObj_NewNil(ref_ast->ref_gc);
}

static PadBltFuncInfo
builtin_func_infos[] = {
    {"set", builtin_alias_set},
    {0},
};

PadObj *
CapBltAliasMod_NewMod(const PadConfig *ref_config, PadGC *ref_gc) {
    PadTkr *tkr = PadTkr_New(PadMem_Move(PadTkrOpt_New()));
    PadCtx *ctx = PadCtx_New(ref_gc);
    PadAST *ast = PadAST_New(ref_config);
    ast->ref_context = ctx;

    PadBltFuncInfoAry *info_ary = PadBltFuncInfoAry_New();
    PadBltFuncInfoAry_ExtendBackAry(info_ary, builtin_func_infos);
    _construct();

    return PadObj_NewModBy(
        ref_gc,
        "alias",
        NULL,
        NULL,
        PadMem_Move(tkr),
        PadMem_Move(ast),
        PadMem_Move(ctx),
        PadMem_Move(info_ary)
    );
}
