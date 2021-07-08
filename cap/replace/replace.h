#include <getopt.h>
#include <string.h>
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
struct replace;
typedef struct replace replacecmd_t;

/**
 * destruct command
 *
 * @param[in] self pointer to replacecmd_t
 */
void
replacecmd_del(replacecmd_t *self);

/**
 * construct command
 *
 * @param[in] move_config reference to CapConfig 
 * @param[in] argc        number of arguments
 * @param[in] move_argv   reference to array of arguments 
 *
 * @return success to pointer to replacecmd_t
 * @return failed to NULL
 */
replacecmd_t *
replacecmd_new(const CapConfig *config, int argc, char **argv);

/**
 * run command
 *
 * @param[in] self pointer to replacecmd_t
 *
 * @return success to number of 0
 * @return failed to number of not 0
 */
int
replacecmd_run(replacecmd_t *self);