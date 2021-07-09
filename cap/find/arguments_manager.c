#include "find/arguments_manager.h"

struct arguments_manager {
    cstring_array_t *args;
};

void
argsmgr_del(CapArgsMgr *self) {
    if (!self) {
        return;
    }

    cstrarr_del(self->args);
    free(self);
}

CapArgsMgr *
argsmgr_new(char *argv[]) {
    CapArgsMgr *self = PadMem_ECalloc(1, sizeof(*self));

    self->args = cstrarr_new();

    for (char **ap = argv; *ap; ++ap) {
        cstrarr_pushb(self->args, *ap);
    }

    return self; 
}

const char *
argsmgr_getc(const CapArgsMgr *self, int32_t idx) {
    return cstrarr_getc(self->args, idx);
}

bool
CapArgsMgr_ContainsAll(const CapArgsMgr *self, const char *target) {
    bool contain = true;
    for (int32_t i = 0; i < cstrarr_len(self->args); ++i) {
        const char *arg = cstrarr_getc(self->args, i);
        if (!strstr(target, arg)) {
            contain = false;
            break;
        }
    }

    return contain;
}
