#include <getopt.h>
#include <string.h>

#include <pad/lib/memory.h>
#include <pad/lib/file.h>
#include <pad/lib/cstring.h>
#include <pad/lib/string.h>
#include <pad/lang/tokenizer.h>
#include <pad/lang/ast.h>
#include <pad/lang/compiler.h>
#include <pad/lang/traverser.h>
#include <pad/lang/context.h>
#include <pad/lang/opts.h>

#include <cap/core/constant.h>
#include <cap/core/util.h>
#include <cap/core/config.h>
#include <cap/core/error_stack.h>

/**
 * Structure and type of command
 */
struct CapSnptCmd;
typedef struct CapSnptCmd CapSnptCmd;

/**
 * Destruct command
 *
 * @param[in] self pointer to CapSnptCmd
 */
void
CapSnptCmd_Del(CapSnptCmd *self);

/**
 * Construct command
 *
 * @param[in] config reference to CapConfig
 * @param[in] argc   number of arguments
 * @param[in] argv   reference to array of arguments
 *
 * @return success to pointer to CapSnptCmd
 * @return failed to NULL
 */
CapSnptCmd *
CapSnptCmd_New(const CapConfig *config, int argc, char **argv);

/**
 * Run command
 *
 * @param[in] self pointer to CapSnptCmd
 *
 * @return success to number of 0
 * @return failed to number of not 0
 */
int
CapSnptCmd_Run(CapSnptCmd *self);
