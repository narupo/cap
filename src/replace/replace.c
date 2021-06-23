#include <replace/replace.h>

#define error(stack, fmt, ...) errstack_pushb(stack, NULL, 0, NULL, 0, fmt, ##__VA_ARGS__)

/**
 * structure of command
 */
struct replacecmd {
    const config_t *config;
    int argc;
    char **argv;
    errstack_t *errstack;
};

void
replacecmd_del(replacecmd_t *self) {
    if (!self) {
        return;
    }

    errstack_del(self->errstack);
    free(self);
}

replacecmd_t *
replacecmd_new(const config_t *config, int argc, char **argv) {
    replacecmd_t *self = mem_ecalloc(1, sizeof(*self));

    self->config = config;
    self->argc = argc;
    self->argv = argv;
    self->errstack = errstack_new();

    return self;
}

static int
replace(replacecmd_t *self) {
    FILE *fin = NULL;
    FILE *fout = NULL;
    const char *cap_path = NULL;
    char path[FILE_NPATH];
    char *src = NULL;
    char *compiled = NULL;
    bool use_stdin = false;

    if (self->argc < 2 ||
        (self->argv[1] && self->argv[1][0] == '-')) {
        fin = stdin;
        use_stdin = true;
    } else {
        cap_path = self->argv[1];
        if (!solve_cmdline_arg_path(self->config, path, sizeof path, cap_path)) {
            error(self->errstack, "failed to solve cap path");
            return 1;
        }            

        fin = fopen(path, "r");
        if (fin == NULL) {
            error(self->errstack, "failed to open file %s", path);
            goto error;
        }
    }

    src = file_readcp(fin);
    if (src == NULL) {
        error(self->errstack, "failed to read from file");
        goto error;
    }
    fclose(fin);
    fin = NULL;

    compiled = compile_argv(
        self->config,
        self->errstack,
        self->argc,
        self->argv,
        src
    );

    if (use_stdin) {
        printf("%s", compiled);
        free(src);
        free(compiled);
        return 0;
    }

    // replace
    fout = fopen(path, "w");
    if (fout == NULL) {
        error(self->errstack, "failed to open file %s for write", path);
        goto error;
    }

    if (fwrite(compiled, sizeof(char), strlen(compiled), fout) == 0) {
        error(self->errstack, "failed to write data at %s", cap_path);
        goto error;
    }

    free(src);
    free(compiled);
    if (fout) {
        fclose(fout);
    }
    return 0;
error:
    free(src);
    free(compiled);
    if (fout) {
        fclose(fout);
    }
    return 1;
}

int
replacecmd_run(replacecmd_t *self) {
    int result = replace(self);
    if (errstack_len(self->errstack)) {
        errstack_trace(self->errstack, stderr);
        return result;
    }
    return result;
}
