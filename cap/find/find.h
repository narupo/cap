#include <getopt.h>
#include <string.h>

#include <pad/lib/memory.h>
#include <pad/lib/file.h>
#include <pad/lib/string.h>
#include <pad/lang/context.h>

#include <cap/core/constant.h>
#include <cap/core/util.h>
#include <cap/core/config.h>
#include <cap/core/symlink.h>
#include <cap/core/alias_manager.h>
#include <cap/core/alias_info.h>
#include <cap/find/arguments_manager.h>

/**
 * Structure and type of command
 */
struct CapFindCmd;
typedef struct CapFindCmd CapFindCmd;

/**
 * Destruct command
 *
 * @param[in] self pointer to CapFindCmd
 */
void
CapFindCmd_Del(CapFindCmd *self);

/**
 * Construct command
 *
 * @param[in] move_config reference to CapConfig 
 * @param[in] argc        number of arguments
 * @param[in] move_argv   reference to array of arguments 
 *
 * @return success to pointer to CapFindCmd
 * @return failed to NULL
 */
CapFindCmd *
CapFindCmd_New(const CapConfig *config, int argc, char **argv);

/**
 * Run command
 *
 * @param[in] self pointer to CapFindCmd
 *
 * @return success to number of 0
 * @return failed to number of not 0
 */
int
CapFindCmd_Run(CapFindCmd *self);
