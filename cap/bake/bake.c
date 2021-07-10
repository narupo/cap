#include <cap/bake/bake.h>

/**
 * structure of command
 */
struct CapBakeCmd {
    const CapConfig *config;
    int argc;
    char **argv;
    PadErrStack *errstack;
};

void
CapBakeCmd_Del(CapBakeCmd *self) {
    if (!self) {
        return;
    }

    PadErrStack_Del(self->errstack);
    free(self);
}

CapBakeCmd *
CapBakeCmd_New(const CapConfig *config, int argc, char **argv) {
    CapBakeCmd *self = PadMem_Calloc(1, sizeof(*self));
    if (self == NULL) {
        goto error;
    }

    self->config = config;
    self->argc = argc;
    self->argv = argv;
    self->errstack = PadErrStack_New();
    if (self->errstack == NULL) {
        goto error;
    }

    return self;
error:
    if (self) {
        PadErrStack_Del(self->errstack);
        free(self);
    }
    return NULL;
}

static int
bake(CapBakeCmd *self) {
    FILE *fin = NULL;
    FILE *fout = NULL;
    const char *cap_path = NULL;
    char path[PAD_FILE__NPATH];
    char *make_path = NULL;
    char *src = NULL;
    char *compiled = NULL;
    bool use_stdin = false;

    if (self->argc < 2 ||
        (self->argv[1] && self->argv[1][0] == '-')) {
        fin = stdin;
        use_stdin = true;
    } else {
        cap_path = self->argv[1];
        if (!Cap_SolveCmdlineArgPath(self->config, path, sizeof path, cap_path)) {
            Pad_PushErr("failed to solve cap path");
            return 1;
        }            

        make_path = path;

        fin = fopen(path, "r");
        if (fin == NULL) {
            Pad_PushErr("failed to open file %s", path);
            goto error;
        }
    }

    src = PadFile_ReadCopy(fin);
    if (src == NULL) {
        Pad_PushErr("failed to read from file");
        goto error;
    }
    fclose(fin);
    fin = NULL;

    compiled = Cap_MakeArgv(
        self->config,
        self->errstack,
        make_path,
        src,
        self->argc - 1,
        self->argv + 1
    );
    if (compiled == NULL) {
        Pad_PushErr("failed to compile");
        goto error;        
    }

    if (use_stdin) {
        printf("%s", compiled);
        free(src);
        free(compiled);
        return 0;
    }

    // bake
    fout = fopen(path, "wb");
    if (fout == NULL) {
        Pad_PushErr("failed to open file %s for write", path);
        goto error;
    }

    if (fwrite(compiled, sizeof(char), strlen(compiled), fout) == 0) {
        Pad_PushErr("failed to write data at %s", cap_path);
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
CapBakeCmd_Run(CapBakeCmd *self) {
    int result = bake(self);
    if (PadErrStack_Len(self->errstack)) {
        PadErrStack_TraceSimple(self->errstack, stderr);
        return result;
    }
    return result;
}
