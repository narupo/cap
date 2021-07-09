#include <stdio.h>
#include <getopt.h>
#include <string.h>

#include <pad/lib/memory.h>
#include <pad/lib/file.h>
#include <pad/lib/string.h>
#include <pad/lib/cstring.h>
#include <pad/lib/cmdline.h>
#include <pad/lib/cl.h>
#include <pad/lib/term.h>
#include <pad/lang/kit.h>

#include <cap/core/constant.h>
#include <cap/core/util.h>
#include <cap/core/config.h>
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
#include <cap/find/find.h>

/**
 * structure and type of command
 */
struct CapShCmd;
typedef struct CapShCmd CapShCmd;

/**
 * destruct command
 *
 * @param[in] self pointer to CapShCmd
 */
void
CapShCmd_Del(CapShCmd *self);

/**
 * construct command
 *
 * @param[in] config pointer to CapConfig (writable)
 * @param[in] argc   number of arguments
 * @param[in] argv   reference to array of arguments
 *
 * @return success to pointer to CapShCmd
 * @return failed to NULL
 */
CapShCmd *
CapShCmd_New(CapConfig *config, int argc, char **argv);

/**
 * run command
 *
 * @param[in] self pointer to CapShCmd
 *
 * @return success to number of 0
 * @return failed to number of not 0
 */
int
CapShCmd_Run(CapShCmd *self);
