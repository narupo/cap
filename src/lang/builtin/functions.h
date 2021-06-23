#pragma once

#include <core/config.h>
#include <lang/types.h>
#include <lang/object.h>
#include <lang/ast.h>
#include <lang/utils.h>
#include <lang/gc.h>
#include <lang/arguments.h>
#include <lang/tokenizer.h>
#include <lang/compiler.h>
#include <lang/traverser.h>

/**
 * construct module
 *
 * @param[in] *ref_config
 * @param[in] *ref_gc
 *
 * @return
 */
object_t *
builtin_module_new(const config_t *ref_config, gc_t *ref_gc);
