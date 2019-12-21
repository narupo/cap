#pragma once

// TODO: test

#include <stdint.h>

#include "lib/memory.h"
#include "lib/string.h"
#include "lib/dict.h"
#include "core/alias_info.h"
#include "lang/types.h"
#include "lang/object_dict.h"
#include "lang/scope.h"

void
ctx_del(context_t *self);

context_t *
ctx_new(void);

void
ctx_clear(context_t *self);

context_t *
ctx_set_alias(context_t *self, const char *key, const char *val, const char *desc);

context_t *
ctx_set_config(context_t *self, const char *key, const char *val);

const char *
ctx_get_alias_value(context_t *self, const char *key);

context_t *
ctx_pushb_buf(context_t *self, const char *str);

const char *
ctx_getc_buf(const context_t *self);

void
ctx_import_alias(context_t *self);

void
ctx_import_config(context_t *self);

bool
ctx_get_imported_alias(const context_t *self);

bool
ctx_get_imported_config(const context_t *self);

const alinfo_t *
ctx_getc_alinfo(const context_t *self);

const dict_t *
ctx_getc_confmap(const context_t *self);

object_dict_t *
ctx_get_varmap(context_t *self);

object_dict_t *
ctx_get_varmap_at_global(context_t *self);

bool
ctx_get_do_break(const context_t *self);

void
ctx_set_do_break(context_t *self, bool do_break);

bool
ctx_get_do_continue(const context_t *self);

void
ctx_set_do_continue(context_t *self, bool do_continue);

bool
ctx_get_do_return(const context_t *self);

void
ctx_set_do_return(context_t *self, bool do_return);

void
ctx_clear_jump_flags(context_t *self);

void
ctx_pushb_scope(context_t *self);

void
ctx_popb_scope(context_t *self);

object_t *
ctx_find_var_ref(context_t *self, const char *key);
