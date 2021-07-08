#include <getopt.h>
#include <string.h>
#include <lib/memory.h>
#include <lib/file.h>
#include <lib/string.h>
#include <core/constant.h>
#include <core/util.h>
#include <core/config.h>
#include <core/error_stack.h>
#include <make/make.h>

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
