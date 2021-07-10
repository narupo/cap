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
    CapKit *kit;
    char error_detail[ERR_DETAIL_SIZE];
};

void
CapAliasMgr_Del(CapAliasMgr *self) {
    if (!self) {
        return;
    }

    CapKit_Del(self->kit);
    Pad_SafeFree(self);
}

CapAliasMgr *
CapAliasMgr_New(const CapConfig *config) {
    CapAliasMgr *self = PadMem_Calloc(1, sizeof(*self));
    if (self == NULL) {
        goto error;
    }

    self->config = config;
    self->kit = CapKit_New(config);
    if (self->kit == NULL) {
        goto error;
    }

    return self;
error:
    CapAliasMgr_Del(self);
    return NULL;
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

    if (!CapKit_CompileFromStrArgs(
        self->kit,
        path,
        src,
        0,
        NULL
    )) {
        set_err(self, "failed to compile");
        goto error;
    }

    Pad_SafeFree(src);
    return self;
error:
    Pad_SafeFree(src);
    return NULL;
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

    const CapAliasInfo *alinfo = CapBltAliasMod_GetAliasInfo();
    if (alinfo == NULL) {
        return NULL;
    } 

    // find alias value by key
    const char *value = CapAliasInfo_GetcValue(alinfo, key);
    if (value == NULL) {
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
    CapKit_Clear(self->kit);
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

const CapAliasInfo *
CapAliasMgr_GetcAliasInfo(const CapAliasMgr *self) {
    return CapBltAliasMod_GetAliasInfo();
}

const PadCtx *
CapAliasMgr_GetcCtx(const CapAliasMgr *self) {
    return CapKit_GetRefCtx(self->kit);
}