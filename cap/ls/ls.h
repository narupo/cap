#pragma once

#include <getopt.h>

#include <pad/lib/memory.h>
#include <pad/lib/file.h>
#include <pad/lib/cstring_array.h>
#include <pad/lib/term.h>

#include <cap/core/util.h>
#include <cap/core/config.h>
#include <cap/core/symlink.h>

struct CapLsCmd;
typedef struct CapLsCmd CapLsCmd;

void
CapLsCmd_Del(CapLsCmd *self);

CapLsCmd *
CapLsCmd_New(const CapConfig *config, int argc, char **argv);

int
CapLsCmd_Run(CapLsCmd *self);
