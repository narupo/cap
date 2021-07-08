#include <core/config.h>

void
CapConfig_Del(CapConfig *self) {
    if (self) {
        Pad_SafeFree(self);
    }
}

CapConfig *
CapConfig_New(void) {
    CapConfig *self = PadMem_Calloc(1, sizeof(*self));
    if (self == NULL) {
        goto error;
    }

    self->errstack = PadErrStack_New();
    if (self->errstack == NULL) {
        goto error;
    }

    self->pad_config = PadConfig_New();
    if (self->pad_config == NULL) {
        goto error;
    }

    return self;
error:
    if (self) {
        PadErrStack_Del(self->errstack);
        PadConfig_Del(self->pad_config);
        free(self);
    }
    return NULL;
}

static bool
if_not_exists_to_mkdir(CapConfig *self, const char *dirpath) {
    char path[FILE_NPATH];
    if (!PadFile_Solve(path, sizeof path, dirpath)) {
        Pad_PushErr("failed to solve %s", dirpath);
        return false;
    }
    if (!PadFile_IsExists(path)) {
        PadFile_MkdirQ(path);
    }
    return true;
}

char *
Pad_PopTailSlash(char *path);

PadErrStack *
CapConfig_GetErrStack(CapConfig *self) {
    return self->errstack;
}

CapConfig *
CapConfig_Init(CapConfig *self) {
    if (!PadConfig_Init(self->pad_config)) {
        Pad_PushErr("failed to init pad-config");
        return NULL;
    }

    self->scope = CAP_SCOPE_LOCAL;
    self->recursion_count = 0;

    strcpy(self->line_encoding, "lf");

    // init environment
    
    if (!if_not_exists_to_mkdir(self, "~/.cap")) {
        Pad_PushErr("failed to create ~/.cap");
        return NULL;
    }
    if (!if_not_exists_to_mkdir(self, "~/.cap/var")) {
        Pad_PushErr("failed to create ~/.cap/var");
        return NULL;
    }
    if (!if_not_exists_to_mkdir(self, "~/.cap/codes")) {
        Pad_PushErr("failed to create ~/.cap/codes");
        return NULL;
    }
    if (!if_not_exists_to_mkdir(self, "~/.cap/stdlib")) {
        Pad_PushErr("failed to create ~/.cap/stdlib");
        return NULL;
    }

    // solve path

    if (!PadFile_Solve(self->var_cd_path, sizeof self->var_cd_path, "~/.cap/var/cd")) {
        Pad_PushErr("failed to create path of cd of variable");
        return NULL;
    }
    if (!PadFile_Solve(self->var_home_path, sizeof self->var_home_path, "~/.cap/var/home")) {
        Pad_PushErr("failed to create path of home of variable");
        return NULL;
    }
    if (!PadFile_Solve(self->var_editor_path, sizeof self->var_editor_path, "~/.cap/var/editor")) {
        Pad_PushErr("failed to create path of editor of variable");
        return NULL;
    }
    if (!PadFile_Solve(self->codes_dir_path, sizeof self->codes_dir_path, "~/.cap/codes")) {
        Pad_PushErr("failed to solve path for snippet codes directory path");
        return NULL;
    }
    if (!PadFile_Solve(self->std_lib_dir_path, sizeof self->std_lib_dir_path, "~/.cap/stdlib")) {
        Pad_PushErr("failed to solve path for standard libraries directory");
        return NULL;
    }

    // read path from variables

    if (!PadFile_ReadLine(self->cd_path, sizeof self->cd_path, self->var_cd_path)) {
        // nothing todo
    }
    Pad_PopTailSlash(self->cd_path);

    if (!PadFile_ReadLine(self->home_path, sizeof self->home_path, self->var_home_path)) {
        if (!PadFile_Solve(self->home_path, sizeof self->home_path, "~/")) {
            Pad_PushErr("failed to solve path for home path");
            return NULL;
        }
    }
    Pad_PopTailSlash(self->home_path);

    if (!PadFile_ReadLine(self->editor, sizeof self->editor, self->var_editor_path)) {
        // nothing todo
    }

    return self;
}

PadConfig *
CapConfig_GetPadConfig(CapConfig *self) {
    return self->pad_config;
}
