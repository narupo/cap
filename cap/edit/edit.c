#include <edit/edit.h>

struct Opts {
    bool is_global;
};

struct CapEditCmd {
    const CapConfig *config;
    struct Opts opts;
    int argc;
    char **argv;
    char editor[1024];
    char cmdline[2048];
    char open_fname[1024];
};

static CapEditCmd *
parse_opts(CapEditCmd *self) {
    // Parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {"global", no_argument, 0, 'g'},
        {0},
    };

    self->opts = (struct Opts){0};

    extern int opterr;
    opterr = 0; // ignore error messages
    optind = 0; // init index of parse

    for (;;) {
        int optsindex;
        int cur = getopt_long(self->argc, self->argv, "hg", longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 0: /* Long option only */ break;
        case 'h': /* Help */ break;
        case 'g': self->opts.is_global = true; break;
        case '?':
        default:
            PadErr_Err("unsupported option");
            return NULL;
            break;
        }
    }

    if (self->argc < optind) {
        PadErr_Err("failed to parse option");
        return NULL;
    }

    return self;
}

void
CapEditCmd_Del(CapEditCmd *self) {
    if (!self) {
        return;
    }
    free(self);
}

CapEditCmd *
CapEditCmd_New(const CapConfig *config, int argc, char **argv) {
    CapEditCmd *self = PadMem_Calloc(1, sizeof(*self));
    if (self == NULL) {
        return NULL;
    }

    self->config = config;
    self->argc = argc;
    self->argv = argv;

    if (!parse_opts(self)) {
        PadErr_Die("failed to parse options");
    }

    return self;
}

static CapEditCmd *
read_editor(CapEditCmd *self) {
    self->editor[0] = '\0';
    if (!PadFile_ReadLine(self->editor, sizeof self->editor, self->config->var_editor_path)) {
        return NULL;
    }
    if (!strlen(self->editor)) {
        return NULL;
    }
    return self;
}

static CapEditCmd *
create_open_fname(CapEditCmd *self, const char *cap_path) {
    if (!Cap_SolveCmdlineArgPath(self->config, self->open_fname, sizeof self->open_fname, cap_path)) {
        PadErr_Err("failed to solve cap path");
        return NULL;
    }

    return self;
}

int
CapEditCmd_Run(CapEditCmd *self) {
    const char *fname = NULL;
    if (self->argc >= 2) {
        fname = self->argv[optind];
    }

    if (!read_editor(self)) {
        PadErr_Die("not found editor. please setting with 'cap editor' command");
        return 1;
    }

    PadCStr_App(self->cmdline, sizeof self->cmdline, self->editor);
    if (fname) {
        if (!create_open_fname(self, fname)) {
            PadErr_Die("failed to create open file name");
            return 1;
        }
        PadCStr_App(self->cmdline, sizeof self->cmdline, " ");
        PadCStr_App(self->cmdline, sizeof self->cmdline, self->open_fname);
    }

    return Pad_SafeSystem(self->cmdline, PAD_SAFESYSTEM__EDIT);
}
