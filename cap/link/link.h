#pragma once

#include <getopt.h>
#include <stdbool.h>

#include <lib/memory.h>
#include <lib/file.h>
#include <lib/string.h>
#include <core/constant.h>
#include <core/util.h>
#include <core/config.h>
#include <core/symlink.h>

struct CapLinkCmd;
typedef struct CapLinkCmd CapLinkCmd;

void
CapLinkCmd_Del(CapLinkCmd *self);

CapLinkCmd *
CapLinkCmd_New(const CapConfig *config, int argc, char **argv);

int
CapLinkCmd_Run(CapLinkCmd *self);
