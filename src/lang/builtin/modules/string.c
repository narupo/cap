#include <lang/builtin/modules/string.h>

static object_t *
call_basic_str_func(ast_t *ast, const char *method_name) {
    const object_t *owner = ast->ref_dot_owner;
    if (!owner) {
        return obj_new_nil();
    }
    ast->ref_dot_owner = NULL;

again:
    switch (owner->type) {
    default:
        ast_set_error_detail(ast, "can't call %s function", method_name);
        return NULL;
        break;
    case OBJ_TYPE_STRING: {
        string_t *result = NULL;
        if (cstr_eq(method_name, "lower")) {
            result = str_lower(owner->string);
        } else if (cstr_eq(method_name, "upper")) {
            result = str_upper(owner->string);
        } else if (cstr_eq(method_name, "capitalize")) {
            result = str_capitalize(owner->string);
        } else if (cstr_eq(method_name, "snake")) {
            result = str_snake(owner->string);
        } else {
            ast_set_error_detail(ast, "invalid method name \"%s\" for call basic string function", method_name);
            return NULL;
        }
        return obj_new_str(result);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        owner = ctx_find_var_ref(ast->context, str_getc(owner->identifier));
        if (!owner) {
            ast_set_error_detail(ast, "not found \"%s\" in %s function", owner->identifier, method_name);
            return NULL;
        }
        goto again;
    } break;
    }

    assert(0 && "impossible. failed to invoke basic string function");
    return obj_new_nil();
}

static object_t *
builtin_string_lower(ast_t *ast, const object_t *_) {
    return call_basic_str_func(ast, "lower");
}

static object_t *
builtin_string_upper(ast_t *ast, const object_t *_) {
    return call_basic_str_func(ast, "upper");
}

static object_t *
builtin_string_capitalize(ast_t *ast, const object_t *_) {
    return call_basic_str_func(ast, "capitalize");
}

static object_t *
builtin_string_snake(ast_t *ast, const object_t *_) {
    return call_basic_str_func(ast, "snake");
}

static builtin_func_info_t
builtin_string_func_infos[] = {
    {"lower", builtin_string_lower},
    {"upper", builtin_string_upper},
    {"capitalize", builtin_string_capitalize},
    {"snake", builtin_string_snake},
    {0},
};

object_t *
builtin_string_module_new(void) {
    object_t *mod = obj_new_module();

    str_set(mod->module.name, "__string__");
    mod->module.builtin_func_infos = builtin_string_func_infos;

    return mod;
}