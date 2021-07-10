#include <cap/core/alias_info.h>

struct CapAliasInfo {
    PadDict *key_val_map;
    PadDict *key_desc_map;
};

void
CapAliasInfo_Del(CapAliasInfo *self) {
    if (!self) {
        return;
    }

    PadDict_Del(self->key_val_map);
    PadDict_Del(self->key_desc_map);
    free(self);
}

CapAliasInfo *
CapAliasInfo_New(void) {
    CapAliasInfo *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->key_val_map = PadDict_New(32);
    if (!self->key_val_map) {
        CapAliasInfo_Del(self);
        return NULL;
    }

    self->key_desc_map = PadDict_New(32);
    if (!self->key_desc_map) {
        CapAliasInfo_Del(self);
        return NULL;
    }

    return self;
}

CapAliasInfo *
CapAliasInfo_DeepCopy(const CapAliasInfo *other) {
    if (!other) {
        return NULL;
    }

    CapAliasInfo *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->key_val_map = PadDict_DeepCopy(other->key_val_map);
    if (!self->key_val_map) {
        CapAliasInfo_Del(self);
        return NULL;
    }

    self->key_desc_map = PadDict_DeepCopy(other->key_desc_map);
    if (!self->key_desc_map) {
        CapAliasInfo_Del(self);
        return NULL;
    }

    return self;
}

CapAliasInfo *
CapAliasInfo_ShallowCopy(const CapAliasInfo *other) {
    if (!other) {
        return NULL;
    }

    CapAliasInfo *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->key_val_map = PadDict_ShallowCopy(other->key_val_map);
    if (!self->key_val_map) {
        CapAliasInfo_Del(self);
        return NULL;
    }

    self->key_desc_map = PadDict_ShallowCopy(other->key_desc_map);
    if (!self->key_desc_map) {
        CapAliasInfo_Del(self);
        return NULL;
    }

    return self;  
}

const char *
CapAliasInfo_GetcValue(const CapAliasInfo *self, const char *key) {
   const PadDictItem *item = PadDict_Getc(self->key_val_map, key);
   if (!item) {
       return NULL;
   }

   return item->value;
}

const char *
CapAliasInfo_GetcDesc(const CapAliasInfo *self, const char *key) {
   const PadDictItem *item = PadDict_Getc(self->key_desc_map, key);
   if (!item) {
       return NULL;
   }

   return item->value;
}

CapAliasInfo *
CapAliasInfo_SetValue(CapAliasInfo *self, const char *key, const char *value) {
    PadDict *result = PadDict_Set(self->key_val_map, key, value);
    if (!result) {
        return NULL;
    }

    return self;
}

CapAliasInfo *
CapAliasInfo_SetDesc(CapAliasInfo *self, const char *key, const char *desc) {
    PadDict *result = PadDict_Set(self->key_desc_map, key, desc);
    if (!result) {
        return NULL;
    }

    return self;
}

void
CapAliasInfo_Clear(CapAliasInfo *self) {
    PadDict_Clear(self->key_val_map);
    PadDict_Clear(self->key_desc_map);
}

const PadDict *
CapAliasInfo_GetcKeyValueMap(const CapAliasInfo *self) {
    return self->key_val_map;
}

const PadDict *
CapAliasInfo_GetcKeyDescMap(const CapAliasInfo *self) {
    return self->key_desc_map;
}
