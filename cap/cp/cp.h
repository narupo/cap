#include <getopt.h>
#include <string.h>

#include <pad/lib/memory.h>
#include <pad/lib/file.h>
#include <pad/lib/string.h>

#include <cap/core/constant.h>
#include <cap/core/util.h>
#include <cap/core/config.h>
#include <cap/core/symlink.h>

/**
 * Structure and type of command
 */
struct CapCpCmd;
typedef struct CapCpCmd CapCpCmd;

/**
 * Destruct command
 *
 * @param[in] self pointer to CapCpCmd
 */
void
CapCpCmd_Del(CapCpCmd *self);

/**
 * Construct command
 *
 * @param[in] config reference to CapConfig 
 * @param[in] argc   number of arguments
 * @param[in] argv   reference to array of arguments 
 *
 * @return success to pointer to CapCpCmd
 * @return failed to NULL
 */
CapCpCmd *
CapCpCmd_New(const CapConfig *config, int argc, char **argv);

/**
 * Run command
 *
 * @param[in] self pointer to CapCpCmd
 *
 * @return success to number of 0
 * @return failed to number of not 0
 */
int
CapCpCmd_Run(CapCpCmd *self);
