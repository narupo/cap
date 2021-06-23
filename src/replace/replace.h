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
struct replacecmd;
typedef struct replacecmd replacecmd_t;

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
 * @param[in] config      pointer to config_t (read-only)
 * @param[in] argc        number of arguments
 * @param[in] move_argv   pointer to array of arguments 
 *
 * @return success to pointer to replacecmd_t
 * @return failed to NULL
 */
replacecmd_t *
replacecmd_new(const config_t *config, int argc, char **argv);

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
