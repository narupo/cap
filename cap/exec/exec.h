/**
 * 2020/01/22
 *
 * 現在のexecのパイプや&&は、bashの仕様とは異なった実装になっている
 * リダイレクトが見つかった時点でコマンドラインの解釈は終了するし、&&もグループ分けしていない
 * &&でパイプを含むコマンドのグループ分けが必要かもしれない
 * 現在の用途には足りているので是正しないが、あまり美しい仕様にはなっていないので時間がある時に是正してほしい
 */
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include <pad/lib/memory.h>
#include <pad/lib/file.h>
#include <pad/lib/string.h>
#include <pad/lib/error.h>
#include <pad/lib/cmdline.h>

#include <cap/core/constant.h>
#include <cap/core/util.h>
#include <cap/core/config.h>
#include <cap/core/symlink.h>

/**
 * Structure and type of command
 */
struct CapExecCmd;
typedef struct CapExecCmd CapExecCmd;

/**
 * Destruct command
 *
 * @param[in] self pointer to CapExecCmd
 */
void
CapExecCmd_Del(CapExecCmd *self);

/**
 * Construct command
 *
 * @param[in] move_config pointer to CapConfig with move semantics
 * @param[in] argc        number of arguments
 * @param[in] move_argv   pointer to array of arguments with move semantics
 *
 * @return success to pointer to CapExecCmd
 * @return failed to NULL
 */
CapExecCmd *
CapExecCmd_New(const CapConfig *config, int argc, char **argv);

/**
 * Run command
 *
 * @param[in] self pointer to CapExecCmd
 *
 * @return success to number of 0
 * @return failed to number of not 0
 */
int
CapExecCmd_Run(CapExecCmd *self);
