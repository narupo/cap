/**
 * symlinkの仕様
 *
 * Capのシンボリックリンクはファイルで表現される
 * ファイル内にはヘッダーとパスが書かれている
 * たとえば↓のようにである
 *
 *      cap symlink: /path/to/file
 *
 * 'cap symlink:`はヘッダーである
 * Capはファイルにこれが記述されている場合、そのファイルをシンボリックリンクとして判断する
 *
 * ヘッダーに続く文字列はパスである
 * このパスはCapの環境下の*絶対パス*である（ファイルシステム上のパスではない）
 * Capの環境下のパスにすることでCapの移植性と可用性を上げている
 */
#pragma once

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <pad/lib/file.h>
#include <pad/lib/cstring.h>
#include <pad/lib/cstring_array.h>
#include <pad/lib/string.h>

#include <cap/core/constant.h>
#include <cap/core/config.h>

#define CAP_SYMLINK__HEADER "cap symlink:"

/**
 * Follow path for symbolic links and save real path at destination
 *
 * @param[in] *dst     pointer to destination
 * @param[in] dstsz    number of size of destination
 * @param[in] *drtpath string of dirty path
 *
 * @return success to pointer to path, failed to NULL
 */
char *
CapSymlink_FollowPath(const CapConfig *config, char *dst, uint32_t dstsz, const char *drtpath);

/**
 * Normalize dirty path and save normalized path to destination
 *
 * Like the below
 *
 * dirty path: /path/to/../dir
 *  norm path: /path/dir
 *
 * dirty path: /path/../to/../dir
 *  norm path: /dir
 *
 * @param[in] *dst     pointer to destination
 * @param[in] dstsz    number of size of destination
 * @param[in] *drtpath string of dirty path
 *
 * @return success to pointer to path, failed to NULL
 */
char *
CapSymlink_NormPath(const CapConfig *config, char *dst, uint32_t dstsz, const char *drtpath);

/**
 * Check file is Cap's symbolic link
 *
 * @param[in] *path pointer to path
 *
 * @return file is link to true, else false
 */
bool
CapSymlink_IsLinkFile(const char *path);
