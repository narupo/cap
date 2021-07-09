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

#include <pad/lib/file.h>
#include <pad/lib/error.h>
#include <pad/lib/cl.h>
#include <pad/lib/cstring_array.h>
#include <pad/lib/unicode.h>
#include <pad/lib/path.h>
#include <pad/core/error_stack.h>
#include <pad/lang/gc.h>
#include <pad/lang/tokenizer.h>
#include <pad/lang/ast.h>
#include <pad/lang/compiler.h>
#include <pad/lang/traverser.h>
#include <pad/lang/context.h>

#include <cap/core/types.h>
#include <cap/core/constant.h>
#include <cap/core/config.h>
#include <cap/run/run.h>

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
 * @param[in] *config pointer to CapConfig
 * @param[in] *cap_path pointer to cap_path
 *
 * @return
 */
const char *
Cap_GetOrigin(const CapConfig *config, const char *cap_path);

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
Cap_ExecSnippet(const CapConfig *config, bool *found, int argc, char *argv[], const char *name);

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
Cap_ExecProg(const CapConfig *config, bool *found, int argc, char *argv[], const char *cmdname);

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
Cap_ExecRun(const CapConfig *config, int argc, char *argv[]);

/**
 * If argpath is out of home of Cap then return true else return false
 * 
 * @param[in] *homepath 
 * @param[in] *argpath  
 * 
 * @return true|false
 */
bool
Cap_IsOutOfHome(const char *homepath, const char *argpath);

