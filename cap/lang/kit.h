#pragma once

#include <pad/lib/memory.h>
#include <pad/core/error_stack.h>
#include <pad/lang/kit.h>
#include <pad/lang/builtin/functions.h>

#include <cap/core/config.h>
#include <cap/lang/opts.h>
#include <cap/lang/importer.h>
#include <cap/lang/builtin/functions.h>

struct CapKit;
typedef struct CapKit CapKit;

void
CapKit_Del(CapKit *self);

CapKit *
CapKit_New(const CapConfig *config);

CapKit *
CapKit_CompileFromStrArgs(
    CapKit *self,
    const char *prog_fname,
    const char *src,
    int argc,
    char **argv
);

const char *
CapKit_GetcStdoutBuf(const CapKit *self);

PadCtx *
CapKit_GetRefCtx(const CapKit *self);

void
CapKit_Clear(CapKit *self);

const PadErrStack *
CapKit_GetcErrStack(CapKit *self);

