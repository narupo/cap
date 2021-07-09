#include <cap/link/link.h>

/**
 * Structure of options
 */
struct Opts {
    bool is_help;
    bool is_unlink;
};

/**
 * Structure of command
 */
struct CapLinkCmd {
    const CapConfig *config;
    int argc;
    char **argv;
    struct Opts opts;
    int optind;
};

/**
 * Parse options
 * 
 * @param[in] *self   
 * @param[in] argc    
 * @param[in] *argv[] 
 * 
 * @return success to pointer to self
 * @return failed to NULL
 */
static CapLinkCmd *
parse_opts(CapLinkCmd *self) {
    // Parse options
    static struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {"unlink", no_argument, 0, 'u'},
        {0},
    };

    self->opts = (struct Opts){
        .is_help = false,
        .is_unlink = false,
    };
    opterr = 0;
    optind = 0;

    for (;;) {
        int optsindex;
        int cur = getopt_long(self->argc, self->argv, "hu", longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 0: /* Long option only */ break;
        case 'h': self->opts.is_help = true; break;
        case 'u': self->opts.is_unlink = true; break;
        case '?':
        default:
            PadErr_Die("unsupported option");
            return NULL;
            break;
        }
    }

    if (self->argc < optind) {
        return NULL;
    }

    self->optind = optind;

    return self;
}

void
CapLinkCmd_Del(CapLinkCmd *self) {
    if (!self) {
        return;
    }
    free(self);
}

CapLinkCmd *
CapLinkCmd_New(const CapConfig *config, int argc, char **argv) {
    CapLinkCmd *self = PadMem_Calloc(1, sizeof(*self));
    if (self == NULL) {
        return NULL;
    }

    self->config = config;
    self->argc = argc;
    self->argv = argv;

    if (!parse_opts(self)) {
        CapLinkCmd_Del(self);
        return NULL;
    }

    return self;
}

static int
usage(const CapLinkCmd *self) {
    fprintf(stderr,
        "Usage:\n"
        "\n"
        "    cap link [options] [link-name] [cap-path]\n"
        "\n"
        "The options are:\n"
        "\n"
        "    -h, --help       show usage.\n"
        "    -u, --unlink     unlink link.\n"
        "\n"
        "Examples:\n"
        "\n"
        "    $ cap link mylink /path/to/file\n"
        "    $ cap link mylink /path/to/dir\n"
        "    $ cap link -u mylink\n"
        "\n"
    );
    return 0;
}

static int
_unlink(CapLinkCmd *self) {
    if (self->argc-self->optind < 1) {
        return usage(self);
    }

    const char *linkname = self->argv[self->optind];
    const char *org = Cap_GetOrigin(self->config, linkname);

    char path[PAD_FILE__NPATH];
    if (!PadFile_SolveFmt(path, sizeof path, "%s/%s", org, linkname)) {
        PadErr_Err("failed to solve path");
        return 1;
    }

    if (Cap_IsOutOfHome(self->config->home_path, path)) {
        PadErr_Err("\"%s\" is out of home", linkname);
        return 1;
    }

    if (!CapSymlink_IsLinkFile(path)) {
        PadErr_Err("\"%s\" is not Cap's symbolic link", linkname);
        return 1;
    }

    if (PadFile_Remove(path) != 0) {
        PadErr_Err("failed to unlink");
        return 1;
    }

    return 0;
}

static int
_link(CapLinkCmd *self) {
    if (self->argc-self->optind < 2) {
        return usage(self);
    }

    const char *linkname = self->argv[self->optind];
    if (strstr(linkname, "..")) {
        PadErr_Err("Cap's symbolic link is not allow relative path");
        return 1;
    }

    const char *cappath = self->argv[self->optind+1];
    const char *org = Cap_GetOrigin(self->config, linkname);

    char dstpath[PAD_FILE__NPATH];
    if (!PadFile_SolveFmt(dstpath, sizeof dstpath, "%s/%s", org, linkname)) {
        PadErr_Err("failed to solve path");
        return 1;
    }

    if (Cap_IsOutOfHome(self->config->home_path, dstpath)) {
        PadErr_Err("\"%s\" is out of home", linkname);
        return 1;
    }

    char line[PAD_FILE__NPATH + 100];
    snprintf(line, sizeof line, "%s %s", CAP_SYMLINK__HEADER, cappath);
    if (!PadFile_WriteLine(line, dstpath)) {
        PadErr_Err("failed to create link");
        return 1;        
    }

    return 0;
}

int
CapLinkCmd_Run(CapLinkCmd *self) {
    if (self->opts.is_help) {
        return usage(self);
    }

    if (self->opts.is_unlink) {
        return _unlink(self);
    }

    return _link(self);
}
