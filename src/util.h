/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#pragma once

/****************************************************
* Util module is allow dependency to other modules. *
****************************************************/

#define _GNU_SOURCE 1 /* cap: util.h: getenv */
#include <stdlib.h>
#include <stdbool.h>

#include "file.h"
#include "error.h"
#include "env.h"
#include "string.h"
#include "cl.h"
#include "array.h"

/**
 * Free argv memory.
 * 
 * @param[in] argc    
 * @param[in] *argv[] 
 */
void
freeargv(int argc, char *argv[]);

/**
 * Show argv values.
 * 
 * @param[in] argc    
 * @param[in] *argv[] 
 */
void
showargv(int argc, char *argv[]);

/**
 * Check path is out of cap's home?
 *
 * @param[in] string path check path
 *
 * @return bool is out of home to true
 * @return bool is not out of home to false
 */
bool
isoutofhome(const char *path);

/**
 * Get random number of range.
 * 
 * @param[in] int min minimum number of range
 * @param[in] int max maximum number of range
 * 
 * @return int random number (n >= min && n <= max)
 */
int
randrange(int min, int max);

/**
 * Wrapper of system(3) for the safe execute.
 *
 * @example safesystem("/bin/sh -c \"date\"");
 *
 * @see system(3)
 */
int
safesystem(const char *cmdline);

struct cap_array *
argsbyoptind(int argc, char *argv[], int optind);
