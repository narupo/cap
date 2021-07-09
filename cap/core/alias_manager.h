#pragma once

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <pad/lib/error.h>
#include <pad/lib/memory.h>
#include <pad/lang/gc.h>
#include <pad/lang/tokenizer.h>
#include <pad/lang/ast.h>
#include <pad/lang/compiler.h>
#include <pad/lang/traverser.h>
#include <pad/lang/context.h>

#include <cap/core/constant.h>
#include <cap/core/config.h>
#include <cap/core/util.h>
#include <cap/core/symlink.h>
#include <cap/core/alias_info.h>

struct CapAliasMgr;
typedef struct CapAliasMgr CapAliasMgr;

/**
 * Destruct module
 *
 * @param[in] self pointer to dynamic allocate memory of CapAliasMgr
 */
void
CapAliasMgr_Del(CapAliasMgr *self);

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
CapAliasMgr_FindAliasValue(CapAliasMgr *self, char *dst, uint32_t dstsz, const char *key, int scope);

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
CapAliasMgr_LoadAliasList(CapAliasMgr *self, int scope);

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
CapAliasMgr_HasErr(const CapAliasMgr *self);

/**
 * Clear error 
 *
 * @param[in] self pointer to dynamic allocate memory of CapAliasMgr
 */
void
CapAliasMgr_ClearError(CapAliasMgr *self);

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
CapAliasMgr_GetErrDetail(const CapAliasMgr *self);

/**
 * Get context
 *
 * @param[in] self pointer to dynamic allocate memory of CapAliasMgr
 *
 * @return pointer to PadCtx
 */
const PadCtx *
CapAliasMgr_GetcCtx(const CapAliasMgr *self);
