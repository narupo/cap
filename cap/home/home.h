#pragma once

#include <lib/memory.h>
#include <lib/file.h>
#include <core/util.h>
#include <core/config.h>

struct CapHomeCmd;
typedef struct CapHomeCmd CapHomeCmd;

void
CapHomeCmd_Del(CapHomeCmd *self);

CapHomeCmd *
CapHomeCmd_New(const CapConfig *config, int argc, char **argv);

int
CapHomeCmd_Run(CapHomeCmd *self);
