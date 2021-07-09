#include <cap/find/arguments_manager.h>

struct CapArgsMgr {
    PadCStrAry *args;
};

void
CapArgsMgr_Del(CapArgsMgr *self) {
    if (!self) {
        return;
    }

    PadCStrAry_Del(self->args);
    Pad_SafeFree(self);
}

CapArgsMgr *
CapArgsMgr_New(char *argv[]) {
    CapArgsMgr *self = PadMem_Calloc(1, sizeof(*self));
    if (self == NULL) {
        return NULL;
    }

    self->args = PadCStrAry_New();

    for (char **ap = argv; *ap; ++ap) {
        PadCStrAry_PushBack(self->args, *ap);
    }

    return self; 
}

const char *
CapArgsMgr_Getc(const CapArgsMgr *self, int32_t idx) {
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
