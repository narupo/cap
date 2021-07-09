#include <getopt.h>
#include <string.h>

#include <pad/core/config.h>
#include <pad/lib/memory.h>
#include <pad/lib/file.h>
#include <pad/lib/string.h>

#include <cap/core/constant.h>
#include <cap/core/util.h>
#include <cap/core/config.h>

/**
 * structure and type of command
 */
struct CapCloneCmd;
typedef struct CapCloneCmd CapCloneCmd;

/**
 * destruct command
 *
 * @param[in] self pointer to CapCloneCmd
 */
void
CapCloneCmd_Del(CapCloneCmd *self);

/**
 * construct command
 *
 * @param[in] config      reference to CapConfig 
 * @param[in] argc        number of arguments
 * @param[in] move_argv   reference to array of arguments 
 *
 * @return success to pointer to CapCloneCmd
 * @return failed to NULL
 */
CapCloneCmd *
CapCloneCmd_New(const CapConfig *config, int argc, char **argv);

/**
 * run command
 *
 * @param[in] self pointer to CapCloneCmd
 *
 * @return success to number of 0
 * @return failed to number of not 0
 */
int
CapCloneCmd_Run(CapCloneCmd *self);
