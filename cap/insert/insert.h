#include <getopt.h>
#include <string.h>
#include <stdint.h>

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
struct CapInsertCmd;
typedef struct CapInsertCmd CapInsertCmd;

/**
 * destruct command
 *
 * @param[in] self pointer to CapInsertCmd
 */
void
CapInsertCmd_Del(CapInsertCmd *self);

/**
 * construct command
 *
 * @param[in] move_config reference to CapConfig 
 * @param[in] argc        number of arguments
 * @param[in] move_argv   reference to array of arguments 
 *
 * @return success to pointer to CapInsertCmd
 * @return failed to NULL
 */
CapInsertCmd *
CapInsertCmd_New(const CapConfig *config, int argc, char **argv);

/**
 * run command
 *
 * @param[in] self pointer to CapInsertCmd
 *
 * @return success to number of 0
 * @return failed to number of not 0
 */
int
CapInsertCmd_Run(CapInsertCmd *self);
