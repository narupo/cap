#include <cap/home/home.h>

struct CapHomeCmd {
    const CapConfig *config;
    int argc;
    char **argv;
};

void
CapHomeCmd_Del(CapHomeCmd *self) {
    if (!self) {
        return;
    }
    free(self);
}

CapHomeCmd *
CapHomeCmd_New(const CapConfig *config, int argc, char *argv[]) {
    CapHomeCmd *self = PadMem_Calloc(1, sizeof(*self));
    if (self == NULL) {
        return NULL;
    }

    self->config = config;
    self->argc = argc;
    self->argv = argv;

    return self;
}

int
CapHomeCmd_Run(CapHomeCmd *self) {
    int argc = self->argc;
    char **argv = self->argv;

    if (argc < 2) {
        char line[PAD_FILE__NPATH];
        if (!PadFile_ReadLine(line, sizeof line, self->config->var_home_path)) {
            PadErr_Err("failed to read line from home of variable");
            return 1;
        }
        printf("%s\n", line);
        return 0;
    }

    char newhome[PAD_FILE__NPATH];
    if (!PadFile_Solve(newhome, sizeof newhome, argv[1])) {
        PadErr_Err("failed to solve path from \"%s\"", argv[1]);
        return 1;
    }
    if (!PadFile_IsDir(newhome)) {
        PadErr_Err("%s is not a directory", newhome);
        return 1;
    }

    if (!PadFile_WriteLine(newhome, self->config->var_home_path)) {
        PadErr_Err("failed to write line to home variable");
        return 1;
    }

    if (!PadFile_WriteLine(newhome, self->config->var_cd_path)) {
        PadErr_Err("failed to write line to cd variable");
        return 1;
    }

    return 0;
}
