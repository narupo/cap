#include <getopt.h>
#include <string.h>

#include <pad/lib/memory.h>
#include <pad/lib/file.h>
#include <pad/lib/string.h>
#include <pad/core/util.h>
#include <pad/core/config.h>
#include <pad/core/error_stack.h>

#include <cap/core/config.h>
#include <cap/core/constant.h>
#include <cap/make/make.h>

/**
 * structure and type of command
 */
struct CapBakeCmd;
typedef struct CapBakeCmd CapBakeCmd;

/**
 * destruct command
 *
 * @param[in] self pointer to CapBakeCmd
 */
void
CapBakeCmd_Del(CapBakeCmd *self);

/**
 * construct command
 *
 * @param[in] config      pointer to CapConfig (read-only)
 * @param[in] argc        number of arguments
 * @param[in] move_argv   pointer to array of arguments 
 *
 * @return success to pointer to CapBakeCmd
 * @return failed to NULL
 */
CapBakeCmd *
CapBakeCmd_New(const CapConfig *config, int argc, char **argv);

/**
 * run command
 *
 * @param[in] self pointer to CapBakeCmd
 *
 * @return success to number of 0
 * @return failed to number of not 0
 */
int
CapBakeCmd_Run(CapBakeCmd *self);
