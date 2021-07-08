#pragma once

#include <getopt.h>
#include <string.h>

#include <pad/lib/memory.h>
#include <pad/lib/file.h>
#include <pad/lib/string.h>
#include <pad/lib/cstring.h>
#include <pad/lang/tokenizer.h>
#include <pad/lang/ast.h>
#include <pad/lang/context.h>

#include <cap/core/util.h>
#include <cap/core/config.h>
#include <cap/core/symlink.h>

struct CapEditCmd;
typedef struct CapEditCmd CapEditCmd;

/**
 * destruct object
 *
 * @param[in|out] *self pointer to CapEditCmd
 */
void
CapEditCmd_Del(CapEditCmd *self);

/**
 * construct object
 *
 * @param[in] *config pointer to CapConfig read-only
 * @param[in] argc    number of arguments
 * @param[in] **argv  arguments
 *
 * @return pointer to CapEditCmd dynamic allocate memory
 */
CapEditCmd * 
CapEditCmd_New(const CapConfig *config, int argc, char **argv);

/**
 * run module
 *
 * @param[in] *self pointer to CapEditCmd
 *
 * @return success to number of 0 else other
 */
int 
CapEditCmd_Run(CapEditCmd *self);

