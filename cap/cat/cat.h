#pragma once

#include <getopt.h>

#include <pad/lib/memory.h>
#include <pad/lib/file.h>
#include <pad/lib/string.h>
#include <pad/lib/cstring.h>
#include <pad/core/config.h>
#include <pad/core/util.h>
#include <pad/lang/kit.h>

#include <cap/core/constant.h>
#include <cap/core/config.h>
#include <cap/core/symlink.h>
#include <cap/lang/kit.h>

struct CapCatCmd;
typedef struct CapCatCmd CapCatCmd;

/**
 * destruct command
 *
 * @param[in] *self
 */
void
CapCatCmd_Del(CapCatCmd *self);

/**
 * construct command
 *
 * @param[in] *config
 * @param[in] argc
 * @param[in] **argv
 *
 * @return success to pointer_t CapCatCmd
 * @return failed to NULL
 */
CapCatCmd *
CapCatCmd_New(const CapConfig *config, int argc, char **argv);

/**
 * run command
 *
 * @param[in] *self
 *
 * @return success to 0. failed to other
 */
int
CapCatCmd_Run(CapCatCmd *self);

/**
 * set debug value
 *
 * @param[in] *self
 * @param[in] debug
 */
void
CapCatCmd_SetDebug(CapCatCmd *self, bool debug);
