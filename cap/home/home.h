#pragma once

#include <pad/lib/memory.h>
#include <pad/lib/file.h>
#include <cap/core/util.h>
#include <cap/core/config.h>

struct CapHomeCmd;
typedef struct CapHomeCmd CapHomeCmd;

void
CapHomeCmd_Del(CapHomeCmd *self);

CapHomeCmd *
CapHomeCmd_New(const CapConfig *config, int argc, char **argv);

int
CapHomeCmd_Run(CapHomeCmd *self);
