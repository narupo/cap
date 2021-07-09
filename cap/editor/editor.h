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
struct CapEditorCmd;
typedef struct CapEditorCmd CapEditorCmd;

/**
 * destruct command
 *
 * @param[in] self pointer to CapEditorCmd
 */
void
CapEditorCmd_Del(CapEditorCmd *self);

/**
 * construct command
 *
 * @param[in] config reference to CapConfig 
 * @param[in] argc   number of arguments
 * @param[in] argv   reference to array of arguments 
 *
 * @return success to pointer to CapEditorCmd
 * @return failed to NULL
 */
CapEditorCmd *
CapEditorCmd_New(const CapConfig *config, int argc, char **argv);

/**
 * run command
 *
 * @param[in] self pointer to CapEditorCmd
 *
 * @return success to number of 0
 * @return failed to number of not 0
 */
int
CapEditorCmd_Run(CapEditorCmd *self);
