/* alias_info modules is for alias manage in context module 
   alias_info module has key and value, and key and description value */
#pragma once

#include <lib/dict.h>
#include <lib/memory.h>

struct CapAliasInfo;
typedef struct CapAliasInfo CapAliasInfo;

/**
 * destruct alinfo
 *
 * @param[in] *self pointer to CapAliasInfo dynamic allocate memory
 */
void 
CapAliasInfo_Del(CapAliasInfo *self);

/**
 * construct alinfo
 *
 * @return pointer to CapAliasInfo dynamic allocate memory
 */
CapAliasInfo * 
CapAliasInfo_New(void);

CapAliasInfo *
CapAliasInfo_DeepCopy(const CapAliasInfo *other);

CapAliasInfo *
CapAliasInfo_ShallowCopy(const CapAliasInfo *other);

/**
 * get value of alias
 *
 * @param[in] *self pointer to CapAliasInfo dynamic allocate memory
 * @param[in] *key  key value
 *
 * @return found to pointer to string of value
 * @return not found to pointer to NULL
 */
const char * 
CapAliasInfo_GetcValue(const CapAliasInfo *self, const char *key);

/**
 * get description value of alias
 *
 * @param[in] *self pointer to CapAliasInfo dynamic allocate memory
 * @param[in] *key  key value
 *
 * @return found to pointer to string of description value
 * @return not found to pointer to NULL
 */
const char * 
CapAliasInfo_GetcDesc(const CapAliasInfo *self, const char *key);

/**
 * set value
 *
 * @param[in] *self  pointer to CapAliasInfo dynamic allocate memory
 * @param[in] *key   key value
 * @param[in] *value value
 *
 * @return success to pointer to CapAliasInfo 
 * @return failed to pointer to NULL
 */
CapAliasInfo * 
CapAliasInfo_SetValue(CapAliasInfo *self, const char *key, const char *value);

/**
 * set description value
 *
 * @param[in] *self pointer to CapAliasInfo dynamic allocate memory
 * @param[in] *key  key value
 * @param[in] *desc description value
 *
 * @return success to pointer to CapAliasInfo 
 * @return failed to pointer to NULL
 */
CapAliasInfo * 
CapAliasInfo_SetDesc(CapAliasInfo *self, const char *key, const char *desc);

/**
 * clear values
 *
 * @param[in] *self pointer to CapAliasInfo dynamic allocate memory
 */
void
CapAliasInfo_Clear(CapAliasInfo *self);

/**
 * get key and value map (dict) from alinfo
 *
 * @param[in] *self pointer to CapAliasInfo dynamic allocate memory
 *
 * @return pointer to PadDict dynamic allocate memory
 */
const PadDict *
CapAliasInfo_GetcKeyValueMap(const CapAliasInfo *self);

/**
 * get key and description value map (dict) from alinfo
 *
 * @param[in] *self pointer to CapAliasInfo dynamic allocate memory
 *
 * @return pointer to PadDict dynamic allocate memory
 */
const PadDict *
CapAliasInfo_GetcKeyDescMap(const CapAliasInfo *self);
