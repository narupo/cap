#include <cap/snippet/snippet.h>

/**
 * Structure of command
 */
struct CapSnptCmd {
    const CapConfig *config;
    int argc;
    int optind;
    char **argv;
};

/**
 * Show usage of command
 *
 * @param[in] self pointer to CapSnptCmd
 */
static void
usage(CapSnptCmd *self) {
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
    exit(0);
}

void
CapSnptCmd_Del(CapSnptCmd *self) {
    if (!self) {
        return;
    }
    Pad_SafeFree(self);
}

CapSnptCmd *
CapSnptCmd_New(const CapConfig *config, int argc, char **argv) {
    CapSnptCmd *self = PadMem_Calloc(1, sizeof(*self));
    if (self == NULL) {
        return NULL;
    }

    self->config = config;
    self->argc = argc;
    self->argv = argv;

    return self;
}

static int
_show_files(CapSnptCmd *self) {
    PadDir *dir = PadDir_Open(self->config->codes_dir_path);
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
_add(CapSnptCmd *self) {
    if (self->argc < 3) {
        PadErr_Err("failed to add snippet. need file name");
        return 1;
    }

    const char *name = self->argv[2];
    char path[PAD_FILE__NPATH];

    if (!strlen(self->config->codes_dir_path)) {
        PadErr_Err("codes directory path is empty");
        return 1;
    }

    if (!PadFile_SolveFmt(path, sizeof path, "%s/%s", self->config->codes_dir_path, name)) {
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
snptcmd_show(CapSnptCmd *self) {
    if (self->argc < 2) {
        PadErr_Err("failed to show snippet. need file name");
        return 1;
    }

    const char *name = self->argv[1];
    char path[PAD_FILE__NPATH];

    if (!PadFile_SolveFmt(path, sizeof path, "%s/%s", self->config->codes_dir_path, name)) {
        PadErr_Err("failed to solve path for \"%s\"", name);
        return 1;
    }

    FILE *fin = PadFile_Open(path, "rb");
    if (!fin) {
        PadErr_Err("failed to open snippet \"%s\"", name);
        return 1;
    }

    char *content = PadFile_ReadCopy(fin);
    if (!content) {
        PadErr_Err("failed to read snippet code from \"%s\"", path);
        return 1;
    }

    fclose(fin);

    PadErrStack *errstack = PadErrStack_New();
    char *compiled = Pad_CompileArgv(
        self->config,
        errstack,
        self->argc-2,
        self->argv+2,
        content
    );
    if (!compiled) {
        PadErrStack_Trace(errstack, stderr);
        fflush(stderr);
        Pad_SafeFree(content);
        PadErrStack_Del(errstack);
        return 1;
    }

    printf("%s", compiled);
    fflush(stdout);

    Pad_SafeFree(compiled);
    Pad_SafeFree(content);
    PadErrStack_Del(errstack);
    return 0;
}

static int
_clear(CapSnptCmd *self) {
    PadDir *dir = PadDir_Open(self->config->codes_dir_path);
    if (!dir) {
        PadErr_Err("failed to open directory \"%s\"", self->config->codes_dir_path);
        return 1;
    }

    for (PadDirNode *node; (node = PadDir_Read(dir)); ) {
        const char *name = PadDirNode_Name(node);
        if (!strcmp(name, ".") || !strcmp(name, "..")) {
            continue;
        }
        char path[PAD_FILE__NPATH];
        if (!PadFile_SolveFmt(path, sizeof path, "%s/%s", self->config->codes_dir_path, name)) {
            PadErr_Err("failed to solve path for \"%s\"", name);
            goto fail;
        }

        if (PadFile_Remove(path) != 0) {
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
CapSnptCmd_Run(CapSnptCmd *self) {
    if (self->argc < 2) {
        usage(self);
    } else if (PadCStr_Eq(self->argv[1], "clear")) {
        return _clear(self);
    } else if (PadCStr_Eq(self->argv[1], "ls")) {
        return _show_files(self);
    } else if (PadCStr_Eq(self->argv[1], "add")) {
        return _add(self);
    }
    return snptcmd_show(self);
}
