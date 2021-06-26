#include <core/config.h>

void
config_del(config_t *self) {
    if (self) {
        free(self);
    }
}

config_t *
config_new(void) {
    config_t *self = mem_ecalloc(1, sizeof(*self));
    if (self == NULL) {
        goto error;
    }

    self->errstack = errstack_new();
    if (self->errstack == NULL) {
        goto error;
    }

    return self;
error:
    free(self);
    return NULL;
}

static bool
if_not_exists_to_mkdir(config_t *self, const char *dirpath) {
    char path[FILE_NPATH];
    if (!file_solve(path, sizeof path, dirpath)) {
        blush("failed to solve %s", dirpath);
        return false;
    }
    if (!file_exists(path)) {
        file_mkdirq(path);
    }
    return true;
}

char *
pop_tail_slash(char *path);

errstack_t *
config_get_error_stack(config_t *self) {
    return self->errstack;
}

config_t *
config_init(config_t *self) {
    self->scope = CAP_SCOPE_LOCAL;
    self->recursion_count = 0;

    strcpy(self->line_encoding, "lf");

    // init environment
    
    if (!if_not_exists_to_mkdir(self, "~/.cap")) {
        blush("failed to create ~/.cap");
        return false;
    }
    if (!if_not_exists_to_mkdir(self, "~/.cap/var")) {
        blush("failed to create ~/.cap/var");
        return false;
    }
    if (!if_not_exists_to_mkdir(self, "~/.cap/codes")) {
        blush("failed to create ~/.cap/codes");
        return false;
    }
    if (!if_not_exists_to_mkdir(self, "~/.cap/stdlib")) {
        blush("failed to create ~/.cap/stdlib");
        return false;
    }

    // solve path

    if (!file_solve(self->var_cd_path, sizeof self->var_cd_path, "~/.cap/var/cd")) {
        blush("failed to create path of cd of variable");
        return false;
    }
    if (!file_solve(self->var_home_path, sizeof self->var_home_path, "~/.cap/var/home")) {
        blush("failed to create path of home of variable");
        return false;
    }
    if (!file_solve(self->var_editor_path, sizeof self->var_editor_path, "~/.cap/var/editor")) {
        blush("failed to create path of editor of variable");
        return false;
    }
    if (!file_solve(self->codes_dir_path, sizeof self->codes_dir_path, "~/.cap/codes")) {
        blush("failed to solve path for snippet codes directory path");
        return false;
    }
    if (!file_solve(self->std_lib_dir_path, sizeof self->std_lib_dir_path, "~/.cap/stdlib")) {
        blush("failed to solve path for standard libraries directory");
        return false;
    }

    // read path from variables

    if (!file_readline(self->cd_path, sizeof self->cd_path, self->var_cd_path)) {
        // nothing todo
    }
    pop_tail_slash(self->cd_path);

    if (!file_readline(self->home_path, sizeof self->home_path, self->var_home_path)) {
        if (!file_solve(self->home_path, sizeof self->home_path, "~/")) {
            blush("failed to solve path for home path");
            return false;
        }
    }
    pop_tail_slash(self->home_path);

    if (!file_readline(self->editor, sizeof self->editor, self->var_editor_path)) {
        // nothing todo
    }

    return self;
}
