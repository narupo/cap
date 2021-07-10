#include <cap/lang/kit.h>

struct CapKit {
    const CapConfig *config;
    PadKit *kit;
};

void
CapKit_Del(CapKit *self) {
    if (self == NULL) {
        return;
    }

    PadKit_Del(self->kit);
    free(self);
}

CapKit *
CapKit_New(const CapConfig *config) {
    if (config == NULL) {
        return NULL;
    }

    CapKit *self = PadMem_Calloc(1, sizeof(*self));
    if (self == NULL) {
        goto error;
    }

    self->config = config;
    self->kit = PadKit_New(config->pad_config);
    assert(self->kit);
    if (self->kit == NULL) {
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
    PadGC *ref_gc = PadKit_GetRefGC(self->kit);
    PadCtx *ref_ctx = PadKit_GetRefCtx(self->kit);

    // set fix-path function at importer
    CapImporter_SetCapConfig(self->config);
    PadKit_SetImporterFixPathFunc(self->kit, CapImporter_FixPath);

    // parse options
    CapOpts *opts = CapOpts_New();
    if (!CapOpts_Parse(opts, argc, argv)) {
        CapOpts_Del(opts);
        goto error;
    }

    // install built-in functions
    CapBltFuncs_SetCapConfig(self->config);
    PadKit_SetBltFuncInfos(self->kit, CapBltFuncs_GetBltFuncInfos());

    // install opts module
    CapBltOptsMod_MoveOpts(ref_ctx, opts);
    PadObj *opts_mod = CapBltOptsMod_NewMod(self->config->pad_config, ref_gc);
    PadKit_MoveBltMod(self->kit, PadMem_Move(opts_mod));

    // install alias module
    PadObj *alias_mod = CapBltAliasMod_NewMod(self->config->pad_config, ref_gc);
    PadKit_MoveBltMod(self->kit, PadMem_Move(alias_mod));

    // compile
    if (!PadKit_CompileFromStrArgs(self->kit, prog_fname, src, argc, argv)) {
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