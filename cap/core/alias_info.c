#include <core/alias_info.h>

struct CapAliasInfo {
    PadDict *key_val_map;
    PadDict *key_desc_map;
};

void
CapAliasInfo_Del(PadAliasInfo *self) {
    if (!self) {
        return;
    }

    dict_del(self->key_val_map);
    dict_del(self->key_desc_map);
    free(self);
}

PadAliasInfo *
CapAliasInfo_New(void) {
    PadAliasInfo *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->key_val_map = PadDict_New(128);
    if (!self->key_val_map) {
        CapAliasInfo_Del(self);
        return NULL;
    }

    self->key_desc_map = PadDict_New(128);
    if (!self->key_desc_map) {
        CapAliasInfo_Del(self);
        return NULL;
    }

    return self;
}

PadAliasInfo *
CapAliasInfo_DeepCopy(const PadAliasInfo *other) {
    if (!other) {
        return NULL;
    }

    PadAliasInfo *self = PadMem_Calloc(1, sizeof(*self));
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

PadAliasInfo *
CapAliasInfo_ShallowCopy(const PadAliasInfo *other) {
    if (!other) {
        return NULL;
    }

    PadAliasInfo *self = PadMem_Calloc(1, sizeof(*self));
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
CapAliasInfo_GetcValue(const PadAliasInfo *self, const char *key) {
   const PadDictItem *item = PadDict_Getc(self->key_val_map, key);
   if (!item) {
       return NULL;
   }

   return item->value;
}

const char *
CapAliasInfo_GetcDesc(const PadAliasInfo *self, const char *key) {
   const PadDictItem *item = PadDict_Getc(self->key_desc_map, key);
   if (!item) {
       return NULL;
   }

   return item->value;
}

PadAliasInfo *
CapAliasInfo_SetValue(PadAliasInfo *self, const char *key, const char *value) {
    PadDict *result = PadDict_Set(self->key_val_map, key, value);
    if (!result) {
        return NULL;
    }

    return self;
}

PadAliasInfo *
CapAliasInfo_SetDesc(PadAliasInfo *self, const char *key, const char *desc) {
    PadDict *result = PadDict_Set(self->key_desc_map, key, desc);
    if (!result) {
        return NULL;
    }

    return self;
}

void
CapAliasInfo_Clear(PadAliasInfo *self) {
    PadDict_Clear(self->key_val_map);
    PadDict_Clear(self->key_desc_map);
}

const PadDict *
PadAliasInfo_GetcKeyValueMap(const PadAliasInfo *self) {
    return self->key_val_map;
}

const PadDict *
CapAliasInfo_GetcKeyDescMap(const PadAliasInfo *self) {
    return self->key_desc_map;
}
