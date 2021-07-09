#pragma once

#include <getopt.h>
#include <string.h>
    
#include <pad/lib/memory.h>
#include <pad/lib/file.h>
#include <pad/lib/string.h>

#include <cap/core/constant.h>
#include <cap/core/util.h>
#include <cap/core/config.h>
#include <cap/core/symlink.h>

struct CapMvCmd;
typedef struct CapMvCmd CapMvCmd;

/**
 * destruct object
 *
 * @param[in] *self pointer to CapMvCmd
 */
void 
CapMvCmd_Del(CapMvCmd *self);

/**
 * construct object
 *
 * @param[in] *config pointer to CapConfig
 * @param[in] argc    number of length of arguments
 * @param[in] **argv  arguments
 *
 * @return pointer to CapMvCmd dynamic allocate memory
 */
CapMvCmd * 
CapMvCmd_New(CapConfig *config, int argc, char **argv);

/**
 * run object
 *
 * @param[in] *self pointer to CapMvCmd
 *
 * @return success to number of 0 else other
 */
int 
CapMvCmd_Run(CapMvCmd *self);

