#include <cap/cook/cook.h>

/**
 * structure of command
 */
struct CapCookCmd {
    const CapConfig *config;
    int argc;
    char **argv;
    PadErrStack *errstack;
};

void
CapCookCmd_Del(CapCookCmd *self) {
    if (!self) {
        return;
    }

    PadErrStack_Del(self->errstack);
    free(self);
}

CapCookCmd *
CapCookCmd_New(const CapConfig *config, int argc, char **argv) {
    CapCookCmd *self = PadMem_Calloc(1, sizeof(*self));
    if (self == NULL) {
        return NULL;
    }

    self->config = config;
    self->argc = argc;
    self->argv = argv;
    self->errstack = PadErrStack_New();

    return self;
}

int
CapCookCmd_Run(CapCookCmd *self) {
    int result = CapMakeCmd_MakeFromArgs(
        self->config,
        self->errstack,
        self->argc,
        self->argv,
        false  // look me!
    );
    if (result != 0) {
        PadErrStack_TraceSimple(self->errstack, stderr);
    }

    return result;
}
