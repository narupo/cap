/**
 * Cap
 *
 * License: MIT
 *  Author: narupo
 *   Since: 2016
 */
#pragma once

#if defined(_WIN32) || defined(_WIN64)
# define CAP_TESTS__WINDOWS
#endif

#define _SVID_SOURCE 1 /* cap: tests: strdup */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <stdarg.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>

#include <pad/lib/cstring_array.h>
#include <pad/lib/string.h>
#include <pad/lib/cstring.h>
#include <pad/lib/file.h>
#include <pad/lib/cl.h>
#include <pad/lib/error.h>
#include <pad/lib/cmdline.h>
#include <pad/lib/unicode.h>
#include <pad/core/error_stack.h>
#include <pad/lang/types.h>
#include <pad/lang/tokens.h>
#include <pad/lang/tokenizer.h>
#include <pad/lang/nodes.h>
#include <pad/lang/ast.h>
#include <pad/lang/compiler.h>
#include <pad/lang/traverser.h>
#include <pad/lang/object.h>
#include <pad/lang/object_array.h>
#include <pad/lang/object_dict.h>
#include <pad/lang/opts.h>
#include <pad/lang/gc.h>

#include <cap/core/util.h>
#include <cap/core/symlink.h>
#include <cap/core/config.h>
#include <cap/core/alias_info.h>
#include <cap/home/home.h>
#include <cap/cd/cd.h>
#include <cap/pwd/pwd.h>
#include <cap/ls/ls.h>
#include <cap/cat/cat.h>
#include <cap/make/make.h>
#include <cap/alias/alias.h>
#include <cap/editor/editor.h>
#include <cap/mkdir/mkdir.h>
#include <cap/rm/rm.h>
#include <cap/mv/mv.h>
#include <cap/cp/cp.h>
#include <cap/touch/touch.h>
#include <cap/snippet/snippet.h>
#include <cap/link/link.h>
#include <cap/bake/bake.h>
#include <cap/replace/replace.h>
