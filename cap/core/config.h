#pragma once

#include <pad/lib/memory.h>
#include <pad/lib/file.h>
#include <pad/lib/path.h>
#include <pad/core/config.h>
#include <pad/core/error_stack.h>

#include <cap/core/constant.h>

typedef struct CapConfig {
    PadConfig *pad_config;
    PadErrStack *errstack;  // error stack for error handling
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
} CapConfig;

/**
 * destruct CapConfig
 * 
 * @param[in] *self 
 */
void
CapConfig_Del(CapConfig *self);

/**
 * construct CapConfig
 * 
 * @return pointer to CapConfig dynamic allocate memory
 */
CapConfig *
CapConfig_New(void);

/**
 * initialize CapConfig
 * 
 * @param[in] *self 
 * 
 * @return pointer to self
 */
CapConfig *
CapConfig_Init(CapConfig *self);

/**
 * get config's error stack
 * 
 * @param[in] *self 
 * 
 * @return 
 */
PadErrStack *
CapConfig_GetErrStack(CapConfig *self);

/**
 * Get pad's config
 * 
 * @param[in] *self 
 * 
 * @return pointer to PadConfig
 */
PadConfig *
CapConfig_GetPadConfig(CapConfig *self);
