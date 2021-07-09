#include <snippet/snippet.h>

/**
 * Structure of command
 */
struct snptcmd {
    const CapConfig *config;
    int argc;
    int optind;
    char **argv;
};

/**
 * Show usage of command
 *
 * @param[in] self pointer to snptcmd_t
 */
static void
snptcmd_show_usage(snptcmd_t *self) {
    fflush(stdout);
    fflush(stderr);
    fprintf(stderr, "Save or show snippet codes.\n"
        "\n"
        "Usage:\n"
        "\n"
        "    cap snippet [commands] [arguments]\n"
        "\n"
        "The commands are:\n"
        "\n"
        "    add     add snippet code\n"
        "    ls      show list of snippets\n"
        "    clear   clear all snippet codes\n"
        "\n"
    );
    fflush(stderr);
}

void
snptcmd_del(snptcmd_t *self) {
    if (!self) {
        return;
    }

    free(self);
}

snptcmd_t *
snptcmd_new(const CapConfig *config, int argc, char **argv) {
    snptcmd_t *self = PadMem_ECalloc(1, sizeof(*self));

    self->config = config;
    self->argc = argc;
    self->argv = argv;

    return self;
}

static int
snptcmd_show_files(snptcmd_t *self) {
    file_dir_t *dir = PadDir_Open(self->config->codes_dir_path);
    if (!dir) {
        PadErr_Err("failed to open directory \"%s\"", self->config->codes_dir_path);
        return 1;
    }

    for (PadDirNode *node; (node = PadDir_Read(dir)); ) {
        const char *name = PadDirNode_Name(node);
        if (!strcmp(name, ".") || !strcmp(name, "..")) {
            continue;
        }
        puts(name);
        PadDirNode_Del(node);
    }

    PadDir_Close(dir);
    return 0;
}

static int
snptcmd_add(snptcmd_t *self) {
    if (self->argc < 3) {
        PadErr_Err("failed to add snippet. need file name");
        return 1;
    }

    const char *name = self->argv[2];
    char path[FILE_NPATH];

    if (!strlen(self->config->codes_dir_path)) {
        PadErr_Err("codes directory path is empty");
        return 1;
    }

    if (!PadFile_Solvefmt(path, sizeof path, "%s/%s", self->config->codes_dir_path, name)) {
        PadErr_Err("failed to solve path for \"%s\"", name);
        return 1;
    }

    printf("path[%s]\n", path);
    FILE *fout = PadFile_Open(path, "wb");
    if (!fout) {
        PadErr_Err("failed to open snippet \"%s\"", name);
        return 1;
    }

    for (int c; (c = fgetc(stdin)) != EOF; ) {
        fputc(c, fout);
    }

    fflush(fout);
    PadFile_Close(fout);
    return 0;
}

static int
snptcmd_show(snptcmd_t *self) {
    if (self->argc < 2) {
        PadErr_Err("failed to show snippet. need file name");
        return 1;
    }

    const char *name = self->argv[1];
    char path[FILE_NPATH];

    if (!PadFile_Solvefmt(path, sizeof path, "%s/%s", self->config->codes_dir_path, name)) {
        PadErr_Err("failed to solve path for \"%s\"", name);
        return 1;
    }

    FILE *fin = PadFile_Open(path, "rb");
    if (!fin) {
        PadErr_Err("failed to open snippet \"%s\"", name);
        return 1;
    }

    char *content = file_readcp(fin);
    if (!content) {
        PadErr_Err("failed to read snippet code from \"%s\"", path);
        return 1;
    }

    fclose(fin);

    errstack_t *errstack = errstack_new();
    char *compiled = compile_argv(
        self->config,
        errstack,
        self->argc-2,
        self->argv+2,
        content
    );
    if (!compiled) {
        errstack_trace(errstack, stderr);
        fflush(stderr);
        free(content);
        errstack_del(errstack);
        return 1;
    }

    printf("%s", compiled);
    fflush(stdout);

    free(compiled);
    free(content);
    errstack_del(errstack);
    return 0;
}

static int
snptcmd_clear(snptcmd_t *self) {
    file_dir_t *dir = PadDir_Open(self->config->codes_dir_path);
    if (!dir) {
        PadErr_Err("failed to open directory \"%s\"", self->config->codes_dir_path);
        return 1;
    }

    for (PadDirNode *node; (node = PadDir_Read(dir)); ) {
        const char *name = PadDirNode_Name(node);
        if (!strcmp(name, ".") || !strcmp(name, "..")) {
            continue;
        }
        char path[FILE_NPATH];
        if (!PadFile_Solvefmt(path, sizeof path, "%s/%s", self->config->codes_dir_path, name)) {
            PadErr_Err("failed to solve path for \"%s\"", name);
            goto fail;
        }

        if (file_remove(path) != 0) {
            PadErr_Err("failed to remove file \"%s\"", path);
            goto fail;
        }

        PadDirNode_Del(node);
    }

fail:
    PadDir_Close(dir);
    return 0;
}

int
snptcmd_run(snptcmd_t *self) {
    if (self->argc < 2) {
        snptcmd_show_usage(self);
        return 0;
    } else if (PadCStr_Eq(self->argv[1], "clear")) {
        return snptcmd_clear(self);
    } else if (PadCStr_Eq(self->argv[1], "ls")) {
        return snptcmd_show_files(self);
    } else if (PadCStr_Eq(self->argv[1], "add")) {
        return snptcmd_add(self);
    }
    return snptcmd_show(self);
}