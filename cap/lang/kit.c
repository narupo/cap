#include <cap/lang/kit.h>

struct CapKit {
    const CapConfig *config;
    PadKit *kit;
    PadErrStack *errstack;
};

void
CapKit_Del(CapKit *self) {
    if (self == NULL) {
        return;
    }

    PadKit_Del(self->kit);
    PadErrStack_Del(self->errstack);
    free(self);
}

CapKit *
CapKit_New(const CapConfig *config) {
    if (config == NULL) {
        return NULL;
    }

    CapKit *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        goto error;
    }

    self->config = config;
    self->kit = PadKit_New(config->pad_config);
    if (!self->kit) {
        goto error;
    }

    self->errstack = PadErrStack_New();
    if (!self->errstack) {
        goto error;
    }

    return self;
error:
    CapKit_Del(self);
    return NULL;
}

CapKit *
CapKit_CompileFromStrArgs(
    CapKit *self,  // required
    const char *prog_fname,  // optional
    const char *src,  // required
    int argc,  // optional
    char **argv  // optional
) {
    PadAST *ref_ast = PadKit_GetRefAST(self->kit);
    PadGC *ref_gc = PadKit_GetRefGC(self->kit);

    // set fix-path function at importer
    CapImporter_SetCapConfig(self->config);
    PadKit_SetImporterFixPathFunc(self->kit, CapImporter_FixPath);

    // parse pad-options
    PadOpts *opts = PadOpts_New();
    if (!PadOpts_Parse(opts, argc, argv)) {
        Pad_PushErr("failed to parse options");
        goto error;
    }

    // set pad-options
    PadAST_MoveOpts(ref_ast, PadMem_Move(opts));

    // install built-in functions
    CapBltFuncs_SetCapConfig(self->config);
    PadKit_SetBltFuncInfos(self->kit, CapBltFuncs_GetBltFuncInfos());

    // install alias module
    PadObj *alias_mod = CapBltAliasMod_NewMod(self->config->pad_config, ref_gc);
    PadObj_IncRef(alias_mod);
    PadKit_MoveBltMod(self->kit, PadMem_Move(alias_mod));

    // compile
    if (!PadKit_CompileFromStrArgs(self->kit, prog_fname, src, argc, argv)) {
        const PadErrStack *es = PadKit_GetcErrStack(self->kit);
        PadErrStack_ExtendBackOther(self->errstack, es);
        goto error;
    }

    return self;
error:
    return NULL;
}

const char *
CapKit_GetcStdoutBuf(const CapKit *self) {
    if (self == NULL) {
        return NULL;
    }

    return PadKit_GetcStdoutBuf(self->kit);
}

PadCtx *
CapKit_GetRefCtx(const CapKit *self) {
    if (self == NULL) {
        return NULL;
    }

    return PadKit_GetRefCtx(self->kit);
}

void
CapKit_Clear(CapKit *self) {
    if (self == NULL) {
        return;
    }

    PadKit_ClearCtx(self->kit);
}

const PadErrStack *
CapKit_GetcErrStack(CapKit *self) {
    if (self == NULL) {
        return NULL;
    }

    return self->errstack;
}
