#pragma once

#include <pad/core/config.h>
#include <pad/lib/void_dict.h>
#include <pad/lang/types.h>
#include <pad/lang/object.h>
#include <pad/lang/ast.h>
#include <pad/lang/gc.h>
#include <pad/lang/tokenizer.h>
#include <pad/lang/context.h>
#include <pad/lang/arguments.h>

#include <cap/core/alias_info.h>

const CapAliasInfo *
CapBltAliasMod_GetAliasInfo(const PadCtx *ctx);

/**
 * construct alias module
 *
 * @param[in] *ref_config
 * @param[in] *ref_gc
 *
 * @return
 */
PadObj *
CapBltAliasMod_NewMod(const PadConfig *ref_config, PadGC *ref_gc);
