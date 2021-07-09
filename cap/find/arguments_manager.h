#pragma once

#include <string.h>
#include <stdint.h>

#include "lib/memory.h"
#include "lib/cstring_array.h"

struct arguments_manager;
typedef struct arguments_manager CapArgsMgr;

void
argsmgr_del(CapArgsMgr *self);

CapArgsMgr *
argsmgr_new(char *argv[]);

const char *
argsmgr_getc(const CapArgsMgr *self, int32_t idx);

bool
CapArgsMgr_ContainsAll(const CapArgsMgr *self, const char *target);
