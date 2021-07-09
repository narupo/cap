#pragma once

#include <string.h>
#include <stdint.h>

#include <pad/lib/memory.h>
#include <pad/lib/cstring_array.h>

struct CapArgsMgr;
typedef struct CapArgsMgr CapArgsMgr;

void
CapArgsMgr_Del(CapArgsMgr *self);

CapArgsMgr *
CapArgsMgr_New(char *argv[]);

const char *
CapArgsMgr_Getc(const CapArgsMgr *self, int32_t idx);

bool
CapArgsMgr_ContainsAll(const CapArgsMgr *self, const char *target);
