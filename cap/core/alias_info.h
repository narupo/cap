/* alias_info modules is for alias manage in context module 
   alias_info module has key and value, and key and description value */
#pragma once

#include <lib/dict.h>
#include <lib/memory.h>

struct alias_info;
typedef struct alias_info PadAliasInfo;

/**
 * destruct alinfo
 *
 * @param[in] *self pointer to PadAliasInfo dynamic allocate memory
 */
void 
alinfo_del(PadAliasInfo *self);

/**
 * construct alinfo
 *
 * @return pointer to PadAliasInfo dynamic allocate memory
 */
PadAliasInfo * 
alinfo_new(void);

PadAliasInfo *
alinfo_deep_copy(const PadAliasInfo *other);

PadAliasInfo *
alinfo_shallow_copy(const PadAliasInfo *other);

/**
 * get value of alias
 *
 * @param[in] *self pointer to PadAliasInfo dynamic allocate memory
 * @param[in] *key  key value
 *
 * @return found to pointer to string of value
 * @return not found to pointer to NULL
 */
const char * 
alinfo_getc_value(const PadAliasInfo *self, const char *key);

/**
 * get description value of alias
 *
 * @param[in] *self pointer to PadAliasInfo dynamic allocate memory
 * @param[in] *key  key value
 *
 * @return found to pointer to string of description value
 * @return not found to pointer to NULL
 */
const char * 
alinfo_getc_desc(const PadAliasInfo *self, const char *key);

/**
 * set value
 *
 * @param[in] *self  pointer to PadAliasInfo dynamic allocate memory
 * @param[in] *key   key value
 * @param[in] *value value
 *
 * @return success to pointer to PadAliasInfo 
 * @return failed to pointer to NULL
 */
PadAliasInfo * 
alinfo_set_value(PadAliasInfo *self, const char *key, const char *value);

/**
 * set description value
 *
 * @param[in] *self pointer to PadAliasInfo dynamic allocate memory
 * @param[in] *key  key value
 * @param[in] *desc description value
 *
 * @return success to pointer to PadAliasInfo 
 * @return failed to pointer to NULL
 */
PadAliasInfo * 
alinfo_set_desc(PadAliasInfo *self, const char *key, const char *desc);

/**
 * clear values
 *
 * @param[in] *self pointer to PadAliasInfo dynamic allocate memory
 */
void
alinfo_clear(PadAliasInfo *self);

/**
 * get key and value map (dict) from alinfo
 *
 * @param[in] *self pointer to PadAliasInfo dynamic allocate memory
 *
 * @return pointer to PadDict dynamic allocate memory
 */
const PadDict *
PadAliasInfo_GetcKeyValueMap(const PadAliasInfo *self);

/**
 * get key and description value map (dict) from alinfo
 *
 * @param[in] *self pointer to PadAliasInfo dynamic allocate memory
 *
 * @return pointer to PadDict dynamic allocate memory
 */
const PadDict *
alinfo_getc_key_desc_map(const PadAliasInfo *self);
