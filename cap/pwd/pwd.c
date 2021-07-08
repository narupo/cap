/**
 * Cap
 *
 * License: MIT
 *  Author: narupo
 *   Since: 2016
 */
#include <pwd/pwd.h>

struct Opts {
    bool ishelp;
    bool isnorm;
};

struct pwdcmd {
    const CapConfig *config;
    int argc;
    char **argv;
    struct Opts opts;
};

static bool
pwdcmd_parse_opts(pwdcmd_t *self) {
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
        default: perror("Unknown option"); break;
        }
    }

    if (self->argc < optind) {
        perror("Failed to parse option");
        return false;
    }

    return true;
}

void
pwdcmd_del(pwdcmd_t *self) {
	if (!self) {
        return;
    }

    free(self);
}

pwdcmd_t *
pwdcmd_new(const CapConfig *config, int argc, char **argv) {
	pwdcmd_t *self = PadMem_ECalloc(1, sizeof(*self));
	self->config = config;
    self->argc = argc;
    self->argv = argv;
	return self;
}

char *
replace_slashes(const char *s) {
    string_t *dst = str_new();

    for (const char *p = s; *p; ++p) {
        if (*p == '\\') {
            str_pushb(dst, '/');
        } else {
            str_pushb(dst, *p);
        }
    }

    return str_esc_del(dst);
}

int
pwdcmd_run(pwdcmd_t *self) {
    if (!pwdcmd_parse_opts(self)) {
        PadErr_Error("failed to parse option");
        return 1;
    }

	const char *cd = self->config->cd_path;
    const char *home = self->config->home_path;

    if (self->opts.isnorm) {
    	printf("%s\n", cd);
    } else {
        int32_t homelen = strlen(home);
        int32_t cdlen = strlen(cd);
        if (cdlen-homelen < 0) {
            PadErr_Error("invalid cd \"%s\" or home \"%s\"", cd, home);
            return 4;
        }
        if (cdlen-homelen == 0) {
            printf("/\n");
        } else {
            const char *p = cd + homelen;
            char *s = replace_slashes(p);
            printf("%s\n", s);
            free(s);
        }
    }

    fflush(stdout);
	return 0;
}
