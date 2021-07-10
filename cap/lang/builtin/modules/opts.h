#pragma once

#include <pad/lib/void_dict.h>
#include <pad/core/config.h>
#include <pad/core/util.h>
#include <pad/lang/types.h>
#include <pad/lang/object.h>
#include <pad/lang/ast.h>
#include <pad/lang/gc.h>
#include <pad/lang/tokenizer.h>
#include <pad/lang/context.h>
#include <pad/lang/arguments.h>
#include <pad/lang/builtin/func_info.h>
#include <pad/lang/builtin/func_info_array.h>

#include <cap/lang/opts.h>

bool
CapBltOptsMod_MoveOpts(void *pkey, CapOpts *move_opts);

/**
 * construct PadOpts module
 *
 * @param[in] *config
 * @param[in] *ref_gc
 *
 * @return
 */
PadObj *
CapBltOptsMod_NewMod(const PadConfig *config, PadGC *ref_gc);
