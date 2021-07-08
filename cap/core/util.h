/**
 * Cap
 *
 * License: MIT
 *  Author: narupo
 *   Since: 2016
 */
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <lib/file.h>
#include <lib/error.h>
#include <lib/cl.h>
#include <lib/cstring_array.h>
#include <lib/unicode.h>
#include <lib/path.h>
#include <core/types.h>
#include <core/constant.h>
#include <core/config.h>
#include <core/error_stack.h>
#include <lang/gc.h>
#include <lang/tokenizer.h>
#include <lang/ast.h>
#include <lang/compiler.h>
#include <lang/traverser.h>
#include <lang/context.h>
#include <run/run.h>

#ifdef PAD__WINDOWS
# include <windows.h>
#endif

/**
 * solve path of comannd line argument
 *
 * like the following
 *
 *     path/to/file  -> /caps/environment/path/to/file
 *     /path/to/file -> /caps/environment/path/to/file
 *     :path/to/file -> /users/file/system/path/to/file
 *
 * @param[in]  *config        pointer to CapConfig read-only
 * @param[out] *dst           pointer to destination
 * @param[in]  dstsz          number of size of destination
 * @param[in]  *caps_arg_path path of command line argument of cap (not contain windows path. cap's path is unix like)
 *
 * @return success to pointer to dst
 * @return failed to pointer to NULL
 */
char *
Cap_SolveCmdlineArgPath(
    const CapConfig *config,
    char *dst,
    int32_t dstsz,
    const char *caps_arg_path
);

/**
 * cap_pathとconfigのscopeから基点となるパスを取得する
 * 取得するパスはconfig->home_pathかconfig->cd_pathのいずれかである
 * cap_pathの先頭がセパレータ、つまりcap_pathが絶対パスであるとき、戻り値はconfig->home_pathである
 * このcap_pathはCap環境上のパスである
 * つまり、cap_pathが絶対パスの場合、cap_path[0]は必ず'/'になる
 * scopeが不正の場合、プログラムを終了する
 *
 * @param[in] *config pointer to config_t
 * @param[in] *cap_path pointer to cap_path
 *
 * @return
 */
const char *
Cap_GetOrigin(const config_t *config, const char *cap_path);

/**
 * Show snippet code by name
 *
 * @param[in] *config reference to config
 * @param[in] *name   snippet name
 * @param[in] argc    number of arguments
 * @param[in] **argv  arguments
 *
 * @return success to 0
 * @return failed to not 0
 */
int
Cap_ExecSnippet(const config_t *config, bool *found, int argc, char *argv[], const char *name);

/**
 * execute program in directory of token of PATH in resource file
 * this function first find to local scope and next to find global scope and execute
 * if program is not found to store a 'false' value at *found variable
 *
 * @param[in] *config
 * @param[in] *found
 * @param[in] argc
 * @param[in] *argv[]
 * @param[in] *cmdname
 */
int
Cap_ExecProg(const config_t *config, bool *found, int argc, char *argv[], const char *cmdname);

/**
 * execute run command with command arguments
 *
 * @param[in] config
 * @param[in] argc
 * @param[in] argv
 *
 * @return success to 0 else other
 */
int
Cap_ExecRun(const config_t *config, int argc, char *argv[]);
