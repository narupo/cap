#include <make/make.h>

/**
 * Structure of command
 */
struct CapMakeCmd {
    const CapConfig *config;
    int argc;
    char **argv;
    PadErrStack *errstack;
};

void
CapMakeCmd_Del(CapMakeCmd *self) {
    if (!self) {
        return;
    }

    PadErrStack_Del(self->errstack);
    Pad_SafeFree(self);
}

CapMakeCmd *
CapMakeCmd_New(const CapConfig *config, int argc, char **argv) {
    CapMakeCmd *self = PadMem_Calloc(1, sizeof(*self));
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
        Pad_SafeFree(self);
    }
    return NULL;
}

int
CapMakeCmd_MakeFromArgs(
    const CapConfig *config,
    PadErrStack *errstack,
    int argc,
    char *argv[],
    bool solve_path
) {
    bool use_stdin = false;
    if (argc < 2) {
        use_stdin = true;
    } else {
        if (argv[1] && argv[1][0] == '-') {
            use_stdin = true;
        }
    }

    char *src = NULL;

    if (use_stdin) {
        src = PadFile_ReadCopy(stdin);
        if (!src) {
            PadErrStack_Add(errstack, "failed to read from stdin");
            return 1;
        }
    } else {
        char path[FILE_NPATH];
        const char *cap_path = argv[1];

        if (solve_path) {
            if (!Cap_SolveCmdlineArgPath(config, path, sizeof path, cap_path)) {
                PadErrStack_Add(errstack, "failed to solve cap path");
                return 1;
            }            
        } else {
            PadCStr_Copy(path, sizeof path, cap_path);
        }

        src = PadFile_ReadCopy_from_path(path);
        if (!src) {
            PadErrStack_Add(errstack, "failed to read from \"%s\"", path);
            return 1;
        }
    }

    char *compiled = Pad_CompileArgv(
        config,
        errstack,
        argc - 1,
        argv + 1,
        src
    );
    if (!compiled) {
        PadErrStack_Add(
            errstack,
            "failed to compile from \"%s\"",
            (argv[1] ? argv[1] : "stdin")
        );
        fflush(stderr);
        Pad_SafeFree(src);
        return 1;
    }

    PadCStr_PopLastNewline(compiled);
    printf("%s", compiled);
    fflush(stdout);

    Pad_SafeFree(compiled);
    Pad_SafeFree(src);
    return 0;
}

int
CapMakeCmd_Run(CapMakeCmd *self) {
    int result = CapMakeCmd_MakeFromArgs(
        self->config,
        self->errstack,
        self->argc,
        self->argv,
        true
    );
    if (result != 0) {
        PadErrStack_TraceSimple(self->errstack, stderr);
    }

    return result;
}
