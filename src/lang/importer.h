#pragma once

#include <core/config.h>
#include <core/util.h>
#include <lang/context.h>
#include <lang/ast.h>
#include <lang/compiler.h>
#include <lang/tokenizer.h>
#include <lang/traverser.h>
#include <lang/gc.h>
#include <lang/opts.h>
#include <lang/object_dict.h>
#include <lang/object_array.h>

struct importer;
typedef struct importer importer_t;

/**
 * destruct object
 *
 * @param[out] *self poitner to importer_t
 */
void
importer_del(importer_t *self);

/**
 * construct object
 *
 * @return pointer to importer_t
 */
importer_t *
importer_new(const config_t *ref_config);

/**
 * import module from path as alias
 *
 *      import "path" as mod
 * 
 * @param[in] *self      
 * @param[in] *ref_gc    
 * @param[in] *dstvarmap 
 * @param[in] *path      
 * @param[in] *alias     
 * 
 * @return 
 */
importer_t * 
importer_import_as(
    importer_t *self,
    gc_t *ref_gc,
    const ast_t *ref_ast,
    context_t *dstctx,
    const char *path,
    const char *alias
);

/**
 * import objects from path 
 *
 *      from "path" import ( f1, f2 )
 * 
 * @param[in] *self    
 * @param[in] *ref_gc  
 * @param[in] *ref_ast 
 * @param[in] *dstctx  
 * @param[in] *path    
 * @param[in] *vars    
 * 
 * @return 
 */
importer_t * 
importer_from_import(
    importer_t *self,
    gc_t *ref_gc,
    const ast_t *ref_ast,
    context_t *dstctx,
    const char *path,
    object_array_t *vars
);

/**
 * set error string
 *
 * @param[in] *self 
 * @param[in] *fmt  
 * @param[in] ...   
 */
void 
importer_set_error(importer_t *self, const char *fmt, ...);

/**
 * get error string
 *
 * @param[in] *self 
 *
 * @return 
 */
const char * 
importer_getc_error(const importer_t *self);