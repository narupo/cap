#include <core/alias_info.h>

struct alias_info {
    PadDict *key_val_map;
    PadDict *key_desc_map;
};

void
alinfo_del(PadAliasInfo *self) {
    if (!self) {
        return;
    }

    dict_del(self->key_val_map);
    dict_del(self->key_desc_map);
    free(self);
}

PadAliasInfo *
alinfo_new(void) {
    PadAliasInfo *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->key_val_map = dict_new(128);
    if (!self->key_val_map) {
        alinfo_del(self);
        return NULL;
    }

    self->key_desc_map = dict_new(128);
    if (!self->key_desc_map) {
        alinfo_del(self);
        return NULL;
    }

    return self;
}

PadAliasInfo *
alinfo_deep_copy(const PadAliasInfo *other) {
    if (!other) {
        return NULL;
    }

    PadAliasInfo *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->key_val_map = dict_deep_copy(other->key_val_map);
    if (!self->key_val_map) {
        alinfo_del(self);
        return NULL;
    }

    self->key_desc_map = dict_deep_copy(other->key_desc_map);
    if (!self->key_desc_map) {
        alinfo_del(self);
        return NULL;
    }

    return self;
}

PadAliasInfo *
alinfo_shallow_copy(const PadAliasInfo *other) {
    if (!other) {
        return NULL;
    }

    PadAliasInfo *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->key_val_map = dict_shallow_copy(other->key_val_map);
    if (!self->key_val_map) {
        alinfo_del(self);
        return NULL;
    }

    self->key_desc_map = dict_shallow_copy(other->key_desc_map);
    if (!self->key_desc_map) {
        alinfo_del(self);
        return NULL;
    }

    return self;  
}

const char *
alinfo_getc_value(const PadAliasInfo *self, const char *key) {
   const dict_item_t *item = dict_getc(self->key_val_map, key);
   if (!item) {
       return NULL;
   }

   return item->value;
}

const char *
alinfo_getc_desc(const PadAliasInfo *self, const char *key) {
   const dict_item_t *item = dict_getc(self->key_desc_map, key);
   if (!item) {
       return NULL;
   }

   return item->value;
}

PadAliasInfo *
alinfo_set_value(PadAliasInfo *self, const char *key, const char *value) {
    PadDict *result = dict_set(self->key_val_map, key, value);
    if (!result) {
        return NULL;
    }

    return self;
}

PadAliasInfo *
alinfo_set_desc(PadAliasInfo *self, const char *key, const char *desc) {
    PadDict *result = dict_set(self->key_desc_map, key, desc);
    if (!result) {
        return NULL;
    }

    return self;
}

void
alinfo_clear(PadAliasInfo *self) {
    dict_clear(self->key_val_map);
    dict_clear(self->key_desc_map);
}

const PadDict *
PadAliasInfo_GetcKeyValueMap(const PadAliasInfo *self) {
    return self->key_val_map;
}

const PadDict *
alinfo_getc_key_desc_map(const PadAliasInfo *self) {
    return self->key_desc_map;
}
