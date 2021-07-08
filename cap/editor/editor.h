#include <getopt.h>
#include <string.h>

#include <lib/memory.h>
#include <lib/file.h>
#include <lib/string.h>
#include <core/constant.h>
#include <core/util.h>
#include <core/config.h>

/**
 * structure and type of command
 */
struct editorcmd;
typedef struct editorcmd editorcmd_t;

/**
 * destruct command
 *
 * @param[in] self pointer to editorcmd_t
 */
void
editorcmd_del(editorcmd_t *self);

/**
 * construct command
 *
 * @param[in] config reference to CapConfig 
 * @param[in] argc   number of arguments
 * @param[in] argv   reference to array of arguments 
 *
 * @return success to pointer to editorcmd_t
 * @return failed to NULL
 */
editorcmd_t *
editorcmd_new(const CapConfig *config, int argc, char **argv);

/**
 * run command
 *
 * @param[in] self pointer to editorcmd_t
 *
 * @return success to number of 0
 * @return failed to number of not 0
 */
int
editorcmd_run(editorcmd_t *self);
