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
#include <pad/lang/ast.h>
#include <pad/lang/compiler.h>
#include <pad/lang/traverser.h>

#include <core/constant.h>
#include <core/config.h>
#include <core/util.h>
#include <core/alias_manager.h>
#include <core/args.h>

#include <home/home.h>
#include <cd/cd.h>
#include <pwd/pwd.h>
#include <ls/ls.h>
#include <cat/cat.h>
#include <run/run.h>
#include <exec/exec.h>
#include <alias/alias.h>
#include <edit/edit.h>
#include <editor/editor.h>
#include <mkdir/mkdir.h>
#include <rm/rm.h>
#include <mv/mv.h>
#include <cp/cp.h>
#include <touch/touch.h>
#include <snippet/snippet.h>
#include <link/link.h>
#include <make/make.h>
#include <cook/cook.h>
#include <sh/sh.h>
#include <find/find.h>
#include <bake/bake.h>
#include <insert/insert.h>
#include <clone/clone.h>
#include <replace/replace.h>
