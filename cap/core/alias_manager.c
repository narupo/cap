#include <cap/core/alias_manager.h>

/**
 * Numbers
 */
enum {
    ERR_DETAIL_SIZE = 1024,
};

/**
 * Structure of alias_manager
 */
struct CapAliasMgr {
    const CapConfig *config;
    PadTkr *tkr;
    PadAST *ast;
    PadGC *gc;
    PadCtx *context;
    char error_detail[ERR_DETAIL_SIZE];
};

void
CapAliasMgr_Del(CapAliasMgr *self) {
    if (!self) {
        return;
    }

    PadAST_Del(self->ast);
    PadTkr_Del(self->tkr);
    PadCtx_Del(self->context);
    PadGC_Del(self->gc);
    Pad_SafeFree(self);
}

CapAliasMgr *
CapAliasMgr_New(const CapConfig *config) {
    CapAliasMgr *self = PadMem_Calloc(1, sizeof(*self));
    if (self == NULL) {
        return NULL;
    }

    self->config = config;

    PadTkrOpt *opt = PadTkrOpt_New();
    self->tkr = PadTkr_New(opt);
    self->ast = PadAST_New(config);
    self->gc = PadGC_New();
    self->context = PadCtx_New(self->gc);

    return self;
}

static void
set_err(CapAliasMgr *self, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(self->error_detail, sizeof self->error_detail, fmt, ap);
    va_end(ap);
}

static char *
create_resource_path(CapAliasMgr *self, char *dst, size_t dstsz, int scope) {
    const char *org = NULL;

    if (scope == CAP_SCOPE__LOCAL) {
        org = self->config->cd_path;
    } else if (scope == CAP_SCOPE__GLOBAL) {
        org = self->config->home_path;
    } else {
        set_err(self, "invalid scope");
        return NULL;
    }

    char drtpath[PAD_FILE__NPATH*2];
    snprintf(drtpath, sizeof drtpath, "%s/.caprc", org);

    if (!CapSymlink_FollowPath(self->config, dst, dstsz, drtpath)) {
        set_err(self, "failed to follow path of resource file");
        return NULL;
    }

    return dst;
}

CapAliasMgr *
CapAliasMgr_LoadPath(CapAliasMgr *self, const char *path) {
    char *src = PadFile_ReadCopyFromPath(path);
    if (!src) {
        set_err(self, "failed to read content from file \"%s\"", path);
        return NULL;
    }

    CapAliasMgr *ret = self;

    PadTkr_Parse(self->tkr, src);
    if (PadTkr_HasErrStack(self->tkr)) {
        set_err(self, PadTkr_GetcFirstErrMsg(self->tkr));
        ret = NULL;
        goto fail;
    }

    PadAST_Clear(self->ast);
    PadCC_Compile(self->ast, PadTkr_GetToks(self->tkr));
    if (PadAST_HasErrs(self->ast)) {
        set_err(self, PadAST_GetcFirstErrMsg(self->ast));
        ret = NULL;
        goto fail;
    }

    PadCtx_Clear(self->context);
    PadTrv_Trav(self->ast, self->context);
    if (PadAST_HasErrs(self->ast)) {
        set_err(self, PadAST_GetcFirstErrMsg(self->ast));
        ret = NULL;
        goto fail;
    }

fail:
    Pad_SafeFree(src);
    return ret;
}

CapAliasMgr *
CapAliasMgr_LoadAliasList(CapAliasMgr *self, int scope) {
    char path[PAD_FILE__NPATH];
    if (!create_resource_path(self, path, sizeof path, scope)) {
        set_err(self, "failed to create path by scope %d", scope);
        return NULL;
    }
    if (!PadFile_IsExists(path)) {
        // don't write error detail
        return NULL;
    }

    return CapAliasMgr_LoadPath(self, path);
}

CapAliasMgr *
CapAliasMgr_FindAliasValue(CapAliasMgr *self, char *dst, uint32_t dstsz, const char *key, int scope) {
    if (!CapAliasMgr_LoadAliasList(self, scope)) {
        return NULL;
    }

    // find alias value by key
    const char *value = PadCtx_GetAliasValue(self->context, key);
    if (!value) {
        return NULL;
    }

    snprintf(dst, dstsz, "%s", value);
    return self;
}

bool
CapAliasMgr_HasErr(const CapAliasMgr *self) {
    return self->error_detail[0] != '\0';
}

void
CapAliasMgr_Clear(CapAliasMgr *self) {
    PadCtx_Clear(self->context);
    CapAliasMgr_ClearError(self);
}

void
CapAliasMgr_ClearError(CapAliasMgr *self) {
    self->error_detail[0] = '\0';
}

const char *
CapAliasMgr_GetErrDetail(const CapAliasMgr *self) {
    return self->error_detail;
}

const PadAliasInfo *
almgr_getc_alinfo(const CapAliasMgr *self) {
    return PadCtx_GetcAliasInfo(self->context);
}

const PadCtx *
CapAliasMgr_GetcContext(const CapAliasMgr *self) {
    return self->context;
}