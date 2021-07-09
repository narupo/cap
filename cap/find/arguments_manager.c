#include "find/arguments_manager.h"

struct arguments_manager {
    PadCStrAry *args;
};

void
argsmgr_del(CapArgsMgr *self) {
    if (!self) {
        return;
    }

    PadCStrAry_Del(self->args);
    free(self);
}

CapArgsMgr *
argsmgr_new(char *argv[]) {
    CapArgsMgr *self = PadMem_ECalloc(1, sizeof(*self));

    self->args = PadCStrAry_New();

    for (char **ap = argv; *ap; ++ap) {
        PadCStrAry_PushBackb(self->args, *ap);
    }

    return self; 
}

const char *
argsmgr_getc(const CapArgsMgr *self, int32_t idx) {
    return PadCStrAry_Getc(self->args, idx);
}

bool
CapArgsMgr_ContainsAll(const CapArgsMgr *self, const char *target) {
    bool contain = true;
    for (int32_t i = 0; i < PadCStrAry_Len(self->args); ++i) {
        const char *arg = PadCStrAry_Getc(self->args, i);
        if (!strstr(target, arg)) {
            contain = false;
            break;
        }
    }

    return contain;
}
