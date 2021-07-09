#include <home/home.h>

struct homecmd {
    const CapConfig *config;
    int argc;
    char **argv;
};

void
homecmd_del(homecmd_t *self) {
    if (!self) {
        return;
    }

    free(self);
}

homecmd_t *
homecmd_new(const CapConfig *config, int argc, char *argv[]) {
    homecmd_t *self = PadMem_ECalloc(1, sizeof(*self));
    self->config = config;
    self->argc = argc;
    self->argv = argv;
    return self;
}

int
homecmd_run(homecmd_t *self) {
    int argc = self->argc;
    char **argv = self->argv;

    if (argc < 2) {
        char line[FILE_NPATH];
        if (!PadFile_ReadLine(line, sizeof line, self->config->var_home_path)) {
            PadErr_Err("failed to read line from home of variable");
            return 1;
        }
        printf("%s\n", line);
        return 0;
    }

    char newhome[FILE_NPATH];
    if (!PadFile_Solve(newhome, sizeof newhome, argv[1])) {
        PadErr_Err("failed to solve path from \"%s\"", argv[1]);
        return 2;
    }
    if (!PadFile_IsDir(newhome)) {
        PadErr_Err("%s is not a directory", newhome);
        return 3;
    }

    if (!PadFile_WriteLine(newhome, self->config->var_home_path)) {
        PadErr_Err("failed to write line to home variable");
        return 4;
    }

    if (!PadFile_WriteLine(newhome, self->config->var_cd_path)) {
        PadErr_Err("failed to write line to cd variable");
        return 5;
    }

    return 0;
}