/**
 * Cap
 *
 * License: MIT
 *  Author: narupo
 *   Since: 2016, 2018
 */
#pragma once

#include <pad/lib/memory.h>
#include <pad/lib/file.h>

#include <cap/core/config.h>
#include <cap/core/util.h>
#include <cap/core/symlink.h>

struct CapCdCmd;
typedef struct CapCdCmd CapCdCmd;

void
CapCdCmd_Del(CapCdCmd *self);

CapCdCmd *
CapCdCmd_New(const CapConfig *config, int argc, char **argv);

int
CapCdCmd_Run(CapCdCmd *self);
