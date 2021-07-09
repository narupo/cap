#include <core/alias_manager.h>

/**
 * Numbers
 */
enum {
    ERR_DETAIL_SIZE = 1024,
};

/**
 * Structure of alias_manager
 */
struct alias_manager {
    const CapConfig *config;
    tokenizer_t *tkr;
    ast_t *ast;
    gc_t *gc;
    PadCtx *context;
    char error_detail[ERR_DETAIL_SIZE];
};

void
almgr_del(CapAliasMgr *self) {
    if (!self) {
        return;
    }

    ast_del(self->ast);
    tkr_del(self->tkr);
    ctx_del(self->context);
    gc_del(self->gc);
    free(self);
}

CapAliasMgr *
CapAliasMgr_New(const CapConfig *config) {
    CapAliasMgr *self = PadMem_ECalloc(1, sizeof(*self));

    self->config = config;

    tokenizer_option_t *opt = tkropt_new();
    self->tkr = tkr_new(opt);
    self->ast = ast_new(config);
    self->gc = gc_new();
    self->context = ctx_new(self->gc);

    return self;
}

static void
almgr_set_error_detail(CapAliasMgr *self, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(self->error_detail, sizeof self->error_detail, fmt, ap);
    va_end(ap);
}

static char *
almgr_create_resource_path(CapAliasMgr *self, char *dst, size_t dstsz, int scope) {
    const char *org = NULL;

    if (scope == CAP_SCOPE_LOCAL) {
        org = self->config->cd_path;
    } else if (scope == CAP_SCOPE_GLOBAL) {
        org = self->config->home_path;
    } else {
        almgr_set_error_detail(self, "invalid scope");
        return NULL;
    }

    char drtpath[FILE_NPATH*2];
    snprintf(drtpath, sizeof drtpath, "%s/.caprc", org);

    if (!CapSymlink_FollowPath(self->config, dst, dstsz, drtpath)) {
        almgr_set_error_detail(self, "failed to follow path of resource file");
        return NULL;
    }

    return dst;
}

CapAliasMgr *
CapAliasMgr_LoadPath(CapAliasMgr *self, const char *path) {
    char *src = file_readcp_from_path(path);
    if (!src) {
        almgr_set_error_detail(self, "failed to read content from file \"%s\"", path);
        return NULL;
    }

    CapAliasMgr *ret = self;

    tkr_parse(self->tkr, src);
    if (tkr_has_error_stack(self->tkr)) {
        almgr_set_error_detail(self, tkr_getc_first_error_message(self->tkr));
        ret = NULL;
        goto fail;
    }

    ast_clear(self->ast);
    cc_compile(self->ast, tkr_get_tokens(self->tkr));
    if (ast_has_errors(self->ast)) {
        almgr_set_error_detail(self, ast_getc_first_error_message(self->ast));
        ret = NULL;
        goto fail;
    }

    ctx_clear(self->context);
    trv_traverse(self->ast, self->context);
    if (ast_has_errors(self->ast)) {
        almgr_set_error_detail(self, ast_getc_first_error_message(self->ast));
        ret = NULL;
        goto fail;
    }

fail:
    free(src);
    return ret;
}

CapAliasMgr *
almgr_load_alias_list(CapAliasMgr *self, int scope) {
    char path[FILE_NPATH];
    if (!almgr_create_resource_path(self, path, sizeof path, scope)) {
        almgr_set_error_detail(self, "failed to create path by scope %d", scope);
        return NULL;
    }
    if (!PadFile_IsExists(path)) {
        // don't write error detail
        return NULL;
    }

    return CapAliasMgr_LoadPath(self, path);
}

CapAliasMgr *
almgr_find_alias_value(CapAliasMgr *self, char *dst, uint32_t dstsz, const char *key, int scope) {
    if (!almgr_load_alias_list(self, scope)) {
        return NULL;
    }

    // find alias value by key
    const char *value = ctx_get_alias_value(self->context, key);
    if (!value) {
        return NULL;
    }

    snprintf(dst, dstsz, "%s", value);
    return self;
}

bool
almgr_has_error(const CapAliasMgr *self) {
    return self->error_detail[0] != '\0';
}

void
CapAliasMgr_Clear(CapAliasMgr *self) {
    ctx_clear(self->context);
    CapAliasMgr_Clear_error(self);
}

void
CapAliasMgr_Clear_error(CapAliasMgr *self) {
    self->error_detail[0] = '\0';
}

const char *
almgr_get_error_detail(const CapAliasMgr *self) {
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