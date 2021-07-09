/**
 * Cap
 *
 * License: MIT
 *  Author: narupo
 *   Since: 2016
 */
#include <cap/pwd/pwd.h>

struct Opts {
    bool ishelp;
    bool isnorm;
};

struct CapPwdCmd {
    const CapConfig *config;
    int argc;
    char **argv;
    struct Opts opts;
};

static bool
parse_opts(CapPwdCmd *self) {
    // Parse options
    struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {"normalize", no_argument, 0, 'n'},
        {0},
    };
    const char *shortopts = "hn";

    self->opts = (struct Opts){0};
    extern int opterr;
    extern int optind;
    opterr = 0; // ignore error messages
    optind = 0; // init index of parse

    for (;;) {
        int optsindex;
        int cur = getopt_long(self->argc, self->argv, shortopts, longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 0: /* Long option only */ break;
        case 'h': self->opts.ishelp = true; break;
        case 'n': self->opts.isnorm = true; break;
        case '?':
        default: PadErr_Err("Unknown option"); break;
        }
    }

    if (self->argc < optind) {
        PadErr_Err("Failed to parse option");
        return false;
    }

    return true;
}

void
CapPwdCmd_Del(CapPwdCmd *self) {
	if (!self) {
        return;
    }
    Pad_SafeFree(self);
}

CapPwdCmd *
CapPwdCmd_New(const CapConfig *config, int argc, char **argv) {
	CapPwdCmd *self = PadMem_Calloc(1, sizeof(*self));
    if (self == NULL) {
        return NULL;
    }

	self->config = config;
    self->argc = argc;
    self->argv = argv;

	return self;
}

char *
replace_slashes(const char *s) {
    string_t *dst = PadStr_New();

    for (const char *p = s; *p; ++p) {
        if (*p == '\\') {
            PadStr_PushBack(dst, '/');
        } else {
            PadStr_PushBack(dst, *p);
        }
    }

    return PadStr_EscDel(dst);
}

int
CapPwdCmd_Run(CapPwdCmd *self) {
    if (!parse_opts(self)) {
        PadErr_Err("failed to parse option");
        return 1;
    }

	const char *cd = self->config->cd_path;
    const char *home = self->config->home_path;

    if (self->opts.isnorm) {
    	printf("%s\n", cd);
    } else {
        int32_t homelen = strlen(home);
        int32_t cdlen = strlen(cd);
        if (cdlen - homelen < 0) {
            PadErr_Err("invalid cd \"%s\" or home \"%s\"", cd, home);
            return 4;
        }
        if (cdlen - homelen == 0) {
            printf("/\n");
        } else {
            const char *p = cd + homelen;
            char *s = replace_slashes(p);
            printf("%s\n", s);
            Pad_SafeFree(s);
        }
    }

    fflush(stdout);
	return 0;
}
