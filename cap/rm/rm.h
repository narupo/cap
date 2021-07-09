#pragma once

#include <getopt.h>
#include <string.h>

#include <pad/lib/memory.h>
#include <pad/lib/file.h>
#include <pad/lib/string.h>
#include <pad/lib/cstring.h>

#include <cap/core/constant.h>
#include <cap/core/util.h>
#include <cap/core/config.h>
#include <cap/core/symlink.h>

typedef enum {
    CAP_RMCMD_ERR__NOERR = 0,
    CAP_RMCMD_ERR__UNKNOWN_OPTS,
    CAP_RMCMD_ERR__PARSE_OPTS,
    CAP_RMCMD_ERR__OPENDIR,
    CAP_RMCMD_ERR__SOLVEPATH,
    CAP_RMCMD_ERR__REMOVE_FILE,
    CAP_RMCMD_ERR__READ_CD,
    CAP_RMCMD_ERR__OUTOFHOME,
    CAP_RMCMD_ERR__CLOSEDIR,
} CapRmCmdErrno;

struct CapRmCmd;
typedef struct CapRmCmd CapRmCmd;

void
CapRmCmd_Del(CapRmCmd *self);

CapRmCmd *
CapRmCmd_New(const CapConfig *config, int argc, char **argv);

int
CapRmCmd_Run(CapRmCmd *self);

CapRmCmdErrno
CapRmCmd_Errno(const CapRmCmd *self);

const char *
CapRmCmd_What(const CapRmCmd *self);
