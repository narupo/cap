#pragma once

#include <lib/memory.h>
#include <core/config.h>
#include <core/error_stack.h>
#include <lang/context.h>
#include <lang/ast.h>
#include <lang/compiler.h>
#include <lang/tokenizer.h>
#include <lang/traverser.h>
#include <lang/gc.h>
#include <lang/opts.h>
#include <lang/types.h>

void
kit_del(kit_t *self);

kit_t *
kit_new(const config_t *config);

kit_t *
kit_new_ref_gc(const config_t *config, gc_t *ref_gc);

kit_t *
kit_compile_from_path(kit_t *self, const char *path);

kit_t *
kit_compile_from_path_args(kit_t *self, const char *path, int argc, char *argv[]);

kit_t *
kit_compile_from_string(kit_t *self, const char *str);

kit_t *
kit_compile_from_string_args(kit_t *self, const char *path, const char *str, int argc, char *argv[]);

const char *
kit_getc_stdout_buf(const kit_t *self);

const char *
kit_getc_stderr_buf(const kit_t *self);

void
kit_clear_context(kit_t *self);

context_t *
kit_get_context(kit_t *self);

bool
kit_has_error_stack(const kit_t *self);

const errstack_t *
kit_getc_error_stack(const kit_t *self);

void
kit_clear_context_buffer(kit_t *self);

void
kit_trace_error(const kit_t *self, FILE *fout);

void
kit_trace_error_debug(const kit_t *self, FILE *fout);
