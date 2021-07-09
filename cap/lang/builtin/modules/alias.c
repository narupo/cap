#include <pad/lang/builtin/modules/alias.h>

#define push_error(fmt, ...) \
    Pad_PushBackErrNode(ref_ast->error_stack, fargs->ref_node, fmt, ##__VA_ARGS__)

static CapAliasInfo *_alias_info;

const CapAliasInfo *
CapBltAliasMod_GetAliasInfo(void) {
    return _alias_info;
}

static PadObj *
builtin_alias_set(PadBltFuncArgs *fargs) {
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

    CapAliasInfo_SetValue(_alias_info, key, val);    
    CapAliasInfo_SetDesc(_alias_info, key, desc);    

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

    if (_alias_info == NULL) {
        _alias_info = CapAliasInfo_New();
        // TODO delete
    }

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
