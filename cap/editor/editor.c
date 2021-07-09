#include <cap/editor/editor.h>

/**
 * structure of options
 */
struct Opts {
    bool is_help;
};

/**
 * structure of command
 */
struct CapEditorCmd {
    const CapConfig *config;
    int argc;
    int optind;
    char **argv;
    struct Opts opts;
    char editor[PAD_FILE__NPATH];
};

/**
 * show usage of command
 *
 * @param[in] self pointer to CapEditorCmd
 */
static int
usage(CapEditorCmd *self) {
    fflush(stdout);
    fflush(stderr);
    fprintf(stderr, "Usage:\n"
        "\n"
        "    cap editor [options...]\n"
        "\n"
        "The options are:\n"
        "\n"
        "    -h, --help    show usage\n"
        "\n"
    );
    fflush(stderr);
    return 0;
}

/**
 * parse options
 *
 * @param[in] self pointer to CapEditorCmd 
 *
 * @return success to true
 * @return failed to false
 */
static bool
parse_opts(CapEditorCmd *self) {
    // parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {0},
    };

    self->opts = (struct Opts){0};

    extern int opterr;
    extern int optind;
    opterr = 0; // ignore error messages
    optind = 0; // init index of parse

    for (;;) {
        int optsindex;
        int cur = getopt_long(self->argc, self->argv, "h", longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 0: /* long option only */ break;
        case 'h': self->opts.is_help = true; break;
        case '?':
        default:
            PadErr_Die("unknown option");
            return false;
            break;
        }
    }

    if (self->argc < optind) {
        PadErr_Die("failed to parse option");
        return false;
    }

    self->optind = optind;
    return true;
}

void
CapEditorCmd_Del(CapEditorCmd *self) {
    if (!self) {
        return;
    }
    free(self);
}

CapEditorCmd *
CapEditorCmd_New(const CapConfig *config, int argc, char **argv) {
    CapEditorCmd *self = PadMem_Calloc(1, sizeof(*self));
    if (self == NULL) {
        return NULL;
    }

    self->config = config;
    self->argc = argc;
    self->argv = argv;

    if (!parse_opts(self)) {
        CapEditorCmd_Del(self);
        return NULL;
    }

    return self;
}

static int
show_editor(CapEditorCmd *self) {
    self->editor[0] = '\0';
    if (!PadFile_ReadLine(self->editor, sizeof self->editor, self->config->var_editor_path)) {
        PadErr_Err("failed to read editor from editor of variable");
        return 1;
    }
    if (strlen(self->editor)) {
        puts(self->editor);
    }
    return 0;
}

static int
set_editor(CapEditorCmd *self) {
    const char *editor = self->argv[self->optind];
    if (!PadFile_WriteLine(editor, self->config->var_editor_path)) {
        PadErr_Err("failed to write editor into editor of variable");
        return 1;
    }
    return 0;
}

int
CapEditorCmd_Run(CapEditorCmd *self) {
    if (self->opts.is_help) {
        return usage(self);
    }

    if (self->argc < self->optind+1) {
        return show_editor(self);
    }

    return set_editor(self);
}
