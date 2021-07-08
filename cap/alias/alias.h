#pragma once

#include <getopt.h>
#include <string.h>
#include <stdbool.h>

#include <pad/lib/memory.h>
#include <pad/lib/file.h>
#include <pad/lib/cstring_array.h>
#include <pad/lib/term.h>
#include <pad/core/util.h>

#include <cap/core/constant.h>
#include <cap/core/config.h>
#include <cap/core/alias_manager.h>
#include <cap/core/alias_info.h>

struct CapAlCmd;
typedef struct CapAlCmd CapAlCmd;

/**
 * destruct object
 *
 * @param[in] *self pointer to CapAlCmd
 */
void
CapAlCmd_Del(CapAlCmd *self);

/**
 * construct object
 *
 * @param[in] *config pointer to config_t read-only
 * @param[in] argc    number of length of arguments
 * @param[in] **argv  arguments
 *
 * @return pointer to CapAlCmd dynamic allocate memory
 */
CapAlCmd *
CapAlCmd_New(const config_t *config, int argc, char **argv);

/**
 * run object
 *
 * @param[in] *self pointer to CapAlCmd
 *
 * @return success to number of 0 else other
 */
int
CapAlCmd_Run(CapAlCmd *self);
