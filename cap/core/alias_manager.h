#pragma once

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <lib/error.h>
#include <lib/memory.h>
#include <core/constant.h>
#include <core/config.h>
#include <core/util.h>
#include <core/symlink.h>
#include <core/alias_info.h>
#include <lang/gc.h>
#include <lang/tokenizer.h>
#include <lang/ast.h>
#include <lang/compiler.h>
#include <lang/traverser.h>
#include <lang/context.h>

struct alias_manager;
typedef struct alias_manager CapAliasMgr;

/**
 * Destruct module
 *
 * @param[in] self pointer to dynamic allocate memory of CapAliasMgr
 */
void
almgr_del(CapAliasMgr *self);

/**
 * Construct module
 *
 * @param[in] config read-only pointer to CapConfig
 *
 * @return success to pointer to dynamic allocate memory of CapAliasMgr
 * @return failed to pointer to NULL 
 */
CapAliasMgr *
CapAliasMgr_New(const CapConfig *config);

/**
 * Find alias value by key and scope
 *
 * @param[in] self pointer to dynamic allocate memory of CapAliasMgr
 * @param[in] dst pointer to destination
 * @param[in] dstsz number of size of destination
 * @param[in] key string of key
 * @param[in] scope number of scope (@see constant.h)
 *
 * @return found to pointer to dynamic allocate memory of CapAliasMgr
 * @return not found to pointer to NULL
 */
CapAliasMgr *
almgr_find_alias_value(CapAliasMgr *self, char *dst, uint32_t dstsz, const char *key, int scope);

/**
 * Load alias list by scope
 *
 * @param[in] self pointer to dynamic allocate memory of CapAliasMgr
 * @param[in] scope number of scope of environment
 *
 * @return success to pointer to dynamic allocate memory of CapAliasMgr
 * @return failed to NULL
 */
CapAliasMgr *
almgr_load_alias_list(CapAliasMgr *self, int scope);

/**
 * Load alias list by path
 *
 * @param[in] self pointer to CapAliasMgr
 * @param[in] path path on file system
 *
 * @return success to pointer to CapAliasMgr
 * @return failed to NULL
 */
CapAliasMgr *
CapAliasMgr_LoadPath(CapAliasMgr *self, const char *path);

/**
 * Check if has error
 *
 * @param[in] self pointer to dynamic allocate memory of CapAliasMgr
 *
 * @return if has error to true
 * @return if not has error to false
 */
bool
almgr_has_error(const CapAliasMgr *self);

/**
 * Clear error 
 *
 * @param[in] self pointer to dynamic allocate memory of CapAliasMgr
 */
void
CapAliasMgr_Clear_error(CapAliasMgr *self);

/**
 * Clear status
 *
 * @param[in] self pointer to CapAliasMgr
 */
void
CapAliasMgr_Clear(CapAliasMgr *self);

/**
 * Get error detail
 *
 * @param[in] self pointer to dynamic allocate memory of CapAliasMgr
 *
 * @return pointer to string of error detail
 */
const char *
almgr_get_error_detail(const CapAliasMgr *self);

/**
 * Get context
 *
 * @param[in] self pointer to dynamic allocate memory of CapAliasMgr
 *
 * @return pointer to PadCtx
 */
const PadCtx *
CapAliasMgr_GetcContext(const CapAliasMgr *self);
