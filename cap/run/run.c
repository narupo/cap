#include <run/run.h>

enum {
    NSCRIPTNAME = 100,
    NCMDLINE = 256
};

struct runcmd {
    const CapConfig *config;
    int argc;
    char **argv;
};

void
runcmd_del(runcmd_t *self) {
    if (!self) {
        return;
    }

    free(self);
}

runcmd_t *
runcmd_new(const CapConfig *config, int argc, char **argv) {
    runcmd_t *self = PadMem_ECalloc(1, sizeof(*self));

    self->config = config;
    self->argc = argc;
    self->argv = argv;

    return self;
}

static char *
runcmd_read_script_line(runcmd_t *self, char *dst, size_t dstsz, const char *path) {
    // read first line of file
    FILE *fin = fopen(path, "rb");
    if (!fin) {
        return NULL;
    }

    char tmp[dstsz];
    file_getline(tmp, sizeof tmp, fin);
    cstr_pop_newline(tmp);

    // find !
    const char *needle = "!";
    char *at = strstr(tmp, needle);
    if (!at) {
        fclose(fin);
        return NULL;
    }
    at = at + strlen(needle);

#ifdef CAP__WINDOWS
    // fix script path for Windows (trim execute file name only, remove directories)
    char *last = at+strlen(at)-1;
    char *beg = at;
    for (char *p = last; p >= at; --p){
        if (*p == '/') {
            beg = p+1;
            break;
        }
    }
    at = beg;
#endif

    // copy path
    snprintf(dst, dstsz, "%s", at);

    // done
    if (fclose(fin) == EOF) {
        return NULL;
    }

    return dst;
}

int
runcmd_run(runcmd_t *self) {
    if (self->argc < 2) {
        PadErr_Err("need script file name");
        return 1;
    }

    const char *argpath = self->argv[1];
    const char *org = Cap_GetOrigin(self->config, argpath);

    // Create script path
    char tmppath[FILE_NPATH];
    snprintf(tmppath, sizeof tmppath, "%s/%s", org, argpath);

    char filepath[FILE_NPATH];
    if (!CapSymlink_FollowPath(self->config, filepath, sizeof filepath, tmppath)) {
        PadErr_Err("failed to follow path");
        return 1;
    }

    if (Cap_IsOutOfHome(self->config->home_path, filepath)) {
        PadErr_Err("invalid script. \"%s\" is out of home.", filepath);
        return 1;
    }

    // Read script line in file
    char script[NSCRIPTNAME];
    if (!runcmd_read_script_line(self, script, sizeof script, filepath)) {
        script[0] = '\0';
    }

    // Create command line
    string_t *cmdline = PadStr_New();
    if (strlen(script)) {
        PadStr_App(cmdline, script);
        PadStr_App(cmdline, " ");
    }
    PadStr_App(cmdline, filepath);
    PadStr_App(cmdline, " ");

    for (int32_t i = 2; i < self->argc; ++i) {
        PadStr_App(cmdline, "\"");
        PadStr_App(cmdline, self->argv[i]);
        PadStr_App(cmdline, "\"");
        PadStr_App(cmdline, " ");
    }
    str_popb(cmdline);

    // Start process communication
    int option = SAFESYSTEM_DEFAULT;
    const char *detach = getenv("CAP_RUN_DETACH");
    if (detach && detach[0] == '1') {
        option |= SAFESYSTEM_DETACH;
    }

    const char *scmdline = PadStr_Getc(cmdline);
    int status = Pad_SafeSystem(scmdline, option);
#if CAP__WINDOWS
    int exit_code = status;
#else
    int exit_code = WEXITSTATUS(status);
#endif

    // Done
    PadStr_Del(cmdline);
    return exit_code;
}