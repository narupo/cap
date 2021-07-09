/**
 * Cap
 *
 * License: MIT
 *  Author: narupo
 *   Since: 2016, 2018
 */
#pragma once

#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include <pad/lib/memory.h>
#include <pad/lib/error.h>
#include <pad/lib/file.h>
#include <pad/lib/string.h>

#include <cap/core/util.h>
#include <cap/core/config.h>

struct CapPwdCmd;
typedef struct CapPwdCmd CapPwdCmd;

void
CapPwdCmd_Del(CapPwdCmd *self);

CapPwdCmd *
CapPwdCmd_New(const CapConfig *config, int argc, char **argv);

int
CapPwdCmd_Run(CapPwdCmd *self);
