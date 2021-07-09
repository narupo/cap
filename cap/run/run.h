#pragma once

#include <pad/lib/memory.h>
#include <pad/lib/file.h>
#include <pad/lib/string.h>
#include <pad/lib/cstring.h>
#include <pad/lib/cstring_array.h>

#include <cap/core/constant.h>
#include <cap/core/util.h>
#include <cap/core/config.h>
#include <cap/core/symlink.h>

struct CapRunCmd;
typedef struct CapRunCmd CapRunCmd;

void
CapRunCmd_Del(CapRunCmd *self);

CapRunCmd *
CapRunCmd_New(const CapConfig *config, int argc, char **argv);

int
CapRunCmd_Run(CapRunCmd *self);
