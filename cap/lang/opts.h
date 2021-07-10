#pragma once

#include <assert.h>

#include <pad/core/util.h>
#include <pad/lib/memory.h>
#include <pad/lib/string.h>
#include <pad/lib/cstring_array.h>
#include <pad/lib/dict.h>

struct CapOpts;
typedef struct CapOpts CapOpts;

/**
 * destruct CapOpts
 * the self of argument is will be free'd
 *
 * @param[in] *self pointer to CapOpts
 */
void
CapOpts_Del(CapOpts *self);

/**
 * construct CapOpts
 * allocate memory and init that memory
 * default constructor
 *
 * @return pointer to CapOpts (dynamic allocated memory)
 */
CapOpts *
CapOpts_New(void);

CapOpts *
CapOpts_DeepCopy(const CapOpts *other);

CapOpts *
CapOpts_ShallowCopy(const CapOpts *other);

/**
 * parse arguments and store values at CapOpts
 *
 * @param[in] *self   pointer to CapOpts
 * @param[in] argc    number of arguments
 * @param[in] *argv[] array of arguments (NULL terminated)
 *
 * @return succes to pointer to the self of argument
 * @return failed to NULL
 */
CapOpts *
CapOpts_Parse(CapOpts *self, int argc, char *argv[]);

/**
 * get element in CapOpts by option name
 * the option name will be without '-' like the 'h' or 'help'
 *
 * @param[in] *self    pointer to CapOpts
 * @param[in] *optname strings of option name
 *
 * @return found to pointer to option value of string in CapOpts
 * @return not found to NULL
 */
const char *
CapOpts_Getc(const CapOpts *self, const char *optname);

/**
 * if CapOpts has option name then return true else return false
 *
 * @param[in] *self    pointer to CapOpts
 * @param[in] *optname striongs of option name
 *
 * @return found option name to return true
 * @return not found option name to return false
 */
bool
CapOpts_Has(const CapOpts *self, const char *optname);

/**
 * get element of arguments
 *
 * @param[in] *self pointer to CapOpts
 * @param[in] idx   number of index
 *
 * @return found to return argument value of strings
 * @return not found to NULL
 */
const char *
CapOpts_GetcArgs(const CapOpts *self, int32_t idx);

/**
 * get arguments length
 *
 * @param[in] *self pointer to CapOpts
 *
 * @return number of arguments length
 */
int32_t
CapOpts_ArgsLen(const CapOpts *self);

/**
 * clear status
 *
 * @param[in] *self
 */
void
CapOpts_Clear(CapOpts *self);

void
CapOpts_Show(CapOpts *self, FILE *fout);
