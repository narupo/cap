#pragma once

#include <getopt.h>
#include <string.h>

#include <pad/lib/memory.h>
#include <pad/lib/file.h>
#include <pad/lib/string.h>

#include <cap/core/constant.h>
#include <cap/core/config.h>
#include <cap/core/util.h>
#include <cap/core/symlink.h>

struct CapMkdirCmd;
typedef struct CapMkdirCmd CapMkdirCmd;

void
CapMkdirCmd_Del(CapMkdirCmd *self);

CapMkdirCmd *
CapMkdirCmd_New(const CapConfig *config, int argc, char **argv);

int
CapMkdirCmd_Run(CapMkdirCmd *self);
