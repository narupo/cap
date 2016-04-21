#ifndef TRASH_H
#define TRASH_H

#include "define.h"
#include "util.h"
#include "term.h"
#include "caperr.h"
#include "config.h"
#include "file.h"
#include "hash.h"
#include "io.h"
#include "csvline.h"
#include "string.h"
#include "strarray.h"

void
trash_usage(void);

int
trash_main(int argc, char* argv[]);

#endif