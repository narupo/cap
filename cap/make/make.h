#pragma once

#include <pad/lib/error.h>
#include <pad/lib/string.h>
#include <pad/lib/file.h>
#include <pad/lang/tokenizer.h>
#include <pad/lang/ast.h>
#include <pad/lang/compiler.h>
#include <pad/lang/traverser.h>
#include <pad/lang/context.h>
#include <pad/lang/opts.h>

#include <cap/core/config.h>
#include <cap/core/util.h>
#include <cap/core/symlink.h>
#include <cap/core/error_stack.h>

struct CapMakeCmd;
typedef struct CapMakeCmd CapMakeCmd;

/**
 * destruct command
 * 
 * @param[in] *self pointer to CapMakeCmd (dynamic allocate memory) 
 */
void
CapMakeCmd_Del(CapMakeCmd *self);

/**
 * construct command
 * 
 * @param[in] *config pointer to config
 * @param[in] argc    number of arguments
 * @param[in] **argv  arguments
 * 
 * @return success to pointer to CapMakeCmd (dynamic allocate memory)
 * @return failed to NULL
 */
CapMakeCmd *
CapMakeCmd_New(const CapConfig *config, int argc, char **argv);

/**
 * run command
 * 
 * @param[in] *self 
 * 
 * @return success to 0, else other
 */
int
CapMakeCmd_Run(CapMakeCmd *self);

/**
 * make script or stdin from program arguments
 * 
 * @param[in] *config    pointer to CapConfig (read-only)
 * @param[out] *errstack pointer to PadErrStack (writeable)
 * @param[in] argc       number of arguments
 * @param[in] *argv[]    arguments
 * @param[in] solve_path if want to solve path then true else false
 * 
 * @return success to 0, else other
 */
int
CapMakeCmd_MakeFromArgs(
    const CapConfig *config,
    PadErrStack *errstack,
    int argc,
    char *argv[],
    bool solve_path
);
