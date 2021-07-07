#pragma once

#include <pad/core/config.h>

#include <lib/memory.h>
#include <lib/file.h>
#include <lib/path.h>
#include <core/constant.h>
#include <core/error_stack.h>

typedef struct config {
    PadConfig *pad_config;
    errstack_t *errstack;  // error stack for error handling
    int scope;  // @see constant.h for CAP_SCOPE_*
    int recursion_count;  // count of recursion of call to app
    char line_encoding[32+1];  // line encoding "cr" | "crlf" | "lf"
    char var_cd_path[FILE_NPATH];  // path of variable of cd on file system
    char var_home_path[FILE_NPATH];  // path of variable of home on file system
    char var_editor_path[FILE_NPATH];  // path of variable of editor on file system
    char cd_path[FILE_NPATH];  // value of cd
    char home_path[FILE_NPATH];  // value of home
    char editor[FILE_NPATH];  // value of editor
    char codes_dir_path[FILE_NPATH];  // snippet codes directory path
    char std_lib_dir_path[FILE_NPATH];  // standard libraries directory path
} config_t;

/**
 * destruct config_t
 * 
 * @param[in] *self 
 */
void
config_del(config_t *self);

/**
 * construct config_t
 * 
 * @return pointer to config_t dynamic allocate memory
 */
config_t *
config_new(void);

/**
 * initialize config_t
 * 
 * @param[in] *self 
 * 
 * @return pointer to self
 */
config_t *
config_init(config_t *self);

/**
 * get config's error stack
 * 
 * @param[in] *self 
 * 
 * @return 
 */
errstack_t *
config_get_error_stack(config_t *self);
