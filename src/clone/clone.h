#include <getopt.h>
#include <string.h>
#include "lib/memory.h"
#include "lib/file.h"
#include "lib/string.h"
#include "core/constant.h"
#include "core/util.h"
#include "core/config.h"

/**
 * structure and type of command
 */
struct clone;
typedef struct clone clonecmd_t;

/**
 * destruct command
 *
 * @param[in] self pointer to clonecmd_t
 */
void
clonecmd_del(clonecmd_t *self);

/**
 * construct command
 *
 * @param[in] move_config reference to config_t 
 * @param[in] argc        number of arguments
 * @param[in] move_argv   reference to array of arguments 
 *
 * @return success to pointer to clonecmd_t
 * @return failed to NULL
 */
clonecmd_t *
clonecmd_new(const config_t *config, int argc, char **argv);

/**
 * run command
 *
 * @param[in] self pointer to clonecmd_t
 *
 * @return success to number of 0
 * @return failed to number of not 0
 */
int
clonecmd_run(clonecmd_t *self);