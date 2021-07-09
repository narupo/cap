#include <getopt.h>
#include <string.h>

#include <pad/lib/memory.h>
#include <pad/lib/file.h>
#include <pad/lib/string.h>

#include <cap/core/constant.h>
#include <cap/core/util.h>
#include <cap/core/config.h>
#include <cap/core/error_stack.h>

/**
 * structure and type of command
 */
struct CapReplaceCmd;
typedef struct CapReplaceCmd CapReplaceCmd;

/**
 * destruct command
 *
 * @param[in] self pointer to CapReplaceCmd
 */
void
CapReplaceCmd_Del(CapReplaceCmd *self);

/**
 * construct command
 *
 * @param[in] move_config reference to CapConfig 
 * @param[in] argc        number of arguments
 * @param[in] move_argv   reference to array of arguments 
 *
 * @return success to pointer to CapReplaceCmd
 * @return failed to NULL
 */
CapReplaceCmd *
CapReplaceCmd_New(const CapConfig *config, int argc, char **argv);

/**
 * run command
 *
 * @param[in] self pointer to CapReplaceCmd
 *
 * @return success to number of 0
 * @return failed to number of not 0
 */
int
CapReplaceCmd_Run(CapReplaceCmd *self);
