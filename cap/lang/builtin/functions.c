#include <cap/lang/builtin/functions.h>

#define push_error(fmt, ...) \
    Pad_PushBackErrNode(ref_ast->error_stack, fargs->ref_node, fmt, ##__VA_ARGS__)

static const CapConfig *_cap_config;

static PadObj *
blt_exec(PadBltFuncArgs *fargs) {
    PadAST *ref_ast = fargs->ref_ast;
    assert(ref_ast);
    PadObj *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == PAD_OBJ_TYPE__ARRAY);

    PadObjAry *args = actual_args->objarr;

    if (PadObjAry_Len(args) != 1) {
        push_error("invalid arguments length of builtin exec function");
        return NULL;
    }

    PadObj *cmdlineobj = PadObjAry_Get(args, 0);
    PadStr *cmdline = Pad_ObjToString(ref_ast->error_stack, fargs->ref_node, cmdlineobj);
    if (!cmdline) {
        return NULL;
    }

    PadCStrAry *strarr = PadCStrAry_New();
    PadCStrAry_PushBack(strarr, "exec");
    PadCStrAry_PushBack(strarr, PadStr_Getc(cmdline));
    int argc = PadCStrAry_Len(strarr);
    char **argv = PadCStrAry_EscDel(strarr);

    CapExecCmd *execcmd = CapExecCmd_New(_cap_config, argc, argv);
    int result = CapExecCmd_Run(execcmd);
    CapExecCmd_Del(execcmd);

    Pad_FreeArgv(argc, argv);
    return PadObj_NewInt(ref_ast->ref_gc, result);
}

PadBltFuncInfo blt_func_infos[] = {
    {"exec", blt_exec},
    {0},
};

PadBltFuncInfo *
CapBltFuncs_GetBltFuncInfos(void) {
    return blt_func_infos;
}

void
CapBltFuncs_SetCapConfig(const CapConfig *config) {
    _cap_config = config;
}
