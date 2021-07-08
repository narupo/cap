#include <getopt.h>
#include <string.h>
#include <stdint.h>
#include <lib/memory.h>
#include <lib/file.h>
#include <lib/string.h>
#include <core/constant.h>
#include <core/util.h>
#include <core/config.h>
#include <core/error_stack.h>

/**
 * structure and type of command
 */
struct insert;
typedef struct insert insertcmd_t;

/**
 * destruct command
 *
 * @param[in] self pointer to insertcmd_t
 */
void
insertcmd_del(insertcmd_t *self);

/**
 * construct command
 *
 * @param[in] move_config reference to config_t 
 * @param[in] argc        number of arguments
 * @param[in] move_argv   reference to array of arguments 
 *
 * @return success to pointer to insertcmd_t
 * @return failed to NULL
 */
insertcmd_t *
insertcmd_new(const config_t *config, int argc, char **argv);

/**
 * run command
 *
 * @param[in] self pointer to insertcmd_t
 *
 * @return success to number of 0
 * @return failed to number of not 0
 */
int
insertcmd_run(insertcmd_t *self);