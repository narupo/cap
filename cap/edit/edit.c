#include <edit/edit.h>

struct Opts {
    bool is_global;
};

struct editcmd {
    const CapConfig *config;
    struct Opts opts;
    int argc;
    char **argv;
    char editor[1024];
    char cmdline[2048];
    char open_fname[1024];
};

editcmd_t *
editcmd_parse_opts(editcmd_t *self) {
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
            PadErr_Error("unsupported option");
            return NULL;
            break;
        }
    }

    if (self->argc < optind) {
        PadErr_Error("failed to parse option");
        return NULL;
    }

    return self;
}

void
editcmd_del(editcmd_t *self) {
    if (!self) {
        return;
    }

    free(self);
}

editcmd_t *
editcmd_new(const CapConfig *config, int argc, char **argv) {
    editcmd_t *self = PadMem_ECalloc(1, sizeof(*self));

    self->config = config;
    self->argc = argc;
    self->argv = argv;

    if (!editcmd_parse_opts(self)) {
        PadErr_Die("failed to parse options");
    }

    return self;
}

editcmd_t *
editcmd_read_editor(editcmd_t *self) {
    self->editor[0] = '\0';
    if (!file_readline(self->editor, sizeof self->editor, self->config->var_editor_path)) {
        return NULL;
    }
    if (!strlen(self->editor)) {
        return NULL;
    }
    return self;
}

editcmd_t *
editcmd_create_open_fname(editcmd_t *self, const char *cap_path) {
    if (!solve_cmdline_arg_path(self->config, self->open_fname, sizeof self->open_fname, cap_path)) {
        PadErr_Error("failed to solve cap path");
        return NULL;
    }

    return self;
}

int
editcmd_run(editcmd_t *self) {
    const char *fname = NULL;
    if (self->argc >= 2) {
        fname = self->argv[optind];
    }

    if (!editcmd_read_editor(self)) {
        PadErr_Die("not found editor. please setting with 'cap editor' command");
        return 1;
    }

    cstr_app(self->cmdline, sizeof self->cmdline, self->editor);
    if (fname) {
        if (!editcmd_create_open_fname(self, fname)) {
            PadErr_Die("failed to create open file name");
            return 1;
        }
        cstr_app(self->cmdline, sizeof self->cmdline, " ");
        cstr_app(self->cmdline, sizeof self->cmdline, self->open_fname);
    }

    safesystem(self->cmdline, SAFESYSTEM_EDIT);

    return 0;
}
