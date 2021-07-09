#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <locale.h>

#include <pad/lib/string.h>
#include <pad/lib/term.h>

#include <pad/lib/memory.h>
#include <pad/lib/cl.h>
#include <pad/lib/cstring.h>
#include <pad/core/error_stack.h>
#include <pad/core/args.h>
#include <pad/lang/ast.h>
#include <pad/lang/compiler.h>
#include <pad/lang/traverser.h>

#include <cap/core/constant.h>
#include <cap/core/config.h>
#include <cap/core/util.h>
#include <cap/core/alias_manager.h>

#include <cap/home/home.h>
#include <cap/cd/cd.h>
#include <cap/pwd/pwd.h>
#include <cap/ls/ls.h>
#include <cap/cat/cat.h>
#include <cap/run/run.h>
#include <cap/exec/exec.h>
#include <cap/alias/alias.h>
#include <cap/edit/edit.h>
#include <cap/editor/editor.h>
#include <cap/mkdir/mkdir.h>
#include <cap/rm/rm.h>
#include <cap/mv/mv.h>
#include <cap/cp/cp.h>
#include <cap/touch/touch.h>
#include <cap/snippet/snippet.h>
#include <cap/link/link.h>
#include <cap/make/make.h>
#include <cap/cook/cook.h>
#include <cap/sh/sh.h>
#include <cap/find/find.h>
#include <cap/bake/bake.h>
#include <cap/insert/insert.h>
#include <cap/clone/clone.h>
#include <cap/replace/replace.h>
