#pragma once

#include <pad/lib/file.h>
#include <pad/lib/error.h>
#include <pad/lib/cl.h>
#include <pad/lib/cstring_array.h>
#include <pad/lib/unicode.h>
#include <pad/lib/path.h>
#include <pad/core/error_stack.h>
#include <pad/lang/gc.h>
#include <pad/lang/tokenizer.h>
#include <pad/lang/ast.h>
#include <pad/lang/compiler.h>
#include <pad/lang/traverser.h>
#include <pad/lang/context.h>
#include <pad/lang/kit.h>

#include <cap/core/config.h>
#include <cap/exec/exec.h>

PadBltFuncInfo *
CapBltFuncs_GetBltFuncInfos(void);

void
CapBltFuncs_SetCapConfig(const CapConfig *config);
