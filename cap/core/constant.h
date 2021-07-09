#pragma once

#define _CAP_VERSION "0.35.48"

#if defined(_WIN32) || defined(_WIN64)
# define CAP__WINDOWS 1 /* cap: core/constant.h */
#else
# undef CAP__WINDOWS
#endif

static const int CAP_SCOPE_LOCAL = 1;
static const int CAP_SCOPE_GLOBAL = 2;
