/**
 * Cap
 *
 * License: MIT
 *  Author: narupo
 *   Since: 2016
 */
#include <cap/cd/cd.h>

struct CapCdCmd {
    const CapConfig *config;
    int argc;
    char **argv;
};

void
CapCdCmd_Del(CapCdCmd *self) {
    if (!self) {
        return;
    }

    free(self);
}

CapCdCmd *
CapCdCmd_New(const CapConfig *config, int argc, char *argv[]) {
    CapCdCmd *self = PadMem_Calloc(1, sizeof(*self));
    if (self == NULL) {
        return NULL;
    }

    self->config = config;
    self->argc = argc;
    self->argv = argv;
    
    return self;
}

static bool
cd(CapCdCmd *self, const char *drtpath) {
    char normpath[PAD_FILE__NPATH];
    if (!CapSymlink_NormPath(self->config, normpath, sizeof normpath, drtpath)) {
        PadErr_Err("failed to normalize path");
        return false;
    }

    char realpath[PAD_FILE__NPATH];
    if (!CapSymlink_FollowPath(self->config, realpath, sizeof realpath, normpath)) {
        PadErr_Err("failed to follow path");
        return false;
    }

    if (Cap_IsOutOfHome(self->config->home_path, realpath)) {
        PadErr_Err("\"%s\" is out of home", normpath);
        return false;
    }

    if (!PadFile_IsDir(realpath)) {
        PadErr_Err("\"%s\" is not a directory", normpath);
        return false;
    }

    if (!PadFile_WriteLine(normpath, self->config->var_cd_path)) {
        PadErr_Err("invalid var cd path");
        return false;
    }

    return true;
}

int
CapCdCmd_Run(CapCdCmd *self) {
    if (self->argc < 2) {
        if (!cd(self, self->config->home_path)) {
            return 1;
        }
        return 0;
    }

    const char *argpath = self->argv[1];
    const char *org;
    char drtpath[PAD_FILE__NPATH*2];
    bool has_head_slash = argpath[0] == '/' || argpath[0] == '\\';

    if (has_head_slash) {
        // Absolute of home
        org = self->config->home_path;
    } else {
        // Relative of cd
        org = self->config->cd_path;
    }

    if (!strcmp(argpath, "/")) {
        snprintf(drtpath, sizeof drtpath, "%s", org);
    } else {
        snprintf(drtpath, sizeof drtpath, "%s/%s", org, argpath);
    }

    if (!cd(self, drtpath)) {
        return 1;
    }

    return 0;
}
