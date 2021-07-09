#include <getopt.h>
#include <string.h>

#include <pad/lib/memory.h>
#include <pad/lib/file.h>
#include <pad/lib/string.h>
#include <pad/core/error_stack.h>

#include <cap/core/constant.h>
#include <cap/core/util.h>
#include <cap/core/config.h>
#include <cap/make/make.h>

/**
 * structure and type of command
 */
struct CapCookCmd;
typedef struct CapCookCmd CapCookCmd;

/**
 * destruct command
 *
 * @param[in] self pointer to CapCookCmd
 */
void
CapCookCmd_Del(CapCookCmd *self);

/**
 * construct command
 *
 * @param[in] config      pointer to CapConfig (read-only)
 * @param[in] argc        number of arguments
 * @param[in] move_argv   pointer to array of arguments 
 *
 * @return success to pointer to CapCookCmd
 * @return failed to NULL
 */
CapCookCmd *
CapCookCmd_New(const CapConfig *config, int argc, char **argv);

/**
 * run command
 *
 * @param[in] self pointer to CapCookCmd
 *
 * @return success to number of 0
 * @return failed to number of not 0
 */
int
CapCookCmd_Run(CapCookCmd *self);
