#pragma once

#include <getopt.h>
#include <stdbool.h>

#include <pad/lib/memory.h>
#include <pad/lib/file.h>
#include <pad/lib/string.h>

#include <cap/core/constant.h>
#include <cap/core/util.h>
#include <cap/core/config.h>
#include <cap/core/symlink.h>

struct CapLinkCmd;
typedef struct CapLinkCmd CapLinkCmd;

void
CapLinkCmd_Del(CapLinkCmd *self);

CapLinkCmd *
CapLinkCmd_New(const CapConfig *config, int argc, char **argv);

int
CapLinkCmd_Run(CapLinkCmd *self);
