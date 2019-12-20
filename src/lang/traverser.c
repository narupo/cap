#include "lang/traverser.h"

/*********
* macros *
*********/

#define tready() \
    if (ast->debug) { \
        fprintf(stderr, "debug: %5d: %*s: %3d: %s\n", __LINE__, 40, __func__, dep, ast_get_error_detail(ast)); \
        fflush(stderr); \
    } \

#define return_trav(obj) \
    if (ast->debug) { \
        string_t *s = NULL; \
        if (obj) s = obj_to_str(obj); \
        fprintf(stderr, "debug: %5d: %*s: %3d: return %p (%s): %s\n", __LINE__, 40, __func__, dep, obj, (s ? str_getc(s) : "null"), ast_get_error_detail(ast)); \
        if (obj) str_del(s); \
        fflush(stderr); \
    } \
    return obj; \

#define tcheck(msg) \
    if (ast->debug) { \
        fprintf(stderr, "debug: %5d: %*s: %3d: %s: %s\n", __LINE__, 40, __func__, dep, msg, ast_get_error_detail(ast)); \
    } \

#define vissf(fmt, ...) \
    if (ast->debug) fprintf(stderr, "vissf: %d: " fmt "\n", __LINE__, __VA_ARGS__); \

#define viss(fmt) \
    if (ast->debug) fprintf(stderr, "viss: %d: " fmt "\n", __LINE__); \

/*************
* prototypes *
*************/

static object_t *
_trv_traverse(ast_t *ast, const node_t *node, int dep);

static object_t *
trv_compare_or(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep);

static object_t *
trv_compare_or_array(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep);

static object_t *
trv_compare_or_string(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep);

static object_t *
trv_compare_or_identifier(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep);

static object_t *
trv_compare_or_bool(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep);

static object_t *
trv_compare_or_int(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep);

static object_t *
trv_compare_and(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep);

static object_t *
trv_compare_comparison_eq(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep);

static object_t *
trv_compare_comparison_not_eq(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep);

static object_t *
trv_compare_comparison_gte(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep);

static object_t *
trv_compare_comparison_lte(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep);

static object_t *
trv_compare_comparison_gt(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep);

static object_t *
trv_compare_comparison_lt(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep);

static object_t *
trv_calc_expr_add(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep);

static object_t *
trv_calc_expr_sub(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep);

static object_t *
trv_calc_term_div(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep);

static object_t *
trv_calc_term_mul(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep);

static object_t *
get_var_ref(ast_t *ast, const char *identifier, int dep);

static object_t *
move_var(ast_t *ast, const char *identifier, object_t *move_obj, int dep);

static object_t *
pull_in_ref_by(ast_t *ast, const object_t *idn_obj);

static object_t *
trv_calc_asscalc_ass(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep);

static object_t *
trv_multi_assign(ast_t *ast, const node_t *node, int dep);

static object_t *
trv_calc_assign(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep);

static object_t *
trv_compare_comparison_not_eq_array(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep);

static object_t *
trv_compare_comparison_not_eq_string(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep);

static object_t *
trv_compare_comparison_not_eq_bool(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep);

static object_t *
trv_compare_comparison_not_eq_nil(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep);

static object_t *
trv_compare_comparison_not_eq_int(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep);

static object_t *
trv_compare_comparison_not_eq_func(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep);

static object_t *
trv_compare_comparison_lte_int(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep);

/************
* functions *
************/

void
_identifier_chain_to_array(cstring_array_t *arr, node_identifier_chain_t *identifier_chain
    ) {
    if (!identifier_chain) {
        return;
    }

    if (identifier_chain->identifier) {
        node_identifier_t *identifier = identifier_chain->identifier->real;
        cstrarr_push(arr, identifier->identifier);
    }

    if (identifier_chain->identifier_chain) {
        _identifier_chain_to_array(arr, identifier_chain->identifier_chain->real);
    }
}

static cstring_array_t *
identifier_chain_to_cstrarr(node_identifier_chain_t *identifier_chain) {
    cstring_array_t *arr = cstrarr_new();
    _identifier_chain_to_array(arr, identifier_chain);
    return arr;
}

static object_t *
trv_program(ast_t *ast, const node_t *node, int dep) {
    tready();
    node_program_t *program = node->real;

    tcheck("call _trv_traverse");
    _trv_traverse(ast, program->blocks, dep+1);
    if (ast_has_error(ast)) {
        return_trav(NULL);
    }

    return_trav(NULL);
}

static object_t *
trv_blocks(ast_t *ast, const node_t *node, int dep) {
    tready();
    node_blocks_t *blocks = node->real;

    tcheck("call _trv_traverse");
    _trv_traverse(ast, blocks->code_block, dep+1);
    if (ast_has_error(ast)) {
        return_trav(NULL);
    }

    if (ctx_get_do_break(ast->context) ||
        ctx_get_do_continue(ast->context)) {
        return_trav(NULL);
    }

    tcheck("call _trv_traverse");
    _trv_traverse(ast, blocks->ref_block, dep+1);
    if (ast_has_error(ast)) {
        return_trav(NULL);
    }

    tcheck("call _trv_traverse");
    _trv_traverse(ast, blocks->text_block, dep+1);
    if (ast_has_error(ast)) {
        return_trav(NULL);
    }

    tcheck("call _trv_traverse");
    _trv_traverse(ast, blocks->blocks, dep+1);
    if (ast_has_error(ast)) {
        return_trav(NULL);
    }

    return_trav(NULL);
}

static object_t *
trv_code_block(ast_t *ast, const node_t *node, int dep) {
    tready();
    node_code_block_t *code_block = node->real;

    tcheck("call _trv_traverse");
    _trv_traverse(ast, code_block->elems, dep+1);
    if (ast_has_error(ast)) {
        return_trav(NULL);
    }

    return_trav(NULL);
}

static object_t *
trv_get_value_of_index_obj(ast_t *ast, const object_t *index_obj) {
    assert(index_obj && index_obj->type == OBJ_TYPE_INDEX);
    
    assert(index_obj->index.ref_operand);
    object_t *operand = obj_new_other(index_obj->index.ref_operand);
    object_t *tmp_operand = NULL;
    assert(index_obj->index.indices);
    const object_array_t *indices = index_obj->index.indices;
    assert(operand);
    assert(indices);

    for (int32_t i = 0; i < objarr_len(indices); ++i) {
        const object_t *el = objarr_getc(indices, i);
        assert(el);

        const object_t *idx = el;
        if (el->type == OBJ_TYPE_IDENTIFIER) {
            idx = pull_in_ref_by(ast, el);
            if (!idx) {
                ast_set_error_detail(ast, "\"%s\" is not defined in index object", str_getc(el->identifier));
                obj_del(operand);
                return NULL;
            }
        }

        const char *skey = NULL;
        long ikey = -1;
        switch (idx->type) {
        default: err_die("invalid index type in get value of index obj"); break;
        case OBJ_TYPE_STRING: skey = str_getc(idx->string); break;
        case OBJ_TYPE_INTEGER: ikey = idx->lvalue; break;
        }

        if (operand->type == OBJ_TYPE_IDENTIFIER) {
            object_t *ref = pull_in_ref_by(ast, operand);
            if (ast_has_error(ast)) {
                obj_del(operand);
                return NULL;
            } else if (!ref) {
                ast_set_error_detail(ast, "\"%s\" is not defined in get value of index object", str_getc(operand->identifier));
                obj_del(operand);
                return NULL;
            }
            obj_del(operand);
            operand = obj_new_other(ref);
        }

        switch (operand->type) {
        default:
            ast_set_error_detail(ast, "invalid operand type (%d) in get value of index object", operand->type);
            obj_del(operand);
            break;
        case OBJ_TYPE_ARRAY: {
            if (idx->type != OBJ_TYPE_INTEGER) {
                ast_set_error_detail(ast, "invalid array index value. value is not integer");
                obj_del(operand);
                return NULL;
            }

            if (ikey < 0 || ikey >= objarr_len(operand->objarr)) {
                ast_set_error_detail(ast, "index out of range of array");
                obj_del(operand);
                return NULL;
            }

            tmp_operand = obj_new_other(objarr_getc(operand->objarr, ikey));
            obj_del(operand);
            operand = tmp_operand;
            tmp_operand = NULL;
        } break;
        case OBJ_TYPE_STRING: {
            if (idx->type != OBJ_TYPE_INTEGER) {
                ast_set_error_detail(ast, "invalid string index value. value is not integer");
                obj_del(operand);
                return NULL;
            }

            if (ikey < 0 || ikey >= str_len(operand->string)) {
                ast_set_error_detail(ast, "index out of range of string");
                obj_del(operand);
                return NULL;
            }

            const char ch = str_getc(operand->string)[ikey];
            string_t *str = str_new();
            str_pushb(str, ch);
            
            obj_del(operand);
            operand = obj_new_str(str);
        } break;
        case OBJ_TYPE_DICT: {
            if (idx->type != OBJ_TYPE_STRING) {
                ast_set_error_detail(ast, "invalid dict index value. value is not a string");
                obj_del(operand);
                return NULL;
            }
            assert(skey);

            const object_dict_item_t *item = objdict_getc(operand->objdict, skey);
            if (!item) {
                obj_del(operand);
                return NULL;
            }

            tmp_operand = obj_new_other(item->value);
            obj_del(operand);
            operand = tmp_operand;
            tmp_operand = NULL;
        } break;
        }
    }

    return operand;
}

static object_t *
trv_ref_block(ast_t *ast, const node_t *node, int dep) {
    tready();
    node_ref_block_t *ref_block = node->real;

    tcheck("call _trv_traverse");
    object_t *tmp = _trv_traverse(ast, ref_block->formula, dep+1);
    if (ast_has_error(ast)) {
        obj_del(tmp);
        return_trav(NULL);
    }
    assert(tmp);

    object_t *result = tmp;
    if (tmp->type == OBJ_TYPE_INDEX) {
        result = trv_get_value_of_index_obj(ast, tmp);
        if (ast_has_error(ast)) {
            obj_del(tmp);
            return_trav(NULL);
        }
        if (!result) {
            result = obj_new_nil();
        }
    }

    switch (result->type) {
    default:
        err_die("unsupported result type in traverse ref block");
        break;
    case OBJ_TYPE_NIL:
        ctx_pushb_buf(ast->context, "nil");
        break;
    case OBJ_TYPE_INTEGER: {
        char n[1024]; // very large
        snprintf(n, sizeof n, "%ld", result->lvalue);
        ctx_pushb_buf(ast->context, n);
    } break;
    case OBJ_TYPE_BOOL: {
        if (result->boolean) {
            ctx_pushb_buf(ast->context, "true");
        } else {
            ctx_pushb_buf(ast->context, "false");
        }
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        const char *idn = str_getc(result->identifier);
        object_t *obj = get_var_ref(ast, idn, dep+1);
        if (!obj) {
            ast_set_error_detail(ast, "\"%s\" is not defined in ref block", idn);
            return_trav(NULL);
        }
        string_t *str = obj_to_str(obj);
        ctx_pushb_buf(ast->context, str_getc(str));
        str_del(str);
    } break;
    case OBJ_TYPE_STRING: {
        ctx_pushb_buf(ast->context, str_getc(result->string));
    } break;
    case OBJ_TYPE_ARRAY: {
        ctx_pushb_buf(ast->context, "(array)");
    } break;
    case OBJ_TYPE_DICT: {
        ctx_pushb_buf(ast->context, "(dict)");
    } break;
    case OBJ_TYPE_FUNC: {
        ctx_pushb_buf(ast->context, "(function)");
    } break;
    } // switch

    obj_del(result);
    return_trav(NULL);
}


static object_t *
trv_text_block(ast_t *ast, const node_t *node, int dep) {
    tready();
    node_text_block_t *text_block = node->real;
    if (text_block->text) {
        ctx_pushb_buf(ast->context, text_block->text);
        tcheck("store text block to buf");
    }

    return_trav(NULL);
}

static object_t *
trv_elems(ast_t *ast, const node_t *node, int dep) {
    tready();
    node_elems_t *elems = node->real;
    object_t *result = NULL;

    tcheck("call _trv_traverse with def");
    if (elems->def) {
        _trv_traverse(ast, elems->def, dep+1);
        if (ast_has_error(ast)) {
            return_trav(NULL);
        }
    } else if (elems->stmt) {
        tcheck("call _trv_traverse with stmt");
        result = _trv_traverse(ast, elems->stmt, dep+1);
        if (ast_has_error(ast)) {
            return_trav(NULL);
        }

        if (ctx_get_do_break(ast->context) ||
            ctx_get_do_continue(ast->context)) {
            return_trav(result);
        } else if (ctx_get_do_return(ast->context)) {
            return_trav(result);
        }
        obj_del(result);
    } else if (elems->formula) {
        tcheck("call _trv_traverse with formula");
        _trv_traverse(ast, elems->formula, dep+1);
        if (ast_has_error(ast)) {
            return_trav(NULL);
        }        
    }

    tcheck("call _trv_traverse with elems");
    result = _trv_traverse(ast, elems->elems, dep+1);
    if (ast_has_error(ast)) {
        obj_del(result);
        return_trav(NULL);
    }

    return_trav(result);
}

static object_t *
trv_formula(ast_t *ast, const node_t *node, int dep) {
    tready();
    node_formula_t *formula = node->real;

    tcheck("call _trv_traverse");
    if (formula->assign_list) {
        object_t *result = _trv_traverse(ast, formula->assign_list, dep+1);
        if (ast_has_error(ast)) {
            obj_del(result);
            return_trav(NULL);
        }
        return_trav(result);
    } else if (formula->multi_assign) {
        object_t *result = _trv_traverse(ast, formula->multi_assign, dep+1);
        if (ast_has_error(ast)) {
            obj_del(result);
            return_trav(NULL);
        }
        return_trav(result);
    }

    assert(0 && "impossible. failed to traverse formula");
    return_trav(NULL);
}

static object_t *
trv_stmt(ast_t *ast, const node_t *node, int dep) {
    tready();
    node_stmt_t *stmt = node->real;
    object_t *result = NULL;

    if (stmt->import_stmt) {
        tcheck("call _trv_traverse with import stmt");
        _trv_traverse(ast, stmt->import_stmt, dep+1);
        if (ast_has_error(ast)) {
            return_trav(NULL);
        }
        return_trav(NULL);
    } else if (stmt->if_stmt) {
        tcheck("call _trv_traverse with if stmt");
        result = _trv_traverse(ast, stmt->if_stmt, dep+1);
        if (ast_has_error(ast)) {
            return_trav(NULL);
        }
        return_trav(result);
    } else if (stmt->for_stmt) {
        tcheck("call _trv_traverse with for stmt");
        result = _trv_traverse(ast, stmt->for_stmt, dep+1);
        if (ast_has_error(ast)) {
            return_trav(NULL);
        }
        return_trav(result);
    } else if (stmt->break_stmt) {
        tcheck("call _trv_traverse with break stmt");
        _trv_traverse(ast, stmt->break_stmt, dep+1);
        if (ast_has_error(ast)) {
            return_trav(NULL);
        }
        return_trav(NULL);
    } else if (stmt->continue_stmt) {
        tcheck("call _trv_traverse with continue stmt");
        _trv_traverse(ast, stmt->continue_stmt, dep+1);
        if (ast_has_error(ast)) {
            return_trav(NULL);
        }
        return_trav(NULL);
    } else if (stmt->return_stmt) {
        tcheck("call _trv_traverse with return stmt");
        result = _trv_traverse(ast, stmt->return_stmt, dep+1);
        if (ast_has_error(ast)) {
            return_trav(NULL);
        }
        return_trav(result);
    }

    assert(0 && "impossible. invalid state in traverse stmt");
    return_trav(NULL);
}

static object_t *
trv_import_stmt(ast_t *ast, const node_t *node, int dep) {
    tready();
    node_import_stmt_t *import_stmt = node->real;

    if (import_stmt->identifier_chain) {
        cstring_array_t *arr = identifier_chain_to_cstrarr(import_stmt->identifier_chain->real);
        cstrarr_del(arr);
    }

    // TODO

    return_trav(NULL);
}

/**
 * objectをbool値にする
 */
static bool
trv_parse_bool(ast_t *ast, const object_t *obj) {
    assert(obj);
    switch (obj->type) {
    case OBJ_TYPE_NIL: return false; break;
    case OBJ_TYPE_INTEGER: return obj->lvalue; break;
    case OBJ_TYPE_BOOL: return obj->boolean; break;
    case OBJ_TYPE_IDENTIFIER: {
        const char *idn = str_getc(obj->identifier);
        object_t *obj = get_var_ref(ast, idn, 0);
        if (!obj) {
            ast_set_error_detail(ast, "\"%s\" is not defined in if statement", idn);
            obj_del(obj);
            return false;
        }

        return trv_parse_bool(ast, obj);
    } break;
    case OBJ_TYPE_STRING: return str_len(obj->string); break;
    case OBJ_TYPE_ARRAY: return objarr_len(obj->objarr); break;
    case OBJ_TYPE_DICT: return objdict_len(obj->objdict); break;
    case OBJ_TYPE_FUNC: return true; break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, obj);
        if (!val) {
            ast_set_error_detail(ast, "value is null in parse bool");
            return false;
        }
        bool result = trv_parse_bool(ast, val);
        obj_del(val);
        return result;
    } break;
    }

    assert(0 && "impossible. failed to parse bool");
    return false;
}

static object_t *
trv_if_stmt(ast_t *ast, const node_t *node, int dep) {
    tready();
    node_if_stmt_t *if_stmt = node->real;

    tcheck("call _trv_traverse");
    object_t *result = _trv_traverse(ast, if_stmt->test, dep+1);
    if (ast_has_error(ast)) {
        obj_del(result);
        return_trav(NULL);
    }
    if (!result) {
        ast_set_error_detail(ast, "traverse error. test return null in if statement");
        return_trav(NULL);
    }

    bool boolean = trv_parse_bool(ast, result);
    obj_del(result);
    result = NULL;

    if (boolean) {
        if (if_stmt->elems) {
            tcheck("call _trv_traverse");
            result = _trv_traverse(ast, if_stmt->elems, dep+1);
            if (ast_has_error(ast)) {
                return_trav(NULL);
            }
        } else if (if_stmt->blocks) {
            tcheck("call _trv_traverse");
            result = _trv_traverse(ast, if_stmt->blocks, dep+1);
            if (ast_has_error(ast)) {
                return_trav(NULL);
            }
        } else {
            // pass
        }
    } else {
        if (if_stmt->elif_stmt) {
            tcheck("call _trv_traverse");
            result = _trv_traverse(ast, if_stmt->elif_stmt, dep+1);
            if (ast_has_error(ast)) {
                return_trav(NULL);
            }
        } else if (if_stmt->else_stmt) {
            tcheck("call _trv_traverse");
            result = _trv_traverse(ast, if_stmt->else_stmt, dep+1);
            if (ast_has_error(ast)) {
                return_trav(NULL);
            }
        } else {
            // pass
        }
    }

    return_trav(result);
}

static object_t *
trv_else_stmt(ast_t *ast, const node_t *node, int dep) {
    tready();
    node_else_stmt_t *else_stmt = node->real;
    assert(else_stmt);

    if (else_stmt->elems) {
        tcheck("call _trv_traverse with elems");
        _trv_traverse(ast, else_stmt->elems, dep+1);
    } else if (else_stmt->blocks) {
        tcheck("call _trv_traverse with blocks");
        _trv_traverse(ast, else_stmt->blocks, dep+1);
    }

    return_trav(NULL);
}

static object_t *
trv_for_stmt(ast_t *ast, const node_t *node, int dep) {
    tready();
    node_for_stmt_t *for_stmt = node->real;

    if (for_stmt->init_formula &&
        for_stmt->comp_formula &&
        for_stmt->update_formula) {
        // for 1; 1; 1: end
        tcheck("call _trv_traverse with init_formula");
        _trv_traverse(ast, for_stmt->init_formula, dep+1);
        if (ast_has_error(ast)) {
            return_trav(NULL);
        }

        for (;;) {
            tcheck("call _trv_traverse with update_formula");
            object_t *result = _trv_traverse(ast, for_stmt->comp_formula, dep+1);
            if (ast_has_error(ast)) {
                obj_del(result);
                goto done;
            }
            if (!trv_parse_bool(ast, result)) {
                obj_del(result);
                break;
            }
            obj_del(result);
            result = NULL;

            ctx_clear_jump_flags(ast->context);

            tcheck("call _trv_traverse with elems");
            if (for_stmt->elems) {
                _trv_traverse(ast, for_stmt->elems, dep+1);
                if (ast_has_error(ast)) {
                    goto done;
                }
            } else if (for_stmt->blocks) {
                _trv_traverse(ast, for_stmt->blocks, dep+1);
                if (ast_has_error(ast)) {
                    goto done;
                }                
            } // allow null elems and blocks

            if (ctx_get_do_break(ast->context)) {
                break;
            }

            tcheck("call _trv_traverse with update_formula");
            result = _trv_traverse(ast, for_stmt->update_formula, dep+1);
            if (ast_has_error(ast)) {
                goto done;
            }
            obj_del(result);
            result = NULL;
        } // for
    } else if (for_stmt->comp_formula) {
        // for 1: end
        for (;;) {
            tcheck("call _trv_traverse");
            object_t *result = _trv_traverse(ast, for_stmt->comp_formula, dep+1);
            if (ast_has_error(ast)) {
                goto done;
            }
            if (!trv_parse_bool(ast, result)) {
                obj_del(result);
                break;
            }

            tcheck("call _trv_traverse");
            if (for_stmt->elems) {
                _trv_traverse(ast, for_stmt->elems, dep+1);
                if (ast_has_error(ast)) {
                    goto done;
                }
            } else if (for_stmt->blocks) {
                _trv_traverse(ast, for_stmt->blocks, dep+1);
                if (ast_has_error(ast)) {
                    goto done;
                }                
            } // allow null elems and blocks

            if (ctx_get_do_break(ast->context)) {
                break;
            }

            obj_del(result);
        }
    } else {
        // for: end
        for (;;) {
            tcheck("call _trv_traverse");

            if (for_stmt->elems) {
                _trv_traverse(ast, for_stmt->elems, dep+1);
                if (ast_has_error(ast)) {
                    goto done;
                }
            } else if (for_stmt->blocks) {
                _trv_traverse(ast, for_stmt->blocks, dep+1);
                if (ast_has_error(ast)) {
                    goto done;
                }                
            } // allow null elems and blocks

            if (ctx_get_do_break(ast->context)) {
                break;
            }
        }
    }

done:
    ctx_clear_jump_flags(ast->context);
    return_trav(NULL);
}

static object_t *
trv_break_stmt(ast_t *ast, const node_t *node, int dep) {
    tready();
    assert(node->type == NODE_TYPE_BREAK_STMT);

    tcheck("set true at do break flag");
    ctx_set_do_break(ast->context, true);

    return_trav(NULL);
}

static object_t *
trv_continue_stmt(ast_t *ast, const node_t *node, int dep) {
    tready();
    assert(node->type == NODE_TYPE_CONTINUE_STMT);

    tcheck("set true at do continue flag");
    ctx_set_do_continue(ast->context, true);

    return_trav(NULL);
}

/**
 * extract identifier objects
 *
 * @return new object
 */
static object_t *
extract_obj(ast_t *ast, const object_t *srcobj) {
    assert(srcobj);
     
    switch (srcobj->type) {
    default: return obj_new_other(srcobj); break;
    case OBJ_TYPE_IDENTIFIER: {
        object_t *ref = pull_in_ref_by(ast, srcobj);
        if (!ref) {
            ast_set_error_detail(ast, "\"%s\" is not defined in extract obj", str_getc(srcobj->identifier));
            return NULL;
        }
        return obj_new_other(ref);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_array_t *objarr = objarr_new();
        for (int32_t i = 0; i < objarr_len(srcobj->objarr); ++i) {
            object_t *el = objarr_get(srcobj->objarr, i);
            object_t *newel = extract_obj(ast, el);
            objarr_moveb(objarr, newel);
        }
        return obj_new_array(objarr);
    } break;
    }

    assert(0 && "impossible. failed to extract object");
    return NULL;
}

static object_t *
trv_return_stmt(ast_t *ast, const node_t *node, int dep) {
    tready();
    assert(node->type == NODE_TYPE_RETURN_STMT);
    node_return_stmt_t *return_stmt = node->real;
    assert(return_stmt);

    if (!return_stmt->formula) {
        return_trav(NULL);
    }

    tcheck("call _trv_traverse with formula");
    object_t *result = _trv_traverse(ast, return_stmt->formula, dep+1);
    if (!result) {
        if (ast_has_error(ast)) {
            return_trav(NULL);
        }
        ast_set_error_detail(ast, "result is null from formula in return statement");
        return_trav(NULL);
    }

    // return文の場合、formulaの結果がidentifierだったらidentifierが指す
    // 実体を取得して返さなければならない
    // 関数の戻り値に、関数内の変数を使っていた場合、ここでidentifierをそのまま返すと、
    // 関数呼び出し時の代入で関数内の変数のidentifierが代入されてしまう
    // 例えば以下のようなコードである
    //
    //     def func():
    //         a = 1
    //         return a
    //     end
    //     x = func()
    //
    // そのためここで実体をコピーで取得して実体を返すようにする
    object_t *ret = extract_obj(ast, result);
    obj_del(result);

    tcheck("set true at do return flag");
    ctx_set_do_return(ast->context, true);

    return_trav(ret);
}

static object_t *
trv_calc_assign_to_array(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_ARRAY);

    switch (rhs->type) {
    default:
        ast_set_error_detail(ast, "can't assign element to array");
        return_trav(NULL);
        break;
    case OBJ_TYPE_ARRAY: {
        if (objarr_len(lhs->objarr) != objarr_len(rhs->objarr)) {
            ast_set_error_detail(ast, "can't assign array to array. not same length");
            return_trav(NULL);
        }

        object_array_t *results = objarr_new();

        for (int i = 0; i < objarr_len(lhs->objarr); ++i) {
            object_t *lh = objarr_get(lhs->objarr, i);
            object_t *rh = objarr_get(rhs->objarr, i);
            tcheck("call trv_calc_assign");
            object_t *result = trv_calc_assign(ast, lh, rh, dep+1);
            objarr_moveb(results, result);
        }

        return obj_new_array(results);
    } break;
    }

    assert(0 && "impossible. failed to assign to array");
    return_trav(NULL);
}

typedef struct index_value {
    char type; // 's' ... string, 'i' ... integer
    const char *skey;
    long ikey;
} index_value_t;

static index_value_t *
trv_obj_to_index_value(ast_t *ast, index_value_t *idxval, const object_t *obj) {
    const object_t *src = obj;
    object_t *delme = NULL;

    switch (obj->type) {
    default: break;
    case OBJ_TYPE_IDENTIFIER:
        src = pull_in_ref_by(ast, obj);
        if (ast_has_error(ast)) {
            return NULL;
        } else if (!src) {
            ast_set_error_detail(ast, "\"%s\" is not defined in extract index value", str_getc(obj->identifier));
            return NULL;
        }
        break;
    case OBJ_TYPE_INDEX:
        delme = trv_get_value_of_index_obj(ast, obj);
        if (ast_has_error(ast)) {
            return NULL;
        } else if (!delme) {
            ast_set_error_detail(ast, "index value is null in object to index value");
            return NULL;
        }
        src = delme;
        break;
    }

    switch (src->type) {
    default:
        ast_set_error_detail(ast, "invalid index object in object to index value");
        obj_del(delme);
        return NULL;
        break;
    case OBJ_TYPE_INTEGER:
        idxval->type = 'i';
        idxval->ikey = src->lvalue;
        break;
    case OBJ_TYPE_STRING:
        idxval->type = 's';
        idxval->skey = str_getc(src->string);
        break;
    }

    obj_del(delme);
    return idxval;
}

static object_t *
trv_copy_object_value(ast_t *ast, const object_t *obj) {
    assert(obj);

    switch (obj->type) {
    default: break;
    case OBJ_TYPE_IDENTIFIER: {
        const object_t *ref = pull_in_ref_by(ast, obj);
        if (!ref) {
            return NULL;
        }
        return obj_new_other(ref);
    } break;
    case OBJ_TYPE_INDEX:
        return trv_get_value_of_index_obj(ast, obj);
        break;
    }

    return obj_new_other(obj);
}

static object_t *
trv_assign_to_index(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_INDEX);

    object_t *ref_operand = lhs->index.ref_operand;
    const object_array_t *indices = lhs->index.indices;
    assert(ref_operand);
    assert(indices);

    const int32_t idxslen = objarr_len(indices);
    object_t *ret = NULL;

    for (int32_t i = 0; i < idxslen; ++i) {
        const object_t *el = objarr_getc(indices, i);
        assert(el);

        index_value_t idx = {0};
        trv_obj_to_index_value(ast, &idx, el);
        if (ast_has_error(ast)) {
            ast_set_error_detail(ast, "invalid index in assign to index");
            return_trav(NULL);
        }

        if (ref_operand->type == OBJ_TYPE_IDENTIFIER) {
            object_t *ref = pull_in_ref_by(ast, ref_operand);
            if (ast_has_error(ast)) {
                return_trav(NULL);
            } else if (!ref) {
                ast_set_error_detail(ast, "\"%s\" is not defined in assign to index", str_getc(ref_operand->identifier));
                return_trav(NULL);
            }
            ref_operand = ref;
        }

        switch (ref_operand->type) {
        default:
            ast_set_error_detail(ast, "operand (%d) is not assignable", ref_operand->type);
            return_trav(NULL);
            break;
        case OBJ_TYPE_ARRAY:
            if (idx.type != 'i') {
                ast_set_error_detail(ast, "invalid index type. index is not integer");
                return_trav(NULL);
            }

            if (i == idxslen-1) {
                // assign to
                object_t *copy = trv_copy_object_value(ast, rhs);
                if (ast_has_error(ast)) {
                    return_trav(NULL);
                } else if (!copy) {
                    ast_set_error_detail(ast, "failed to copy object value");
                    return_trav(NULL);
                }

                ret = obj_new_other(copy);

                if (!objarr_move(ref_operand->objarr, idx.ikey, copy)) {
                    ast_set_error_detail(ast, "failed to move object at array");
                    obj_del(copy);
                    obj_del(ret);
                    return_trav(NULL);
                }
            } else {
                // next operand
                if (idx.ikey < 0 || idx.ikey >= objarr_len(ref_operand->objarr)) {
                    ast_set_error_detail(ast, "array index out of range");
                    return_trav(NULL);
                }
                ref_operand = objarr_get(ref_operand->objarr, idx.ikey);
            }
            break;
        case OBJ_TYPE_DICT:
            if (idx.type != 's') {
                ast_set_error_detail(ast, "invalid index type. index is not string");
                return_trav(NULL);
            }

            if (i == idxslen-1) {
                // assign to
                object_t *copy = trv_copy_object_value(ast, rhs);
                if (ast_has_error(ast)) {
                    return_trav(NULL);
                } else if (!copy) {
                    ast_set_error_detail(ast, "failed to copy object value");
                    return_trav(NULL);
                }

                ret = obj_new_other(copy);

                if (!objdict_move(ref_operand->objdict, idx.skey, copy)) {
                    ast_set_error_detail(ast, "failed to move object at dict");
                    obj_del(copy);
                    obj_del(ret);
                    return_trav(NULL);
                }
            } else {
                // next operand
                const object_dict_item_t *item = objdict_getc(ref_operand->objdict, idx.skey);
                if (!item) {
                    ast_set_error_detail(ast, "invalid index key. \"%s\" is not found", idx.skey);
                    return_trav(NULL);
                }

                ref_operand = item->value;
            }
            break;
        }
    }

    assert(ret);
    return_trav(ret);
}

static object_t *
trv_calc_assign_to_index(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_INDEX);

    object_t *obj = trv_assign_to_index(ast, lhs, rhs, dep+1);
    if (ast_has_error(ast)) {
        obj_del(obj);
        return_trav(NULL);
    }

    return_trav(obj);
}

static object_t *
trv_calc_assign(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();

    switch (lhs->type) {
    default:
        ast_set_error_detail(ast, "syntax error. invalid lhs in assign list");
        return_trav(NULL);
        break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_calc_asscalc_ass");
        object_t *obj = trv_calc_asscalc_ass(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        tcheck("call trv_calc_assign_to_array");
        object_t *obj = trv_calc_assign_to_array(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *obj = trv_calc_assign_to_index(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    }    

    assert(0 && "impossible. failed to calc assign");
    return_trav(NULL);
}

/**
 * 右優先結合
 */
static object_t *
trv_simple_assign(ast_t *ast, const node_t *node, int dep) {
    tready();
    assert(node->type == NODE_TYPE_SIMPLE_ASSIGN);
    node_simple_assign_t *simple_assign = node->real;

    if (!nodearr_len(simple_assign->nodearr)) {
        ast_set_error_detail(ast, "failed to traverse simple assign. array is empty");
        return_trav(NULL);
    }

    int32_t arrlen = nodearr_len(simple_assign->nodearr);
    node_t *rnode = nodearr_get(simple_assign->nodearr, arrlen-1);
    assert(rnode->type == NODE_TYPE_TEST);
    tcheck("call _trv_traverse with right test");
    object_t *rhs = _trv_traverse(ast, rnode, dep+1);
    if (ast_has_error(ast)) {
        return_trav(NULL);
    }
    assert(rhs);

    for (int32_t i = arrlen-2; i >= 0; --i) {
        node_t *lnode = nodearr_get(simple_assign->nodearr, i);
        assert(lnode->type == NODE_TYPE_TEST);
        tcheck("call _trv_traverse with test left test");
        object_t *lhs = _trv_traverse(ast, lnode, dep+1);
        if (ast_has_error(ast)) {
            obj_del(rhs);
            obj_del(lhs);
            return_trav(NULL);
        }
        if (!lhs) {
            obj_del(rhs);
            return_trav(NULL);
        }

        object_t *result = trv_calc_assign(ast, lhs, rhs, dep+1);
        if (ast_has_error(ast)) {
            obj_del(rhs);
            obj_del(lhs);
            obj_del(result);
            return_trav(NULL);
        }

        obj_del(rhs);
        obj_del(lhs);
        rhs = result;
    }

    return_trav(rhs);
}

/**
 * 右優先結合
 */
static object_t *
trv_assign(ast_t *ast, const node_t *node, int dep) {
    tready();
    assert(node->type == NODE_TYPE_ASSIGN);
    node_assign_list_t *assign_list = node->real;

    if (!nodearr_len(assign_list->nodearr)) {
        ast_set_error_detail(ast, "failed to traverse assign. array is empty");
        return_trav(NULL);
    }

    int32_t arrlen = nodearr_len(assign_list->nodearr);
    node_t *rnode = nodearr_get(assign_list->nodearr, arrlen-1);
    assert(rnode->type == NODE_TYPE_TEST);
    tcheck("call _trv_traverse with test rnode");
    object_t *rhs = _trv_traverse(ast, rnode, dep+1);
    if (ast_has_error(ast)) {
        return_trav(NULL);
    }
    assert(rhs);

    for (int32_t i = arrlen-2; i >= 0; --i) {
        node_t *lnode = nodearr_get(assign_list->nodearr, i);
        assert(lnode->type == NODE_TYPE_TEST);
        tcheck("call _trv_traverse with test lnode");
        object_t *lhs = _trv_traverse(ast, lnode, dep+1);
        if (ast_has_error(ast)) {
            obj_del(rhs);
            obj_del(lhs);
            return_trav(NULL);
        }
        if (!lhs) {
            obj_del(rhs);
            return_trav(NULL);
        }

        object_t *result = trv_calc_assign(ast, lhs, rhs, dep+1);
        if (ast_has_error(ast)) {
            obj_del(rhs);
            obj_del(lhs);
            obj_del(result);
            return_trav(NULL);
        }

        obj_del(rhs);
        obj_del(lhs);
        rhs = result;
    }

    return_trav(rhs);
}

static object_t *
trv_assign_list(ast_t *ast, const node_t *node, int dep) {
    tready();
    assert(node->type == NODE_TYPE_ASSIGN_LIST);
    node_assign_list_t *assign_list = node->real;

    if (!nodearr_len(assign_list->nodearr)) {
        ast_set_error_detail(ast, "failed to traverse assign list. array is empty");
        return_trav(NULL);
    }

    object_array_t *objarr = objarr_new();

    int32_t arrlen = nodearr_len(assign_list->nodearr);
    node_t *assign = nodearr_get(assign_list->nodearr, 0);
    assert(assign->type == NODE_TYPE_ASSIGN);

    tcheck("call _trv_traverse with assign assign");
    object_t *obj = _trv_traverse(ast, assign, dep+1);
    if (ast_has_error(ast)) {
        return_trav(NULL);
    }
    assert(obj);

    objarr_moveb(objarr, obj);

    for (int32_t i = 1; i < arrlen; ++i) {
        assign = nodearr_get(assign_list->nodearr, i);
        assert(assign->type == NODE_TYPE_ASSIGN);

        tcheck("call _trv_traverse with assign assign");
        obj = _trv_traverse(ast, assign, dep+1);
        if (ast_has_error(ast)) {
            return_trav(NULL);
        }
        if (!obj) {
            goto done;
        }

        objarr_moveb(objarr, obj);
    }

done:
    assert(objarr_len(objarr));
    if (objarr_len(objarr) == 1) {
        obj = objarr_popb(objarr);
        objarr_del(objarr);
        return_trav(obj);
    }

    obj = obj_new_array(objarr);
    return_trav(obj);
}

/**
 * 右優先結合
 */
static object_t *
trv_multi_assign(ast_t *ast, const node_t *node, int dep) {
    tready();
    assert(node->type == NODE_TYPE_MULTI_ASSIGN);
    node_multi_assign_t *multi_assign = node->real;

    if (!nodearr_len(multi_assign->nodearr)) {
        ast_set_error_detail(ast, "failed to traverse assign list. array is empty");
        return_trav(NULL);
    }

    int32_t arrlen = nodearr_len(multi_assign->nodearr);
    node_t *rnode = nodearr_get(multi_assign->nodearr, arrlen-1);
    assert(rnode->type == NODE_TYPE_TEST_LIST);
    tcheck("call _trv_traverse with right test_list node");
    object_t *rhs = _trv_traverse(ast, rnode, dep+1);
    if (ast_has_error(ast)) {
        return_trav(NULL);
    }
    assert(rhs);

    for (int32_t i = arrlen-2; i >= 0; --i) {
        node_t *lnode = nodearr_get(multi_assign->nodearr, i);
        assert(lnode->type == NODE_TYPE_TEST_LIST);
        tcheck("call _trv_traverse with left test_list node");
        object_t *lhs = _trv_traverse(ast, lnode, dep+1);
        if (ast_has_error(ast)) {
            obj_del(rhs);
            obj_del(lhs);
            return_trav(NULL);
        }
        if (!lhs) {
            ast_set_error_detail(ast, "failed to traverse left test_list in multi assign");
            obj_del(rhs);            
            return_trav(NULL);
        }

        object_t *result = trv_calc_assign(ast, lhs, rhs, dep+1);
        if (ast_has_error(ast)) {
            obj_del(result);
            obj_del(rhs);
            obj_del(lhs);
            return_trav(NULL);
        }
        if (!result) {
            ast_set_error_detail(ast, "failed to assign in multi assign");
            obj_del(rhs);
            obj_del(lhs);
            return_trav(NULL);            
        }

        obj_del(rhs);
        obj_del(lhs);
        rhs = result;
    }

    return_trav(rhs);
}

static object_t *
trv_test_list(ast_t *ast, const node_t *node, int dep) {
    tready();
    node_test_list_t *test_list = node->real;
    assert(test_list);

    assert(nodearr_len(test_list->nodearr));
    if (nodearr_len(test_list->nodearr) == 1) {
        node_t *test = nodearr_get(test_list->nodearr, 0);
        tcheck("call _trv_traverse")
        object_t *obj = _trv_traverse(ast, test, dep+1);
        return_trav(obj);
    }

    object_array_t *arr = objarr_new();

    for (int32_t i = 0; i < nodearr_len(test_list->nodearr); ++i) {
        node_t *test = nodearr_get(test_list->nodearr, i);
        tcheck("call _trv_traverse");
        object_t *result = _trv_traverse(ast, test, dep+1);
        if (ast_has_error(ast)) {
            return_trav(NULL);
        }

        objarr_moveb(arr, result);
    }

    object_t *obj = obj_new_array(arr);
    return_trav(obj);
}

static object_t *
trv_test(ast_t *ast, const node_t *node, int dep) {
    tready();
    node_test_t *test = node->real;
    tcheck("call _trv_traverse");
    object_t *obj = _trv_traverse(ast, test->or_test, dep+1);
    return_trav(obj);
}

/**
 * return *reference* (do not delete)
 */
static object_t *
get_var_ref(ast_t *ast, const char *identifier, int dep) {
    tready();

    object_t *obj = ctx_find_var_ref(ast->context, identifier);
    if (!obj) {
        return_trav(NULL);
    }

    return_trav(obj);
}

/**
 * set (move) object at varmap of current scope
 */
static object_t *
move_var(ast_t *ast, const char *identifier, object_t *move_obj, int dep) {
    tready();
    assert(move_obj->type != OBJ_TYPE_IDENTIFIER);

    object_dict_t *varmap = ctx_get_varmap(ast->context);
    objdict_move(varmap, identifier, move_obj);

    return_trav(NULL);
}

static object_t *
trv_roll_identifier_lhs(
    ast_t *ast,
    const object_t *lhs,
    const object_t *rhs,
    object_t* (*func)(ast_t *, const object_t *, const object_t *, int),
    int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_IDENTIFIER);

    const char *idn = str_getc(lhs->identifier);
    object_t *lvar = get_var_ref(ast, idn, dep+1);
    if (!lvar) {
        ast_set_error_detail(ast, "\"%s\" is not defined in roll identifier lhs", idn);
        return_trav(NULL);
    }

    tcheck("call function pointer");
    object_t *obj = func(ast, lvar, rhs, dep+1);
    return_trav(obj);
}

static object_t*
trv_roll_identifier_rhs(
    ast_t *ast,
    const object_t *lhs,
    const object_t *rhs,
    object_t* (*func)(ast_t *, const object_t *, const object_t *, int),
    int dep) {
    tready();
    assert(rhs->type == OBJ_TYPE_IDENTIFIER);

    object_t *rvar = get_var_ref(ast, str_getc(rhs->identifier), dep+1);
    if (!rvar) {
        ast_set_error_detail(ast, "\"%s\" is not defined in roll identifier rhs", str_getc(rhs->identifier));
        return_trav(NULL);
    }

    tcheck("call function pointer");
    object_t *obj = func(ast, lhs, rvar, dep+1);
    return_trav(obj);
}

static object_t *
trv_compare_or_int(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_INTEGER);

    switch (rhs->type) {
    case OBJ_TYPE_NIL: {
        object_t *obj = NULL;
        if (lhs->lvalue && NULL) {
            obj = obj_new_other(lhs);
        } else if (lhs->lvalue && !NULL) {
            obj = obj_new_other(lhs);
        } else if (!lhs->lvalue && NULL) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = NULL;
        if (lhs->lvalue && rhs->lvalue) {
            obj = obj_new_other(lhs);
        } else if (lhs->lvalue && !rhs->lvalue) {
            obj = obj_new_other(lhs);
        } else if (!lhs->lvalue && rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (lhs->lvalue && rhs->boolean) {
            obj = obj_new_other(lhs);
        } else if (lhs->lvalue && !rhs->boolean) {
            obj = obj_new_other(lhs);
        } else if (!lhs->lvalue && rhs->boolean) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (lhs->lvalue && str_len(rhs->string)) {
            obj = obj_new_other(lhs);
        } else if (lhs->lvalue && !str_len(rhs->string)) {
            obj = obj_new_other(lhs);
        } else if (!lhs->lvalue && str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (lhs->lvalue && objarr_len(rhs->objarr)) {
            obj = obj_new_other(lhs);
        } else if (lhs->lvalue && !objarr_len(rhs->objarr)) {
            obj = obj_new_other(lhs);
        } else if (!lhs->lvalue && objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = NULL;
        if (lhs->lvalue && objdict_len(rhs->objdict)) {
            obj = obj_new_other(lhs);
        } else if (lhs->lvalue && !objdict_len(rhs->objdict)) {
            obj = obj_new_other(lhs);
        } else if (!lhs->lvalue && objdict_len(rhs->objdict)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        object_t *rvar = pull_in_ref_by(ast, rhs);
        if (!rvar) {
            ast_set_error_detail(ast, "%s is not defined in compare or int", str_getc(rhs->identifier));
            return_trav(NULL);
        }

        tcheck("call trv_compare_or with rvar");
        object_t *obj = trv_compare_or(ast, lhs, rvar, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_FUNC: {
        object_t *obj = NULL;
        if (lhs->lvalue && rhs) {
            obj = obj_new_other(lhs);
        } else if (lhs->lvalue && !rhs) {
            obj = obj_new_other(lhs);
        } else if (!lhs->lvalue && rhs) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't compare or int. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_or_int(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare or int");
    return_trav(NULL);
}

static object_t *
trv_compare_or_bool(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_BOOL);

    switch (rhs->type) {
    case OBJ_TYPE_NIL: {
        object_t *obj = NULL;
        if (lhs->boolean && NULL) {
            obj = obj_new_other(lhs);            
        } else if (lhs->boolean && !NULL) {
            obj = obj_new_other(lhs);
        } else if (!lhs->boolean && NULL) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = NULL;
        if (lhs->boolean && rhs->lvalue) {
            obj = obj_new_other(lhs);            
        } else if (lhs->boolean && !rhs->lvalue) {
            obj = obj_new_other(lhs);
        } else if (!lhs->boolean && rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (lhs->boolean && rhs->boolean) {
            obj = obj_new_other(lhs);
        } else if (lhs->boolean && !rhs->boolean) {
            obj = obj_new_other(lhs);
        } else if (!lhs->boolean && rhs->boolean) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (lhs->boolean && str_len(rhs->string)) {
            obj = obj_new_other(lhs);
        } else if (lhs->boolean && !str_len(rhs->string)) {
            obj = obj_new_other(lhs);
        } else if (!lhs->boolean && str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (lhs->boolean && objarr_len(rhs->objarr)) {
            obj = obj_new_other(lhs);
        } else if (lhs->boolean && !objarr_len(rhs->objarr)) {
            obj = obj_new_other(lhs);
        } else if (!lhs->boolean && objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = NULL;
        if (lhs->boolean && objdict_len(rhs->objdict)) {
            obj = obj_new_other(lhs);
        } else if (lhs->boolean && !objdict_len(rhs->objdict)) {
            obj = obj_new_other(lhs);
        } else if (!lhs->boolean && objdict_len(rhs->objdict)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        object_t *rvar = get_var_ref(ast, str_getc(rhs->identifier), dep+1);
        if (!rvar) {
            ast_set_error_detail(ast, "%s is not defined compare or bool", str_getc(rhs->identifier));
            return_trav(NULL);
        }

        tcheck("call trv_compare_or");
        object_t *obj = trv_compare_or(ast, lhs, rvar, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_FUNC: {
        object_t *obj = NULL;
        if (lhs->boolean && rhs) {
            obj = obj_new_other(lhs);
        } else if (lhs->boolean && !rhs) {
            obj = obj_new_other(lhs);
        } else if (!lhs->boolean && rhs) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't compare or bool. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_or_bool(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare or bool");
    return_trav(NULL);
}

static object_t *
trv_compare_or_string(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_STRING);

    int32_t slen = str_len(lhs->string);

    switch (rhs->type) {
    case OBJ_TYPE_NIL: {
        object_t *obj = NULL;
        if (slen && NULL) {
            obj = obj_new_other(lhs);
        } else if (slen && !NULL) {
            obj = obj_new_other(lhs);
        } else if (!slen && NULL) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = NULL;
        if (slen && rhs->lvalue) {
            obj = obj_new_other(lhs);
        } else if (slen && !rhs->lvalue) {
            obj = obj_new_other(lhs);
        } else if (!slen && rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (slen && rhs->boolean) {
            obj = obj_new_other(lhs);
        } else if (slen && !rhs->boolean) {
            obj = obj_new_other(lhs);
        } else if (!slen && rhs->boolean) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (slen && str_len(rhs->string)) {
            obj = obj_new_other(lhs);
        } else if (slen && !str_len(rhs->string)) {
            obj = obj_new_other(lhs);
        } else if (!slen && str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (slen && objarr_len(rhs->objarr)) {
            obj = obj_new_other(lhs);
        } else if (slen && !objarr_len(rhs->objarr)) {
            obj = obj_new_other(lhs);
        } else if (!slen && objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = NULL;
        if (slen && objdict_len(rhs->objdict)) {
            obj = obj_new_other(lhs);
        } else if (slen && !objdict_len(rhs->objdict)) {
            obj = obj_new_other(lhs);
        } else if (!slen && objdict_len(rhs->objdict)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_rhs");
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_compare_or, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_FUNC: {
        object_t *obj = NULL;
        if (slen && rhs) {
            obj = obj_new_other(lhs);
        } else if (slen && !rhs) {
            obj = obj_new_other(lhs);
        } else if (!slen && rhs) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't compare or string. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_or_string(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare or string");
    return_trav(NULL);
}

static object_t *
trv_compare_or_array(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_ARRAY);

    int32_t arrlen = objarr_len(lhs->objarr);

    switch (rhs->type) {
    case OBJ_TYPE_NIL: {
        object_t *obj = NULL;
        if (arrlen && NULL) {
            obj = obj_new_other(lhs);
        } else if (arrlen && !NULL) {
            obj = obj_new_other(lhs);
        } else if (!arrlen && NULL) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = NULL;
        if (arrlen && rhs->lvalue) {
            obj = obj_new_other(lhs);
        } else if (arrlen && !rhs->lvalue) {
            obj = obj_new_other(lhs);
        } else if (!arrlen && rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (arrlen && rhs->boolean) {
            obj = obj_new_other(lhs);
        } else if (arrlen && !rhs->boolean) {
            obj = obj_new_other(lhs);
        } else if (!arrlen && rhs->boolean) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (arrlen && str_len(rhs->string)) {
            obj = obj_new_other(lhs);
        } else if (arrlen && !str_len(rhs->string)) {
            obj = obj_new_other(lhs);
        } else if (!arrlen && str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (arrlen && objarr_len(rhs->objarr)) {
            obj = obj_new_other(lhs);
        } else if (arrlen && !objarr_len(rhs->objarr)) {
            obj = obj_new_other(lhs);
        } else if (!arrlen && objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = NULL;
        if (arrlen && objdict_len(rhs->objdict)) {
            obj = obj_new_other(lhs);
        } else if (arrlen && !objdict_len(rhs->objdict)) {
            obj = obj_new_other(lhs);
        } else if (!arrlen && objdict_len(rhs->objdict)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_rhs");
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_compare_or, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_FUNC: {
        object_t *obj = NULL;
        if (arrlen && rhs) {
            obj = obj_new_other(lhs);
        } else if (arrlen && !rhs) {
            obj = obj_new_other(lhs);
        } else if (!arrlen && rhs) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't compare or array. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_or_array(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare or array");
    return_trav(NULL);
}

static object_t *
trv_compare_or_dict(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_DICT);

    int32_t dictlen = objdict_len(lhs->objdict);

    switch (rhs->type) {
    case OBJ_TYPE_NIL: {
        object_t *obj = NULL;
        if (dictlen && NULL) {
            obj = obj_new_other(lhs);
        } else if (dictlen && !NULL) {
            obj = obj_new_other(lhs);
        } else if (!dictlen && NULL) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = NULL;
        if (dictlen && rhs->lvalue) {
            obj = obj_new_other(lhs);
        } else if (dictlen && !rhs->lvalue) {
            obj = obj_new_other(lhs);
        } else if (!dictlen && rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (dictlen && rhs->boolean) {
            obj = obj_new_other(lhs);
        } else if (dictlen && !rhs->boolean) {
            obj = obj_new_other(lhs);
        } else if (!dictlen && rhs->boolean) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (dictlen && str_len(rhs->string)) {
            obj = obj_new_other(lhs);
        } else if (dictlen && !str_len(rhs->string)) {
            obj = obj_new_other(lhs);
        } else if (!dictlen && str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (dictlen && objarr_len(rhs->objarr)) {
            obj = obj_new_other(lhs);
        } else if (dictlen && !objarr_len(rhs->objarr)) {
            obj = obj_new_other(lhs);
        } else if (!dictlen && objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = NULL;
        if (dictlen && objdict_len(rhs->objdict)) {
            obj = obj_new_other(lhs);
        } else if (dictlen && !objdict_len(rhs->objdict)) {
            obj = obj_new_other(lhs);
        } else if (!dictlen && objdict_len(rhs->objdict)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_rhs");
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_compare_or, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_FUNC: {
        object_t *obj = NULL;
        if (dictlen && rhs) {
            obj = obj_new_other(lhs);
        } else if (dictlen && !rhs) {
            obj = obj_new_other(lhs);
        } else if (!dictlen && rhs) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't compare or dict. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_or_dict(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare or dict");
    return_trav(NULL);
}

static object_t *
trv_compare_or_nil(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_NIL);

    switch (rhs->type) {
    case OBJ_TYPE_NIL: {
        object_t *obj = obj_new_other(lhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = obj_new_other(rhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_other(rhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = obj_new_other(rhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = obj_new_other(rhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = obj_new_other(rhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_rhs");
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_compare_or, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_FUNC: {
        object_t *obj = obj_new_other(rhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't compare or nil. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_or_nil(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare or nil");
    return_trav(NULL);
}

static object_t *
trv_compare_or_func(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_FUNC);

    switch (rhs->type) {
    case OBJ_TYPE_NIL: {
        object_t *obj = NULL;
        if (lhs && NULL) {
        } else if (lhs && !NULL) {
            obj = obj_new_other(lhs);
        } else if (!lhs && NULL) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = NULL;
        if (lhs && rhs->lvalue) {
            obj = obj_new_other(lhs);
        } else if (lhs && !rhs->lvalue) {
            obj = obj_new_other(lhs);
        } else if (!lhs && rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (lhs && rhs->boolean) {
            obj = obj_new_other(lhs);
        } else if (lhs && !rhs->boolean) {
            obj = obj_new_other(lhs);
        } else if (!lhs && rhs->boolean) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (lhs && str_len(rhs->string)) {
            obj = obj_new_other(lhs);
        } else if (lhs && !str_len(rhs->string)) {
            obj = obj_new_other(lhs);
        } else if (!lhs && str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (lhs && objarr_len(rhs->objarr)) {
            obj = obj_new_other(lhs);
        } else if (lhs && !objarr_len(rhs->objarr)) {
            obj = obj_new_other(lhs);
        } else if (!lhs && objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = NULL;
        if (lhs && objdict_len(rhs->objdict)) {
            obj = obj_new_other(lhs);
        } else if (lhs && !objdict_len(rhs->objdict)) {
            obj = obj_new_other(lhs);
        } else if (!lhs && objdict_len(rhs->objdict)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_rhs");
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_compare_or, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_FUNC: {
        object_t *obj = NULL;
        if (lhs && rhs) {
            obj = obj_new_other(lhs);
        } else if (lhs && !rhs) {
            obj = obj_new_other(lhs);
        } else if (!lhs && rhs) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't compare or func. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_or_func(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare or array");
    return_trav(NULL);
}

static object_t *
trv_compare_or(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();

    switch (lhs->type) {
    case OBJ_TYPE_NIL: {
        tcheck("call trv_compare_or_nil");
        object_t *obj = trv_compare_or_nil(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        tcheck("call trv_compare_or_int");
        object_t *obj = trv_compare_or_int(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        tcheck("call trv_compare_or_bool");
        object_t *obj = trv_compare_or_bool(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        tcheck("call trv_compare_or_string");
        object_t *obj = trv_compare_or_string(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        tcheck("call trv_compare_or_array");
        object_t *obj = trv_compare_or_array(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        tcheck("call trv_compare_or_dict");
        object_t *obj = trv_compare_or_dict(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_rhs");
        object_t *obj = trv_roll_identifier_lhs(ast, lhs, rhs, trv_compare_or, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_FUNC: {
        tcheck("call trv_compare_or_func");
        object_t *obj = trv_compare_or_func(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, lhs);
        if (!val) {
            ast_set_error_detail(ast, "can't compare or. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_or(ast, val, rhs, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare or");
    return_trav(NULL);
}

static object_t *
trv_or_test(ast_t *ast, const node_t *node, int dep) {
    tready();
    assert(node->type == NODE_TYPE_OR_TEST);
    node_or_test_t *or_test = node->real;

    node_t *lnode = nodearr_get(or_test->nodearr, 0);
    tcheck("call _trv_traverse");
    object_t *lhs = _trv_traverse(ast, lnode, dep+1);
    if (ast_has_error(ast)) {
        return_trav(NULL);
    }
    assert(lhs);

    for (int i = 1; i < nodearr_len(or_test->nodearr); ++i) {
        node_t *rnode = nodearr_get(or_test->nodearr, i);
        tcheck("call _trv_traverse");
        object_t *rhs = _trv_traverse(ast, rnode, dep+1);
        if (ast_has_error(ast)) {
            return_trav(NULL);
        }
        if (!rhs) {
            return_trav(lhs);
        }

        tcheck("call trv_compare_or");
        object_t *result = trv_compare_or(ast, lhs, rhs, dep+1);
        if (ast_has_error(ast)) {
            obj_del(lhs);
            obj_del(rhs);
            return_trav(NULL);
        }
        assert(result);

        obj_del(lhs);
        obj_del(rhs);
        lhs = result;
    }

    return_trav(lhs);
}

static object_t *
trv_compare_and_int(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_INTEGER);

    switch (rhs->type) {
    case OBJ_TYPE_NIL: {
        object_t *obj = NULL;
        if (lhs->lvalue && NULL) {
            obj = obj_new_other(rhs);
        } else if (!NULL) {
            obj = obj_new_other(rhs);
        } else if (!lhs->lvalue) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = NULL;
        if (lhs->lvalue && rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else if (!rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else if (!lhs->lvalue) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (lhs->lvalue && rhs->boolean) {
            obj = obj_new_other(rhs);
        } else if (!rhs->boolean) {
            obj = obj_new_other(rhs);
        } else if (!lhs->lvalue) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (lhs->lvalue && str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else if (!str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else if (!lhs->lvalue) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (lhs->lvalue && objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else if (!objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else if (!lhs->lvalue) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = NULL;
        if (lhs->lvalue && objdict_len(rhs->objdict)) {
            obj = obj_new_other(rhs);
        } else if (!objdict_len(rhs->objdict)) {
            obj = obj_new_other(rhs);
        } else if (!lhs->lvalue) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_rhs");
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_compare_and, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_FUNC: {
        object_t *obj = NULL;
        if (lhs->lvalue && rhs) {
            obj = obj_new_other(rhs);
        } else if (!rhs) {
            obj = obj_new_other(rhs);
        } else if (!lhs->lvalue) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't compare and int. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_and_int(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare and int");
    return_trav(NULL);
}

static object_t *
trv_compare_and_bool(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_BOOL);

    switch (rhs->type) {
    case OBJ_TYPE_NIL: {
        object_t *obj = NULL;
        if (lhs->boolean && NULL) {
            obj = obj_new_other(rhs);
        } else if (!NULL) {
            obj = obj_new_other(rhs);
        } else if (!lhs->boolean) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = NULL;
        if (lhs->boolean && rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else if (!rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else if (!lhs->boolean) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (lhs->boolean && rhs->boolean) {
            obj = obj_new_other(rhs);
        } else if (!rhs->boolean) {
            obj = obj_new_other(rhs);
        } else if (!lhs->boolean) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (lhs->boolean && str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else if (!str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else if (!lhs->boolean) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (lhs->boolean && objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else if (!objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else if (!lhs->boolean) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = NULL;
        if (lhs->boolean && objdict_len(rhs->objdict)) {
            obj = obj_new_other(rhs);
        } else if (!objdict_len(rhs->objdict)) {
            obj = obj_new_other(rhs);
        } else if (!lhs->boolean) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_rhs");
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_compare_and, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_FUNC: {
        object_t *obj = NULL;
        if (lhs->boolean && rhs) {
            obj = obj_new_other(rhs);
        } else if (!rhs) {
            obj = obj_new_other(rhs);
        } else if (!lhs->boolean) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't compare and bool. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_and_bool(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare and bool");
    return_trav(NULL);
}

static object_t *
trv_compare_and_string(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_STRING);

    int32_t slen = str_len(lhs->string);

    switch (rhs->type) {
    case OBJ_TYPE_NIL: {
        object_t *obj = NULL;
        if (slen && NULL) {
            obj = obj_new_other(rhs);
        } else if (!NULL) {
            obj = obj_new_other(rhs);
        } else if (!slen) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = NULL;
        if (slen && rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else if (!rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else if (!slen) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (slen && rhs->boolean) {
            obj = obj_new_other(rhs);
        } else if (!rhs->boolean) {
            obj = obj_new_other(rhs);
        } else if (!slen) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (slen && str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else if (!str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else if (!slen) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (slen && objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else if (!objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else if (!slen) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = NULL;
        if (slen && objdict_len(rhs->objdict)) {
            obj = obj_new_other(rhs);
        } else if (!objdict_len(rhs->objdict)) {
            obj = obj_new_other(rhs);
        } else if (!slen) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_rhs");
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_compare_and, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_FUNC: {
        object_t *obj = NULL;
        if (slen && rhs) {
            obj = obj_new_other(rhs);
        } else if (!rhs) {
            obj = obj_new_other(rhs);
        } else if (!slen) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't compare and string. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_and_string(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare and string");
    return_trav(NULL);
}

static object_t *
trv_compare_and_array(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_ARRAY);

    int32_t arrlen = objarr_len(lhs->objarr);

    switch (rhs->type) {
    case OBJ_TYPE_NIL: {
        object_t *obj = obj_new_other(rhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = NULL;
        if (arrlen && rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else if (!rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else if (!arrlen) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (arrlen && rhs->boolean) {
            obj = obj_new_other(rhs);
        } else if (!rhs->boolean) {
            obj = obj_new_other(rhs);
        } else if (!arrlen) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (arrlen && str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else if (!str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else if (!arrlen) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (arrlen && objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else if (!objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else if (!arrlen) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = NULL;
        if (arrlen && objdict_len(rhs->objdict)) {
            obj = obj_new_other(rhs);
        } else if (!objdict_len(rhs->objdict)) {
            obj = obj_new_other(rhs);
        } else if (!arrlen) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_rhs");
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_compare_and, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_FUNC: {
        object_t *obj = NULL;
        if (arrlen && rhs) {
            obj = obj_new_other(rhs);
        } else if (!rhs) {
            obj = obj_new_other(rhs);
        } else if (!arrlen) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't compare and array. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_and_array(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare and array");
    return_trav(NULL);
}

static object_t *
trv_compare_and_dict(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_DICT);

    int32_t dictlen = objdict_len(lhs->objdict);

    switch (rhs->type) {
    case OBJ_TYPE_NIL: {
        object_t *obj = obj_new_other(rhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = NULL;
        if (dictlen && rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else if (!rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else if (!dictlen) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (dictlen && rhs->boolean) {
            obj = obj_new_other(rhs);
        } else if (!rhs->boolean) {
            obj = obj_new_other(rhs);
        } else if (!dictlen) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (dictlen && str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else if (!str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else if (!dictlen) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (dictlen && objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else if (!objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else if (!dictlen) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = NULL;
        if (dictlen && objdict_len(rhs->objdict)) {
            obj = obj_new_other(rhs);
        } else if (!objdict_len(rhs->objdict)) {
            obj = obj_new_other(rhs);
        } else if (!dictlen) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_rhs");
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_compare_and, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_FUNC: {
        object_t *obj = NULL;
        if (dictlen && rhs) {
            obj = obj_new_other(rhs);
        } else if (!rhs) {
            obj = obj_new_other(rhs);
        } else if (!dictlen) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't compare and dict. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_and_dict(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare and dict");
    return_trav(NULL);
}

static object_t *
trv_compare_and_nil(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_NIL);

    switch (rhs->type) {
    case OBJ_TYPE_NIL: {
        object_t *obj = obj_new_other(rhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = obj_new_other(lhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_other(lhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = obj_new_other(lhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = obj_new_other(lhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = obj_new_other(lhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_rhs");
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_compare_and, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_FUNC: {
        object_t *obj = obj_new_other(lhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't compare and nil. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_and_nil(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare and nil");
    return_trav(NULL);
}

static object_t *
trv_compare_and_func(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_FUNC);

    switch (rhs->type) {
    case OBJ_TYPE_NIL: {
        object_t *obj = obj_new_other(rhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = NULL;
        if (lhs && rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else if (!rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else if (!lhs) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (lhs && rhs->boolean) {
            obj = obj_new_other(rhs);
        } else if (!rhs->boolean) {
            obj = obj_new_other(rhs);
        } else if (!lhs) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (lhs && str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else if (!str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else if (!lhs) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (lhs && objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else if (!objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else if (!lhs) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = NULL;
        if (lhs && objdict_len(rhs->objdict)) {
            obj = obj_new_other(rhs);
        } else if (!objdict_len(rhs->objdict)) {
            obj = obj_new_other(rhs);
        } else if (!lhs) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_rhs");
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_compare_and, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_FUNC: {
        object_t *obj = NULL;
        if (lhs && rhs) {
            obj = obj_new_other(rhs);
        } else if (!rhs) {
            obj = obj_new_other(rhs);
        } else if (!lhs) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't compare and func. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_and_func(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare and array");
    return_trav(NULL);
}

static object_t *
trv_compare_and(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();

    switch (lhs->type) {
    case OBJ_TYPE_NIL: {
        tcheck("call trv_compare_and_nil");
        object_t *obj = trv_compare_and_nil(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        tcheck("call trv_compare_and_int");
        object_t *obj = trv_compare_and_int(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        tcheck("call trv_compare_and_bool");
        object_t *obj = trv_compare_and_bool(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        tcheck("call trv_compare_and_string");
        object_t *obj = trv_compare_and_string(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        tcheck("call trv_compare_and_array");
        object_t *obj = trv_compare_and_array(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        tcheck("call trv_compare_and_dict");
        object_t *obj = trv_compare_and_dict(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_lhs with trv_compare_and");
        object_t *obj = trv_roll_identifier_lhs(ast, lhs, rhs, trv_compare_and, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_FUNC: {
        tcheck("call trv_compare_and_func");
        object_t *obj = trv_compare_and_func(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, lhs);
        if (!val) {
            ast_set_error_detail(ast, "can't compare and. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_and(ast, val, rhs, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare and");
    return_trav(NULL);
}

static object_t *
trv_and_test(ast_t *ast, const node_t *node, int dep) {
    tready();
    assert(node->type == NODE_TYPE_AND_TEST);
    node_and_test_t *and_test = node->real;

    node_t *lnode = nodearr_get(and_test->nodearr, 0);
    tcheck("call _trv_traverse with not_test");
    object_t *lhs = _trv_traverse(ast, lnode, dep+1);
    if (ast_has_error(ast)) {
        return_trav(NULL);
    }
    assert(lhs);

    for (int i = 1; i < nodearr_len(and_test->nodearr); ++i) {
        node_t *rnode = nodearr_get(and_test->nodearr, i);
        tcheck("call _trv_traverse with not_test");
        object_t *rhs = _trv_traverse(ast, rnode, dep+1);
        if (ast_has_error(ast)) {
            return_trav(NULL);
        }
        if (!rhs) {
            return_trav(lhs);
        }

        tcheck("call trv_compare_and");
        object_t *result = trv_compare_and(ast, lhs, rhs, dep+1);
        if (ast_has_error(ast)) {
            obj_del(lhs);
            obj_del(rhs);
            return_trav(NULL);
        }
        assert(result);

        obj_del(lhs);
        obj_del(rhs);
        lhs = result;
    }

    return_trav(lhs);
}

static object_t *
trv_compare_not(ast_t *ast, const object_t *operand, int dep) {
    tready();
    assert(operand);

    switch (operand->type) {
    case OBJ_TYPE_NIL: {
        object_t *obj = obj_new_bool(true);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = obj_new_bool(!operand->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_bool(!operand->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        object_t *var = get_var_ref(ast, str_getc(operand->identifier), dep+1);
        if (!var) {
            ast_set_error_detail(ast, "\"%s\" is not defined compare not", str_getc(operand->identifier));
            return_trav(NULL);
        }

        tcheck("call trv_compare_not");
        object_t *obj = trv_compare_not(ast, var, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = obj_new_bool(!str_len(operand->string));
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = obj_new_bool(!objarr_len(operand->objarr));
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = obj_new_bool(!objdict_len(operand->objdict));
        return_trav(obj);
    } break;
    case OBJ_TYPE_FUNC: {
        object_t *obj = obj_new_bool(!operand);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, operand);
        if (!val) {
            ast_set_error_detail(ast, "can't compare not. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_not(ast, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare not");
    return_trav(NULL);
}

static object_t *
trv_not_test(ast_t *ast, const node_t *node, int dep) {
    tready();
    node_not_test_t *not_test = node->real;

    if (not_test->not_test) {
        object_t *operand = _trv_traverse(ast, not_test->not_test, dep+1);
        if (ast_has_error(ast)) {
            return_trav(NULL);
        }
        if (!operand) {
            ast_set_error_detail(ast, "failed to not test");
            return_trav(NULL);
        }

        tcheck("call trv_compare_not");
        object_t *obj = trv_compare_not(ast, operand, dep+1);
        return_trav(obj);
    } else if (not_test->comparison) {
        object_t *obj = _trv_traverse(ast, not_test->comparison, dep+1);
        return_trav(obj);
    }

    return_trav(NULL);
}

static object_t *
trv_compare_comparison_eq_int(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_INTEGER);

    switch (rhs->type) {
    default:
        ast_set_error_detail(ast, "can't compare equal with int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_NIL: {
        object_t *obj = obj_new_bool(false);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = obj_new_bool(lhs->lvalue == rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_bool(lhs->lvalue == rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_rhs");
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_compare_comparison_eq, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't comparison eq int. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_comparison_eq_int(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison eq int");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_eq_bool(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_BOOL);

    switch (rhs->type) {
    default:
        ast_set_error_detail(ast, "can't compare equal with bool");
        return_trav(NULL);
        break;
    case OBJ_TYPE_NIL: {
        object_t *obj = obj_new_bool(false);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = obj_new_bool(lhs->boolean == rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_bool(lhs->boolean == rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_compare_comparison_eq, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't comparison eq bool. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_comparison_not_eq_bool(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison eq bool");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_eq_string(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_STRING);

    switch (rhs->type) {
    default:
        ast_set_error_detail(ast, "can't compare equal with string");
        return_trav(NULL);
        break;
    case OBJ_TYPE_NIL: {
        object_t *obj = obj_new_bool(false);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_rhs");
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_compare_comparison_eq, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = obj_new_bool(cstr_eq(str_getc(lhs->string), str_getc(rhs->string)));
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't comparison eq string. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_comparison_eq_string(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison string");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_eq_array(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_ARRAY);

    switch (rhs->type) {
    default:
        ast_set_error_detail(ast, "can't compare equal with array");
        return_trav(NULL);
        break;
    case OBJ_TYPE_NIL: {
        object_t *obj = obj_new_bool(false);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_rhs");
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_compare_comparison_eq, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't comparison eq array. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_comparison_not_eq_array(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison array");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_eq_dict(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_DICT);
    ast_set_error_detail(ast, "can't compare equal with dict");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_eq_nil(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_NIL);

    switch (rhs->type) {
    default: {
        object_t *obj = obj_new_bool(false);
        return_trav(obj);        
    } break;
    case OBJ_TYPE_NIL: {
        object_t *obj = obj_new_bool(true);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't comparison eq nil. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_comparison_not_eq_nil(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison eq nil");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_eq_func(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_FUNC);

    switch (rhs->type) {
    default:
        ast_set_error_detail(ast, "can't compare equal with func");
        return_trav(NULL);
        break;
    case OBJ_TYPE_NIL: {
        object_t *obj = obj_new_bool(false);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_rhs");
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_compare_comparison_eq, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_FUNC: {
        object_t *obj = obj_new_bool(lhs == rhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't comparison eq func. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_comparison_not_eq_func(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison array");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_eq_index(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_INDEX);

    object_t *val = trv_get_value_of_index_obj(ast, lhs);
    if (!val) {
        ast_set_error_detail(ast, "index object value is null");
        return_trav(NULL);
    }

    object_t *ret = trv_compare_comparison_eq(ast, val, rhs, dep+1);
    obj_del(val);
    return ret;
}

static object_t *
trv_compare_comparison_eq(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();

    switch (lhs->type) {
    case OBJ_TYPE_NIL: {
        tcheck("call trv_compare_comparison_eq_nil");
        object_t *obj = trv_compare_comparison_eq_nil(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        tcheck("call trv_compare_comparison_eq_int");
        object_t *obj = trv_compare_comparison_eq_int(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        tcheck("call trv_compare_comparison_eq_bool");
        object_t *obj = trv_compare_comparison_eq_bool(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        tcheck("call trv_compare_comparison_eq_string");
        object_t *obj = trv_compare_comparison_eq_string(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        tcheck("call trv_compare_comparison_eq_array");
        object_t *obj = trv_compare_comparison_eq_array(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        tcheck("call trv_compare_comparison_eq_dict");
        object_t *obj = trv_compare_comparison_eq_dict(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_lhs with trv_compare_comparison_eq");
        object_t *obj = trv_roll_identifier_lhs(ast, lhs, rhs, trv_compare_comparison_eq, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_FUNC: {
        tcheck("call trv_compare_comparison_eq_func");
        object_t *obj = trv_compare_comparison_eq_func(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        tcheck("call trv_compare_comparison_eq_index");
        object_t *obj = trv_compare_comparison_eq_index(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison eq");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_not_eq_int(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_INTEGER);

    switch (rhs->type) {
    default:
        ast_set_error_detail(ast, "can't compare not equal with int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_NIL: {
        object_t *obj = obj_new_bool(true);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = obj_new_bool(lhs->lvalue != rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_bool(lhs->lvalue != rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_rhs with trv_compare_comparison_not_eq");
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_compare_comparison_not_eq, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't comparison not eq int. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_comparison_not_eq_int(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison not eq int");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_not_eq_bool(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_BOOL);

    switch (rhs->type) {
    default:
        ast_set_error_detail(ast, "can't compare not equal with bool");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = obj_new_bool(lhs->boolean != rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_bool(lhs->boolean != rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_rhs");
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_compare_comparison_not_eq, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't comparison not eq bool. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_comparison_not_eq_bool(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison not eq bool");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_not_eq_string(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_STRING);

    switch (rhs->type) {
    default:
        ast_set_error_detail(ast, "can't compare not equal with string");
        return_trav(NULL);
        break;
    case OBJ_TYPE_NIL: {
        object_t *obj = obj_new_bool(true);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_compare_comparison_not_eq, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = obj_new_bool(!cstr_eq(str_getc(lhs->string), str_getc(rhs->string)));
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't comparison not eq string. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_comparison_not_eq_string(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison not eq string");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_not_eq_array(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_ARRAY);

    switch (rhs->type) {
    default:
        ast_set_error_detail(ast, "can't compare not equal with array");
        return_trav(NULL);
        break;
    case OBJ_TYPE_NIL: {
        object_t *obj = obj_new_bool(true);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_rhs");
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_compare_comparison_not_eq, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't comparison not eq array. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_comparison_not_eq_array(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison not eq array");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_not_eq_dict(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_DICT);
    ast_set_error_detail(ast, "can't compare not equal with dict");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_not_eq_nil(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_NIL);

    switch (rhs->type) {
    default: {
        object_t *obj = obj_new_bool(true);
        return_trav(obj);
    } break;
    case OBJ_TYPE_NIL: {
        object_t *obj = obj_new_bool(false);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_lhs with trv_compare_comparison_not_eq");
        object_t *obj = trv_roll_identifier_lhs(ast, lhs, rhs, trv_compare_comparison_not_eq, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't comparison not eq nil. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_comparison_not_eq_nil(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison not eq nil");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_not_eq_func(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_FUNC);

    switch (rhs->type) {
    default:
        ast_set_error_detail(ast, "can't compare not equal with func");
        return_trav(NULL);
        break;
    case OBJ_TYPE_NIL: {
        object_t *obj = obj_new_bool(true);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_rhs");
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_compare_comparison_not_eq, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_FUNC: {
        object_t *obj = obj_new_bool(lhs != rhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't comparison not eq func. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_comparison_not_eq_func(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison not eq array");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_not_eq(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();

    switch (lhs->type) {
    case OBJ_TYPE_NIL: {
        tcheck("call trv_compare_comparison_not_eq_nil");
        object_t *obj = trv_compare_comparison_not_eq_nil(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        tcheck("call trv_compare_comparison_not_eq_int");
        object_t *obj = trv_compare_comparison_not_eq_int(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        tcheck("call trv_compare_comparison_not_eq_bool");
        object_t *obj = trv_compare_comparison_not_eq_bool(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        tcheck("call trv_compare_comparison_not_eq_string");
        object_t *obj = trv_compare_comparison_not_eq_string(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        tcheck("call trv_compare_comparison_not_eq_array");
        object_t *obj = trv_compare_comparison_not_eq_array(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        tcheck("call trv_compare_comparison_not_eq_dict");
        object_t *obj = trv_compare_comparison_not_eq_dict(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_lhs with trv_compare_comparison_not_eq");
        object_t *obj = trv_roll_identifier_lhs(ast, lhs, rhs, trv_compare_comparison_not_eq, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_FUNC: {
        tcheck("call trv_compare_comparison_not_eq_func");
        object_t *obj = trv_compare_comparison_not_eq_func(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, lhs);
        if (!val) {
            ast_set_error_detail(ast, "can't comparison not eq. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_comparison_not_eq(ast, val, rhs, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison not eq");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_lte_int(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_INTEGER);

    switch (rhs->type) {
    default:
        ast_set_error_detail(ast, "can't compare lte with int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = obj_new_bool(lhs->lvalue <= rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_bool(lhs->lvalue <= rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_rhs with trv_compare_comparison_lte");
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_compare_comparison_lte, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't comparison lte int. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_comparison_lte_int(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison lte int");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_lte_bool(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_BOOL);

    switch (rhs->type) {
    default:
        ast_set_error_detail(ast, "can't compare lte with int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = obj_new_bool(lhs->boolean <= rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_bool(lhs->boolean <= rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_rhs with trv_compare_comparison_lte");
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_compare_comparison_lte, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't comparison lte bool. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_comparison_lte_bool(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison lte bool");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_lte(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();

    switch (lhs->type) {
    default:
        ast_set_error_detail(ast, "can't compare with lte");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INTEGER: {
        tcheck("call trv_compare_comparison_lte_int");
        object_t *obj = trv_compare_comparison_lte_int(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        tcheck("call trv_compare_comparison_lte_bool");
        object_t *obj = trv_compare_comparison_lte_bool(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_lhs with trv_compare_comparison_lte");
        object_t *obj = trv_roll_identifier_lhs(ast, lhs, rhs, trv_compare_comparison_lte, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, lhs);
        if (!val) {
            ast_set_error_detail(ast, "can't comparison lte. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_comparison_lte(ast, val, rhs, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison not eq");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_gte_int(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_INTEGER);

    switch (rhs->type) {
    default:
        ast_set_error_detail(ast, "can't compare gte with int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = obj_new_bool(lhs->lvalue >= rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_bool(lhs->lvalue >= rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_rhs with trv_compare_comparison_gte");
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_compare_comparison_gte, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't comparison gte int. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_comparison_gte_int(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison gte int");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_gte_bool(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_BOOL);

    switch (rhs->type) {
    default:
        ast_set_error_detail(ast, "can't compare gte with int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = obj_new_bool(lhs->boolean >= rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_bool(lhs->boolean >= rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_rhs with trv_compare_comparison_gte");
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_compare_comparison_gte, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't comparison gte bool. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_comparison_gte_bool(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison gte bool");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_gte(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();

    switch (lhs->type) {
    default:
        ast_set_error_detail(ast, "can't compare with gte");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INTEGER: {
        tcheck("call trv_compare_comparison_gte_int");
        object_t *obj = trv_compare_comparison_gte_int(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        tcheck("call trv_compare_comparison_gte_bool");
        object_t *obj = trv_compare_comparison_gte_bool(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_lhs with trv_compare_comparison_gte");
        object_t *obj = trv_roll_identifier_lhs(ast, lhs, rhs, trv_compare_comparison_gte, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, lhs);
        if (!val) {
            ast_set_error_detail(ast, "can't comparison gte. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_comparison_gte(ast, val, rhs, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison gte");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_lt_int(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_INTEGER);

    switch (rhs->type) {
    default:
        ast_set_error_detail(ast, "can't compare lt with int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = obj_new_bool(lhs->lvalue < rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_bool(lhs->lvalue < rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_rhs with trv_compare_comparison_lt");
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_compare_comparison_lt, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't comparison lt int. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_comparison_lt_int(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison lt int");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_lt_bool(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_BOOL);

    switch (rhs->type) {
    default:
        ast_set_error_detail(ast, "can't compare lt with int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = obj_new_bool(lhs->boolean < rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_bool(lhs->boolean < rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_rhs with trv_compare_comparison_lt");
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_compare_comparison_lt, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't comparison lt bool. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_comparison_lt_bool(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison lt bool");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_lt(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();

    switch (lhs->type) {
    default:
        ast_set_error_detail(ast, "can't compare with lt");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INTEGER: {
        tcheck("call trv_compare_comparison_lt_int");
        object_t *obj = trv_compare_comparison_lt_int(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        tcheck("call trv_compare_comparison_lt_bool");
        object_t *obj = trv_compare_comparison_lt_bool(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_lhs with trv_compare_comparison_lt");
        object_t *obj = trv_roll_identifier_lhs(ast, lhs, rhs, trv_compare_comparison_lt, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, lhs);
        if (!val) {
            ast_set_error_detail(ast, "can't comparison lt. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_comparison_lt(ast, val, rhs, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison lt");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_gt_int(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_INTEGER);

    switch (rhs->type) {
    default:
        ast_set_error_detail(ast, "can't compare gt with int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = obj_new_bool(lhs->lvalue > rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_bool(lhs->lvalue > rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_rhs with trv_compare_comparison_gt");
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_compare_comparison_gt, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't comparison gt int. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_comparison_gt_int(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison gt int");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_gt_bool(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_BOOL);

    switch (rhs->type) {
    default:
        ast_set_error_detail(ast, "can't compare gt with int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = obj_new_bool(lhs->boolean > rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_bool(lhs->boolean > rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_rhs with trv_compare_comparison_gt");
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_compare_comparison_gt, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't comparison gt bool. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_comparison_gt_bool(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison gt bool");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_gt(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();

    switch (lhs->type) {
    default:
        ast_set_error_detail(ast, "can't compare with gt");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INTEGER: {
        tcheck("call trv_compare_comparison_gt_int");
        object_t *obj = trv_compare_comparison_gt_int(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        tcheck("call trv_compare_comparison_gt_bool");
        object_t *obj = trv_compare_comparison_gt_bool(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_lhs with trv_compare_comparison_gt");
        object_t *obj = trv_roll_identifier_lhs(ast, lhs, rhs, trv_compare_comparison_gt, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, lhs);
        if (!val) {
            ast_set_error_detail(ast, "can't comparison gt. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_compare_comparison_gt(ast, val, rhs, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison gt");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison(ast_t *ast, const object_t *lhs, const node_comp_op_t *comp_op, const object_t *rhs, int dep) {
    tready();

    switch (comp_op->op) {
    default: break;
    case OP_EQ: {
        tcheck("call trv_compare_comparison_eq");
        object_t *obj = trv_compare_comparison_eq(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OP_NOT_EQ: {
        tcheck("call trv_compare_comparison_not_eq");
        object_t *obj = trv_compare_comparison_not_eq(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OP_LTE: {
        tcheck("call trv_compare_comparison_lte");
        object_t *obj = trv_compare_comparison_lte(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OP_GTE: {
        tcheck("call trv_compare_comparison_gte");
        object_t *obj = trv_compare_comparison_gte(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OP_LT: {
        tcheck("call trv_compare_comparison_lt");
        object_t *obj = trv_compare_comparison_lt(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OP_GT: {
        tcheck("call trv_compare_comparison_gt");
        object_t *obj = trv_compare_comparison_gt(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison");
    return_trav(NULL);
}

static object_t *
trv_comparison(ast_t *ast, const node_t *node, int dep) {
    tready();
    node_comparison_t *comparison = node->real;
    assert(comparison);

    if (nodearr_len(comparison->nodearr) == 1) {
        node_t *node = nodearr_get(comparison->nodearr, 0);
        assert(node->type == NODE_TYPE_ASSCALC);
        tcheck("call _trv_traverse with asscalc");
        object_t *result = _trv_traverse(ast, node, dep+1);
        return_trav(result);
    } else if (nodearr_len(comparison->nodearr) >= 3) {
        node_t *lnode = nodearr_get(comparison->nodearr, 0);
        assert(lnode->type == NODE_TYPE_ASSCALC);
        tcheck("call _trv_traverse with asscalc");
        object_t *lhs = _trv_traverse(ast, lnode, dep+1);
        if (ast_has_error(ast)) {
            return_trav(NULL);
        }
        assert(lhs);
        
        for (int i = 1; i < nodearr_len(comparison->nodearr); i += 2) {
            node_t *node = nodearr_get(comparison->nodearr, i);
            assert(node->type == NODE_TYPE_COMP_OP);
            node_comp_op_t *node_comp_op = node->real;
            assert(node_comp_op);

            node_t *rnode = nodearr_get(comparison->nodearr, i+1);
            assert(rnode->type == NODE_TYPE_ASSCALC);
            assert(rnode);
            tcheck("call _trv_traverse with asscalc");
            object_t *rhs = _trv_traverse(ast, rnode, dep+1);
            if (ast_has_error(ast)) {
                obj_del(lhs);
                return_trav(NULL);
            }
            assert(rnode);

            tcheck("call trv_compare_comparison");
            object_t *result = trv_compare_comparison(ast, lhs, node_comp_op, rhs, dep+1);
            if (ast_has_error(ast)) {
                obj_del(lhs);
                obj_del(rhs);
                return_trav(NULL);                
            }

            obj_del(lhs);
            obj_del(rhs);
            lhs = result;
        }

        return_trav(lhs);
    }

    assert(0 && "impossible. failed to traverse comparison");
    return_trav(NULL);
}

static object_t *
trv_calc_expr_add_int(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_INTEGER);

    switch (rhs->type) {
    default:
        ast_set_error_detail(ast, "can't add with int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = obj_new_int(lhs->lvalue + rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_int(lhs->lvalue + rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_rhs");
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_calc_expr_add, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't add with int. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_calc_expr_add(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc expr int");
    return_trav(NULL);
}

static object_t *
trv_calc_expr_add_bool(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_BOOL);

    switch (rhs->type) {
    default:
        ast_set_error_detail(ast, "can't add with bool");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = obj_new_int(lhs->boolean + rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_int(lhs->boolean + rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_rhs");
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_calc_expr_add, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't add with bool. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_calc_expr_add(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc expr bool");
    return_trav(NULL);
}

static object_t *
trv_calc_expr_add_string(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_STRING);

    switch (rhs->type) {
    default:
        ast_set_error_detail(ast, "can't add with string");
        return_trav(NULL);
        break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_rhs");
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_calc_expr_add, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        string_t *s = str_new();
        str_app(s, str_getc(lhs->string));
        str_app(s, str_getc(rhs->string));
        object_t *obj = obj_new_str(s);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't add with string. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_calc_expr_add(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc expr string");
    return_trav(NULL);
}

static object_t *
trv_calc_expr_add(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();

    switch (lhs->type) {
    default: {
        ast_set_error_detail(ast, "can't add");
        return_trav(NULL);
    } break;
    case OBJ_TYPE_INTEGER: {
        tcheck("call trv_calc_expr_add_int");
        object_t *obj = trv_calc_expr_add_int(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        tcheck("call trv_calc_expr_add_bool");
        object_t *obj = trv_calc_expr_add_bool(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        tcheck("call trv_calc_expr_add_string");
        object_t *obj = trv_calc_expr_add_string(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_lhs with trv_calc_expr_add");
        object_t *obj = trv_roll_identifier_lhs(ast, lhs, rhs, trv_calc_expr_add, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, lhs);
        if (!val) {
            ast_set_error_detail(ast, "can't add with string. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_calc_expr_add(ast, val, rhs, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc expr add");
    return_trav(NULL);
}

static object_t *
trv_calc_expr_sub_int(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_INTEGER);

    switch (rhs->type) {
    default:
        ast_set_error_detail(ast, "can't sub with int");
        return_trav(NULL);
        break;    
    case OBJ_TYPE_INTEGER: {
        object_t *obj = obj_new_int(lhs->lvalue - rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_int(lhs->lvalue - rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_rhs");
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_calc_expr_sub, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't sub with int. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_calc_expr_sub(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc expr sub int");
    return_trav(NULL);
}

static object_t *
trv_calc_expr_sub_bool(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_BOOL);

    switch (rhs->type) {
    default:
        ast_set_error_detail(ast, "can't sub with bool");
        return_trav(NULL);
        break;    
    case OBJ_TYPE_INTEGER: {
        object_t *obj = obj_new_int(lhs->boolean - rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_int(lhs->boolean - rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("trv_roll_identifier_rhs");
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_calc_expr_sub, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't sub with bool. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_calc_expr_sub(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc expr sub bool");
    return_trav(NULL);
}

static object_t *
trv_calc_expr_sub(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();

    switch (lhs->type) {
    default: {
        ast_set_error_detail(ast, "can't sub");
        return_trav(NULL);
    } break;
    case OBJ_TYPE_INTEGER: {
        tcheck("call trv_calc_expr_sub_int");
        object_t *obj = trv_calc_expr_sub_int(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        tcheck("call trv_calc_expr_sub_bool");
        object_t *obj = trv_calc_expr_sub_bool(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_rhs");
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_calc_expr_sub, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, lhs);
        if (!val) {
            ast_set_error_detail(ast, "can't sub. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_calc_expr_sub(ast, val, rhs, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc expr sub");
    return_trav(NULL);
}

static object_t *
trv_calc_expr(ast_t *ast, const object_t *lhs, const node_add_sub_op_t *add_sub_op, const object_t *rhs, int dep) {
    tready();

    switch (add_sub_op->op) {
    default: break;
    case OP_ADD: {
        tcheck("call trv_calc_expr_add");
        object_t *obj = trv_calc_expr_add(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OP_SUB: {
        tcheck("call trv_calc_expr_sub");
        object_t *obj = trv_calc_expr_sub(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc expr");
    return_trav(NULL);
}

static object_t *
trv_expr(ast_t *ast, const node_t *node, int dep) {
    tready();
    node_expr_t *expr = node->real;
    assert(expr);

    if (nodearr_len(expr->nodearr) == 1) {
        node_t *node = nodearr_get(expr->nodearr, 0);
        tcheck("call _trv_traverse");
        object_t *result =  _trv_traverse(ast, node, dep+1);
        return_trav(result);
    } else if (nodearr_len(expr->nodearr) >= 3) {
        node_t *lnode = nodearr_get(expr->nodearr, 0);
        object_t *lhs = _trv_traverse(ast, lnode, dep+1);
        if (ast_has_error(ast)) {
            return_trav(NULL);
        }
        assert(lhs);
        
        for (int i = 1; i < nodearr_len(expr->nodearr); i += 2) {
            node_t *node = nodearr_get(expr->nodearr, i);
            node_add_sub_op_t *op = node->real;
            assert(op);

            node_t *rnode = nodearr_get(expr->nodearr, i+1);
            assert(rnode);
            tcheck("call _trv_traverse");
            object_t *rhs = _trv_traverse(ast, rnode, dep+1);
            if (ast_has_error(ast)) {
                obj_del(lhs);
                return_trav(NULL);
            }
            assert(rnode);

            tcheck("call trv_calc_expr");
            object_t *result = trv_calc_expr(ast, lhs, op, rhs, dep+1);
            if (ast_has_error(ast)) {
                obj_del(lhs);
                obj_del(rhs);
                return_trav(NULL);                
            }

            obj_del(lhs);
            obj_del(rhs);
            lhs = result;
        }

        return_trav(lhs);
    }

    assert(0 && "impossible. failed to traverse expr");
    return_trav(NULL);
}

static object_t *
trv_calc_term_mul_int(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_INTEGER);

    switch (rhs->type) {
    default:
        ast_set_error_detail(ast, "can't mul with int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = obj_new_int(lhs->lvalue * rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_int(lhs->lvalue * rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_rhs");
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_calc_term_mul, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING:
        err_die("TODO: mul string");
        break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't mul with int. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_calc_term_mul(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc term mul int");
    return_trav(NULL);
}

static object_t *
trv_calc_term_mul_bool(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_BOOL);

    switch (rhs->type) {
    default:
        ast_set_error_detail(ast, "can't mul with bool");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = obj_new_int(lhs->boolean * rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_int(lhs->boolean * rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_rhs");
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_calc_term_mul, dep+1); 
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't mul with bool. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_calc_term_mul(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc term mul bool");
    return_trav(NULL);
}

static object_t *
trv_calc_term_mul_string(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_STRING);

    switch (rhs->type) {
    default:
        ast_set_error_detail(ast, "can't mul with string");
        return_trav(NULL);
        break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_rhs");
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_calc_term_mul, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING:
        err_die("TODO: mul string 2");
        break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't mul with string. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_calc_term_mul(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }
 
    assert(0 && "impossible. failed to calc term mul string");
    return_trav(NULL);
}

static object_t *
trv_calc_term_mul(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();

    switch (lhs->type) {
    default: {
        ast_set_error_detail(ast, "can't mul");
        return_trav(NULL);
    } break;
    case OBJ_TYPE_INTEGER: {
        tcheck("call trv_calc_term_mul_int");
        object_t *obj = trv_calc_term_mul_int(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        tcheck("call trv_calc_term_mul_bool");
        object_t *obj = trv_calc_term_mul_bool(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_lhs with trv_calc_term_mul");
        object_t *obj = trv_roll_identifier_lhs(ast, lhs, rhs, trv_calc_term_mul, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        tcheck("call trv_calc_term_mul_string");
        object_t *obj = trv_calc_term_mul_string(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, lhs);
        if (!val) {
            ast_set_error_detail(ast, "can't mul. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_calc_term_mul(ast, val, rhs, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc term mul");
    return_trav(NULL);
}

static object_t *
trv_calc_term_div_int(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_INTEGER);

    switch (rhs->type) {
    default:
        ast_set_error_detail(ast, "can't division with int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INTEGER: {
        if (!rhs->lvalue) {
            ast_set_error_detail(ast, "zero division error");
            return_trav(NULL);
        }
        object_t *obj = obj_new_int(lhs->lvalue / rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        if (!rhs->boolean) {
            ast_set_error_detail(ast, "zero division error (2)");
            return_trav(NULL);
        }
        object_t *obj = obj_new_int(lhs->lvalue / rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("trv_roll_identifier_rhs");
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_calc_term_div, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't division with int. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_calc_term_div(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc term div int");
    return_trav(NULL);
}

static object_t *
trv_calc_term_div_bool(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_BOOL);

    switch (rhs->type) {
    default:
        ast_set_error_detail(ast, "can't division with bool");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INTEGER:
        if (!rhs->lvalue) {
            ast_set_error_detail(ast, "zero division error (3)");
            return_trav(NULL);
        }
        return obj_new_int(lhs->boolean / rhs->lvalue);
    case OBJ_TYPE_BOOL:
        if (!rhs->boolean) {
            ast_set_error_detail(ast, "zero division error (4)");
            return_trav(NULL);
        }
        return obj_new_int(lhs->boolean / rhs->boolean);
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_rhs");
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_calc_term_div, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        if (!val) {
            ast_set_error_detail(ast, "can't division with bool. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_calc_term_div(ast, lhs, val, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc term div bool");
    return_trav(NULL);
}

static object_t *
trv_calc_term_div(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();

    switch (lhs->type) {
    default: {
        ast_set_error_detail(ast, "can't division");
        return_trav(NULL);
    } break;
    case OBJ_TYPE_INTEGER: {
        tcheck("call trv_calc_term_div_int");
        object_t *obj = trv_calc_term_div_int(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        tcheck("call trv_calc_term_div_bool");
        object_t *obj = trv_calc_term_div_bool(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_roll_identifier_rhs");
        object_t *obj = trv_roll_identifier_rhs(ast, lhs, rhs, trv_calc_term_div, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, lhs);
        if (!val) {
            ast_set_error_detail(ast, "can't division. index object value is null");
            return_trav(NULL);
        }
        object_t *obj = trv_calc_term_div(ast, val, rhs, dep+1);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc term div");
    return_trav(NULL);
}

static object_t *
trv_calc_term(ast_t *ast, const object_t *lhs, const node_mul_div_op_t *mul_div_op, const object_t *rhs, int dep) {
    tready();

    switch (mul_div_op->op) {
    default: break;
    case OP_MUL: {
        tcheck("call trv_calc_term_mul");
        object_t *obj = trv_calc_term_mul(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OP_DIV: {
        tcheck("call trv_calc_term_div");
        object_t *obj = trv_calc_term_div(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc term");
    return_trav(NULL);
}

static object_t *
trv_term(ast_t *ast, const node_t *node, int dep) {
    node_term_t *term = node->real;
    tready();
    assert(term);

    if (nodearr_len(term->nodearr) == 1) {
        node_t *node = nodearr_get(term->nodearr, 0);
        assert(node->type == NODE_TYPE_DOT);
        tcheck("call _trv_traverse with dot");
        object_t *result = _trv_traverse(ast, node, dep+1);
        return_trav(result);
    } else if (nodearr_len(term->nodearr) >= 3) {
        node_t *lnode = nodearr_get(term->nodearr, 0);
        assert(lnode->type == NODE_TYPE_DOT);
        tcheck("call _trv_traverse with dot");
        object_t *lhs = _trv_traverse(ast, lnode, dep+1);
        if (ast_has_error(ast)) {
            return_trav(NULL);
        }
        assert(lhs);
        
        for (int i = 1; i < nodearr_len(term->nodearr); i += 2) {
            node_t *node = nodearr_get(term->nodearr, i);
            assert(node->type == NODE_TYPE_MUL_DIV_OP);
            node_mul_div_op_t *op = node->real;
            assert(op);

            node_t *rnode = nodearr_get(term->nodearr, i+1);
            assert(rnode->type == NODE_TYPE_DOT);
            tcheck("call _trv_traverse with index");
            object_t *rhs = _trv_traverse(ast, rnode, dep+1);
            if (ast_has_error(ast)) {
                obj_del(lhs);
                return_trav(NULL);
            }
            assert(rnode);

            tcheck("trv_calc_term");
            object_t *result = trv_calc_term(ast, lhs, op, rhs, dep+1);
            if (ast_has_error(ast)) {
                obj_del(lhs);
                obj_del(rhs);
                return_trav(NULL);                
            }

            obj_del(lhs);
            obj_del(rhs);
            lhs = result;
        }

        return_trav(lhs);
    }

    assert(0 && "impossible. failed to traverse term");
    return_trav(NULL);
}

static object_t *
trv_dot(ast_t *ast, const node_t *node, int dep) {
    node_dot_t *dot = node->real;
    tready();
    assert(dot);

    if (nodearr_len(dot->nodearr) == 1) {
        node_t *node = nodearr_get(dot->nodearr, 0);
        assert(node->type == NODE_TYPE_INDEX);
        tcheck("call _trv_traverse with dot");
        object_t *result = _trv_traverse(ast, node, dep+1);
        return_trav(result);
    } else if (nodearr_len(dot->nodearr) >= 3) {
        node_t *lnode = nodearr_get(dot->nodearr, 0);
        assert(lnode->type == NODE_TYPE_INDEX);
        tcheck("call _trv_traverse with dot");
        object_t *lhs = _trv_traverse(ast, lnode, dep+1);
        if (ast_has_error(ast)) {
            return_trav(NULL);
        }
        assert(lhs);

        for (int i = 1; i < nodearr_len(dot->nodearr); i += 2) {
            node_t *node = nodearr_get(dot->nodearr, i);
            assert(node->type == NODE_TYPE_DOT_OP);
            node_dot_op_t *op = node->real;
            assert(op);

            node_t *rnode = nodearr_get(dot->nodearr, i+1);
            assert(rnode->type == NODE_TYPE_INDEX);
            tcheck("call _trv_traverse with index");
            object_t *rhs = _trv_traverse(ast, rnode, dep+1);
            if (ast_has_error(ast)) {
                obj_del(lhs);
                return_trav(NULL);
            }
            assert(rnode);

            ast->dot_ref_owner = lhs;
            object_t *result = _trv_traverse(ast, rnode, dep+1);
            if (ast_has_error(ast)) {
                obj_del(lhs);
                return_trav(NULL);
            }
            if (!result) {
                ast_set_error_detail(ast, "result is null");
                obj_del(lhs);
                return_trav(NULL);
            }
            ast->dot_ref_owner = NULL;

            obj_del(lhs);
            obj_del(rhs);
            lhs = result;
        }

        return_trav(lhs);
    }

    assert(0 && "impossible. failed to traverse dot");
    return_trav(NULL);
}

static object_t *
trv_index(ast_t *ast, const node_t *node, int dep) {
    node_index_t *index_node = node->real;
    tready();
    assert(index_node);
    object_t *operand = NULL;
    object_t *ref_operand = NULL;
    object_t *ret = NULL;

    operand = _trv_traverse(ast, index_node->factor, dep+1);
    if (ast_has_error(ast)) {
        return_trav(NULL);
    }
    if (!operand) {
        ast_set_error_detail(ast, "not found operand in index access");
        return_trav(NULL);
    }

    // operand is identifier?
    ref_operand = operand;
    if (operand->type == OBJ_TYPE_IDENTIFIER) {
        if (!nodearr_len(index_node->nodearr)) {
            return_trav(operand);
        }

        // get reference
        ref_operand = pull_in_ref_by(ast, operand);
        if (!ref_operand) {
            // can't index access to null
            ast_set_error_detail(ast, "\"%s\" is not defined", str_getc(operand->identifier));
            obj_del(operand);
            return_trav(NULL);
        }
    }

    if (!nodearr_len(index_node->nodearr)) {
        ret = obj_new_other(ref_operand);
        obj_del(operand);
        return_trav(ret);
    }

    // operand is indexable?
    switch (ref_operand->type) {
    default:
        // not indexable
        if (nodearr_len(index_node->nodearr)) {
            ast_set_error_detail(ast, "operand (%d) is not indexable", ref_operand->type);
            obj_del(operand);
            return_trav(NULL);
        }

        ret = obj_new_other(ref_operand);
        obj_del(operand);
        return_trav(ret);
        break;
    case OBJ_TYPE_IDENTIFIER:
        err_die("impossible. operand is should be not identifier");
        break;
    case OBJ_TYPE_ARRAY:
    case OBJ_TYPE_STRING:
    case OBJ_TYPE_DICT:
        // indexable
        break;
    }

    object_array_t *indices = objarr_new();

    // left priority
    for (int32_t i = 0; i < nodearr_len(index_node->nodearr); ++i) {
        const node_t *node = nodearr_getc(index_node->nodearr, i);
        assert(node);
        object_t *obj = _trv_traverse(ast, node, dep+1);
        assert(obj);
        objarr_moveb(indices, obj);
    }

    // set reference of operand
    ret = obj_new_index(operand, indices);
    return_trav(ret);
}

/**
 * pull-in reference of object by identifier object
 * return reference to variable
 *
 * @param[in] *ast
 * @param[in] *idn_obj identifier object
 *
 * @param return NULL|pointer to object in varmap in current scope
 */
static object_t *
pull_in_ref_by(ast_t *ast, const object_t *idn_obj) {
    assert(idn_obj->type == OBJ_TYPE_IDENTIFIER);

    const char *idn = str_getc(idn_obj->identifier);
    object_t *ref = get_var_ref(ast, idn, 0);
    if (!ref) {
        return NULL;
    }
    if (ref->type == OBJ_TYPE_IDENTIFIER) {
        return pull_in_ref_by(ast, ref);
    }

    return ref;
}

static object_t *
trv_calc_asscalc_ass_idn(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_IDENTIFIER);

    const char *idn = str_getc(lhs->identifier);

    switch (rhs->type) {
    default: {
        move_var(ast, idn, obj_new_other(rhs), dep+1);
        object_t *obj = obj_new_other(rhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = trv_get_value_of_index_obj(ast, rhs);
        move_var(ast, idn, val, dep+1);
        object_t *ret = obj_new_other(val);
        return_trav(ret);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        object_t *rval = pull_in_ref_by(ast, rhs);
        if (!rval) {
            ast_set_error_detail(ast, "\"%s\" is not defined in asscalc ass idn", str_getc(rhs->identifier));
            return_trav(NULL);
        }

        // ここでmove_varにrvalの参照を渡すと、変数の変数への代入は参照になる
        // ↓ではコピーを渡しているので変数の変数への代入は現在はコピーになっている
        // 別名の変数に参照を渡した場合、objdict_clearでダブルフリーが起こる
        // ガーベジコレクションの実装か、メモリ管理の設計（delなど）が必要である
        // TODO: ここの仕様のフィックス
        move_var(ast, idn, obj_new_other(rval), dep+1);
        object_t *obj = obj_new_other(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc asscalc ass idn");
    return_trav(NULL);
}

static object_t *
trv_calc_asscalc_ass(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();

    switch (lhs->type) {
    default:
        ast_set_error_detail(ast, "can't assign to %d", lhs->type);
        return_trav(NULL);
        break;
    case OBJ_TYPE_IDENTIFIER:
        return trv_calc_asscalc_ass_idn(ast, lhs, rhs, dep+1);
        break;
    }

    assert(0 && "impossible. failed to calc asscalc ass");
    return_trav(NULL);
}

static object_t *
trv_calc_asscalc_add_ass_identifier_int(ast_t *ast, object_t *ref_var, const object_t *rhs, int dep) {
    tready();
    assert(ref_var->type == OBJ_TYPE_INTEGER);

    switch (rhs->type) {
    default:
        ast_set_error_detail(ast, "can't add assign to int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INTEGER: {
        ref_var->lvalue += rhs->lvalue;
        object_t *obj = obj_new_other(ref_var);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        ref_var->lvalue += rhs->boolean; 
        object_t *obj = obj_new_other(ref_var);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        const char *idn = str_getc(rhs->identifier);
        object_t *rvar = get_var_ref(ast, idn, dep+1);
        if (!rvar) {
            ast_set_error_detail(ast, "\"%s\" is not defined in add ass identifier int", idn);
            return_trav(NULL);
        }

        tcheck("call trv_calc_asscalc_add_ass_identifier_int");
        object_t *obj = trv_calc_asscalc_add_ass_identifier_int(ast, ref_var, rvar, dep+1);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc asscalc add ass identifier int");
    return_trav(NULL);
}

static object_t *
trv_calc_asscalc_add_ass_identifier_string(ast_t *ast, const object_t *ref_lhs, const object_t *rhs, int dep) {
    tready();
    assert(ref_lhs->type == OBJ_TYPE_STRING);

    switch (rhs->type) {
    default:
        ast_set_error_detail(ast, "can't add assign to string");
        return_trav(NULL);
        break;
    case OBJ_TYPE_STRING: {
        str_app(ref_lhs->string, str_getc(rhs->string));
        object_t *ret = obj_new_other(ref_lhs);
        return_trav(ret);
    } break;
    }

    assert(0 && "impossible. failed to calc asscalc add ass identifier string");
    return_trav(NULL);
}

static object_t *
trv_calc_asscalc_add_ass_identifier(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_IDENTIFIER);

    object_t *ref_lvar = pull_in_ref_by(ast, lhs);
    if (!ref_lvar) {
        ast_set_error_detail(ast, "\"%s\" is not defined in add ass identifier", str_getc(lhs->identifier));
        return_trav(NULL);
    }

    object_t *new_obj = NULL;
    
    switch (ref_lvar->type) {
    default:
        ast_set_error_detail(ast, "not supported add assign to object %d", ref_lvar->type);
        return_trav(NULL);
        break;
    case OBJ_TYPE_NIL:
        ast_set_error_detail(ast, "can't add assign to nil");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INTEGER: {
        tcheck("call trv_calc_asscalc_add_ass_identifier_int");
        new_obj = trv_calc_asscalc_add_ass_identifier_int(ast, ref_lvar, rhs, dep+1);
    } break;
    case OBJ_TYPE_BOOL:
        ast_set_error_detail(ast, "can't add assign to bool");
        return_trav(NULL);
        break;
    case OBJ_TYPE_IDENTIFIER:
        tcheck("call trv_calc_asscalc_add_ass_identifier");
        new_obj = trv_calc_asscalc_add_ass_identifier(ast, ref_lvar, rhs, dep+1);
        vissf("new_obj[%p]", new_obj);
        break;
    case OBJ_TYPE_STRING: {
        tcheck("call trv_calc_asscalc_add_ass_identifier_string");
        new_obj = trv_calc_asscalc_add_ass_identifier_string(ast, ref_lvar, rhs, dep+1);
    } break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(ast, "can't add assign to array");
        return_trav(NULL);
        break;
    case OBJ_TYPE_FUNC:
        ast_set_error_detail(ast, "can't add assign to func");
        return_trav(NULL);
        break;
    }

    return_trav(new_obj);
}

static object_t *
trv_calc_asscalc_add_ass(ast_t *ast, const object_t *lhs, const object_t *rhs, int dep) {
    tready();
    switch (lhs->type) {
    default:
        ast_set_error_detail(ast, "can't add assign to %d", lhs->type);
        return_trav(NULL);
        break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call trv_calc_asscalc_add_ass_identifier");
        object_t *obj = trv_calc_asscalc_add_ass_identifier(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc asscalc add ass");
    return_trav(NULL);
}

static object_t *
trv_calc_asscalc(ast_t *ast, const object_t *lhs, const node_augassign_t *augassign, const object_t *rhs, int dep) {
    tready();

    switch (augassign->op) {
    default: break;
    case OP_ADD_ASS: {
        tcheck("call trv_calc_asscalc_add_ass");
        object_t *obj = trv_calc_asscalc_add_ass(ast, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OP_SUB_ASS:
        err_die("TODO: sub ass");        
        break;
    case OP_MUL_ASS:
        err_die("TODO: mul ass");        
        break;
    case OP_DIV_ASS:
        err_die("TODO: div ass");
        break;
    }

    assert(0 && "impossible. failed to calc asscalc");
    return_trav(NULL);
}

static object_t *
trv_asscalc(ast_t *ast, const node_t *node, int dep) {
    tready();
    node_asscalc_t *asscalc = node->real;
    assert(asscalc);

    if (nodearr_len(asscalc->nodearr) == 1) {
        node_t *node = nodearr_get(asscalc->nodearr, 0);
        assert(node->type == NODE_TYPE_EXPR);
        tcheck("call _trv_traverse");
        object_t *result = _trv_traverse(ast, node, dep+1);
        return_trav(result);
    } else if (nodearr_len(asscalc->nodearr) >= 3) {
        node_t *lnode = nodearr_get(asscalc->nodearr, 0);
        assert(lnode->type == NODE_TYPE_EXPR);
        tcheck("call _trv_traverse");
        object_t *lhs = _trv_traverse(ast, lnode, dep+1);
        if (ast_has_error(ast)) {
            return_trav(NULL);
        }
        assert(lhs);
        
        for (int i = 1; i < nodearr_len(asscalc->nodearr); i += 2) {
            node_t *node = nodearr_get(asscalc->nodearr, i);
            assert(node->type == NODE_TYPE_AUGASSIGN);
            node_augassign_t *op = node->real;
            assert(op);

            node_t *rnode = nodearr_get(asscalc->nodearr, i+1);
            assert(rnode);
            assert(rnode->type == NODE_TYPE_EXPR);
            tcheck("call _trv_traverse");
            object_t *rhs = _trv_traverse(ast, rnode, dep+1);
            if (ast_has_error(ast)) {
                obj_del(lhs);
                return_trav(NULL);
            }
            assert(rnode);

            tcheck("call trv_calc_asscalc");
            object_t *result = trv_calc_asscalc(ast, lhs, op, rhs, dep+1);
            if (ast_has_error(ast)) {
                obj_del(lhs);
                obj_del(rhs);
                return_trav(NULL);                
            }

            obj_del(lhs);
            obj_del(rhs);
            lhs = result;
        }

        return_trav(lhs);
    }

    assert(0 && "impossible. failed to traverse asscalc");
    return_trav(NULL);
}

static object_t *
trv_factor(ast_t *ast, const node_t *node, int dep) {
    tready();
    node_factor_t *factor = node->real;
    assert(factor);

    if (factor->atom) {
        tcheck("call _trv_traverse");
        object_t *obj = _trv_traverse(ast, factor->atom, dep+1);
        return_trav(obj);
    } else if (factor->formula) {
        tcheck("call _trv_traverse");
        object_t *obj = _trv_traverse(ast, factor->formula, dep+1);
        return_trav(obj);
    }

    assert(0 && "impossible. invalid status of factor");
    return_trav(NULL);
}

static object_t *
trv_atom(ast_t *ast, const node_t *node, int dep) {
    tready();
    node_atom_t *atom = node->real;
    assert(atom && node->type == NODE_TYPE_ATOM);

    if (atom->nil) {
        tcheck("call _trv_traverse with nil");
        object_t *obj = _trv_traverse(ast, atom->nil, dep+1);
        return_trav(obj);
    } else if (atom->false_) {
        tcheck("call _trv_traverse with false_");
        object_t *obj = _trv_traverse(ast, atom->false_, dep+1);
        return_trav(obj);
    } else if (atom->true_) {
        tcheck("call _trv_traverse with true_");
        object_t *obj = _trv_traverse(ast, atom->true_, dep+1);
        return_trav(obj);
    } else if (atom->digit) {
        tcheck("call _trv_traverse with digit");
        object_t *obj = _trv_traverse(ast, atom->digit, dep+1);
        return_trav(obj);
    } else if (atom->string) {
        tcheck("call _trv_traverse with string");
        object_t *obj = _trv_traverse(ast, atom->string, dep+1);
        return_trav(obj);
    } else if (atom->array) {
        tcheck("call _trv_traverse with array");
        object_t *obj = _trv_traverse(ast, atom->array, dep+1);
        return_trav(obj);
    } else if (atom->dict) {
        tcheck("call _trv_traverse with dict");
        object_t *obj = _trv_traverse(ast, atom->dict, dep+1);
        return_trav(obj);
    } else if (atom->identifier) {
        tcheck("call _trv_traverse with identifier");
        object_t *obj = _trv_traverse(ast, atom->identifier, dep+1);
        return_trav(obj);
    } else if (atom->caller) {
        tcheck("call _trv_traverse with caller");
        object_t *obj = _trv_traverse(ast, atom->caller, dep+1);
        return_trav(obj);
    }

    assert(0 && "impossible. invalid status of atom");
    return_trav(NULL);
}

static object_t *
trv_nil(ast_t *ast, const node_t *node, int dep) {
    tready();
    node_nil_t *nil = node->real;
    assert(nil && node->type == NODE_TYPE_NIL);
    // not check exists field
    return_trav(obj_new_nil());
}

static object_t *
trv_false(ast_t *ast, const node_t *node, int dep) {
    tready();
    node_false_t *false_ = node->real;
    assert(false_ && node->type == NODE_TYPE_FALSE);
    assert(!false_->boolean);
    return_trav(obj_new_false());
}

static object_t *
trv_true(ast_t *ast, const node_t *node, int dep) {
    tready();
    node_true_t *true_ = node->real;
    assert(true_ && node->type == NODE_TYPE_TRUE);
    assert(true_->boolean);
    return_trav(obj_new_true());
}

static object_t *
trv_digit(ast_t *ast, const node_t *node, int dep) {
    tready();
    node_digit_t *digit = node->real;
    assert(digit && node->type == NODE_TYPE_DIGIT);
    return_trav(obj_new_int(digit->lvalue));
}

static object_t *
trv_string(ast_t *ast, const node_t *node, int dep) {
    tready();
    node_string_t *string = node->real;
    assert(string && node->type == NODE_TYPE_STRING);
    return_trav(obj_new_cstr(string->string));
}

/**
 * left priority
 */
static object_t *
trv_array_elems(ast_t *ast, const node_t *node, int dep) {
    tready();
    node_array_elems_t *array_elems = node->real;
    assert(array_elems && node->type == NODE_TYPE_ARRAY_ELEMS);

    object_array_t *objarr = objarr_new();

    for (int32_t i = 0; i < nodearr_len(array_elems->nodearr); ++i) {
        node_t *n = nodearr_get(array_elems->nodearr, i);
        object_t *result = _trv_traverse(ast, n, dep+1);
        assert(result);
        objarr_moveb(objarr, result);
    }

    object_t *ret = obj_new_array(objarr);
    return_trav(ret); 
}

static object_t *
trv_array(ast_t *ast, const node_t *node, int dep) {
    tready();
    node_array_t_ *array = node->real;
    assert(array && node->type == NODE_TYPE_ARRAY);
    assert(array->array_elems);

    tcheck("call _trv_traverse with array elems");
    object_t *result = _trv_traverse(ast, array->array_elems, dep+1);
    return_trav(result);
}

static object_t *
trv_dict_elem(ast_t *ast, const node_t *node, int dep) {
    tready();
    node_dict_elem_t *dict_elem = node->real;
    assert(dict_elem && node->type == NODE_TYPE_DICT_ELEM);
    assert(dict_elem->key_simple_assign);
    assert(dict_elem->value_simple_assign);

    tcheck("call _trv_traverse with key simple assign");
    object_t *key = _trv_traverse(ast, dict_elem->key_simple_assign, dep+1);
    if (ast_has_error(ast)) {
        obj_del(key);
        return_trav(NULL);
    }
    assert(key);
    switch (key->type) {
    default:
        ast_set_error_detail(ast, "key is not string in dict elem");
        return_trav(NULL);
        break;
    case OBJ_TYPE_STRING:
    case OBJ_TYPE_IDENTIFIER:
        break;
    }

    object_t *val = _trv_traverse(ast, dict_elem->value_simple_assign, dep+1);
    if (ast_has_error(ast)) {
        obj_del(val);
        return_trav(NULL);
    }
    assert(val);
    
    object_array_t *objarr = objarr_new();

    objarr_moveb(objarr, key);
    objarr_moveb(objarr, val);

    object_t *obj = obj_new_array(objarr);
    return_trav(obj);
}

/**
 * left priority
 */
static object_t *
trv_dict_elems(ast_t *ast, const node_t *node, int dep) {
    tready();
    node_dict_elems_t *dict_elems = node->real;
    assert(dict_elems && node->type == NODE_TYPE_DICT_ELEMS);

    object_dict_t *objdict = objdict_new();

    for (int32_t i = 0; i < nodearr_len(dict_elems->nodearr); ++i) {
        node_t *dict_elem = nodearr_get(dict_elems->nodearr, i);
        tcheck("call _trv_traverse with dict_elem");
        object_t *arrobj = _trv_traverse(ast, dict_elem, dep+1);
        if (ast_has_error(ast)) {
            obj_del(arrobj);
            objdict_del(objdict);
            return_trav(NULL);
        }
        assert(arrobj);
        assert(arrobj->type == OBJ_TYPE_ARRAY);
        object_array_t *objarr = arrobj->objarr;
        assert(objarr_len(objarr) == 2);
        const object_t *key = objarr_getc(objarr, 0);
        const object_t *val = objarr_getc(objarr, 1);

        const char *skey = NULL;
        switch (key->type) {
        default:
            ast_set_error_detail(ast, "invalid key type");
            obj_del(arrobj);
            objdict_del(objdict);
            return_trav(NULL);
            break;
        case OBJ_TYPE_STRING:
            skey = str_getc(key->string);
            break;
        case OBJ_TYPE_IDENTIFIER: {
            const object_t *ref = pull_in_ref_by(ast, key);
            if (ref->type != OBJ_TYPE_STRING) {
                ast_set_error_detail(ast, "invalid key type in variable of dict");
                obj_del(arrobj);
                objdict_del(objdict);
                return_trav(NULL);
                break;   
            }
            skey = str_getc(ref->string);
        } break;
        }

        objdict_move(objdict, skey, obj_new_other(val));
        obj_del(arrobj);
}

    object_t *ret = obj_new_dict(objdict);
    return_trav(ret); 
}

static object_t *
trv_dict(ast_t *ast, const node_t *node, int dep) {
    tready();
    node_dict_t *dict = node->real;
    assert(dict && node->type == NODE_TYPE_DICT);
    assert(dict->dict_elems);

    tcheck("call _trv_traverse with dict");
    object_t *result = _trv_traverse(ast, dict->dict_elems, dep+1);
    return_trav(result);
}

static object_t *
trv_identifier(ast_t *ast, const node_t *node, int dep) {
    tready();
    node_identifier_t *identifier = node->real;
    assert(identifier && node->type == NODE_TYPE_IDENTIFIER);
    return_trav(obj_new_cidentifier(identifier->identifier));
}

static object_t *
trv_invoke_alias_set_func(ast_t *ast, const object_t *objargs) {
    if (!objargs) {
        ast_set_error_detail(ast, "can't invoke alias.set. need two arguments");
        return NULL;
    }
    assert(objargs->type == OBJ_TYPE_ARRAY);

    object_array_t *args = objargs->objarr;

    if (objarr_len(args) < 2) {
        ast_set_error_detail(ast, "can't invoke alias.set. too few arguments");
        return NULL;
    } else if (objarr_len(args) >= 4) {
        ast_set_error_detail(ast, "can't invoke alias.set. too many arguments");
        return NULL;
    }

    const object_t *keyobj = objarr_getc(args, 0);
    if (keyobj->type != OBJ_TYPE_STRING) {
        ast_set_error_detail(ast, "can't invoke alias.set. key is not string");
        return NULL;
    }

    const object_t *valobj = objarr_getc(args, 1);
    if (valobj->type != OBJ_TYPE_STRING) {
        ast_set_error_detail(ast, "can't invoke alias.set. value is not string");
        return NULL;
    }

    const object_t *descobj = NULL;
    if (objarr_len(args) == 3) {
        descobj = objarr_getc(args, 2);
        if (descobj->type != OBJ_TYPE_STRING) {
            ast_set_error_detail(ast, "can't invoke alias.set. description is not string");
            return NULL;
        }
    }

    const char *key = str_getc(keyobj->string);
    const char *val = str_getc(valobj->string);
    const char *desc = descobj ? str_getc(descobj->string) : NULL;

    ctx_set_alias(ast->context, key, val, desc);

    return obj_new_nil();
}

static object_t *
trv_invoke_opts_get_func(ast_t *ast, const object_t *objargs) {
    if (!objargs) {
        ast_set_error_detail(ast, "can't invoke opts.get. need one argument");
        return NULL;
    }

    if (objargs->type == OBJ_TYPE_ARRAY) {
        if (objarr_len(objargs->objarr) != 1) {
            ast_set_error_detail(ast, "can't invoke opts.get. need one argument");
            return NULL;
        }

        const object_t *objname = objarr_getc(objargs->objarr, 0);
        assert(objname);

        if (objname->type != OBJ_TYPE_STRING) {
            ast_set_error_detail(ast, "can't invoke opts.get. argument is not string");
            return NULL;
        }

        string_t *optname = objname->string;
        const char *optval = opts_getc(ast->opts, str_getc(optname));
        if (!optval) {
            return obj_new_nil();
        }        

        return obj_new_cstr(optval);
    } else if (objargs->type == OBJ_TYPE_STRING) {
        string_t *optname = objargs->string;
        const char *optval = opts_getc(ast->opts, str_getc(optname));
        if (!optval) {
            return obj_new_nil();
        }        

        return obj_new_cstr(optval);
    } 

    assert(0 && "impossible. invalid arguments");
    return NULL;
}

static object_t *
trv_invoke_func_obj(ast_t *ast, const char *name, const object_t *drtargs, int dep) {
    tready();
    assert(name);
    tcheck("invoke func obj");
    object_t *args = NULL;
    if (drtargs) {
        args = obj_to_array(drtargs);
    }

    object_t *func_obj = get_var_ref(ast, name, 0);
    if (!func_obj) {
        ast_set_error_detail(ast, "\"%s\" is not defined", name);
        obj_del(args);
        return NULL;
    }

    if (func_obj->type != OBJ_TYPE_FUNC) {
        ast_set_error_detail(ast, "\"%s\" is not callable", name);
        obj_del(args);
        return NULL;
    }

    object_func_t *func = &func_obj->func;
    assert(func->args->type == OBJ_TYPE_ARRAY);

    ctx_pushb_scope(ast->context);
    if (args) {
        const object_array_t *formal_args = func->args->objarr;
        const object_array_t *actual_args = args->objarr;

        if (objarr_len(formal_args) != objarr_len(actual_args)) {
            ast_set_error_detail(ast, "arguments not same length");
            obj_del(args);
            ctx_popb_scope(ast->context);
            return NULL;
        }

        for (int32_t i = 0; i < objarr_len(formal_args); ++i) {
            const object_t *farg = objarr_getc(formal_args, i);
            assert(farg->type == OBJ_TYPE_IDENTIFIER);
            const char *fargname = str_getc(farg->identifier);

            object_t *aarg = objarr_get(actual_args, i);
            object_t *ref_aarg = aarg;
            if (aarg->type == OBJ_TYPE_IDENTIFIER) {
                ref_aarg = pull_in_ref_by(ast, aarg);
                if (!ref_aarg) {
                    ast_set_error_detail(ast, "\"%s\" is not defined in invoke function", str_getc(aarg->identifier));
                    obj_del(args);
                    return NULL;
                }
            }
            object_t *copy_aarg = obj_new_other(ref_aarg);

            move_var(ast, fargname, copy_aarg, dep+1);
        }
    }

    obj_del(args);

    tcheck("call _trv_traverse with ref_suite");
    object_t *result = _trv_traverse(ast, func->ref_suite, dep+1);
    ctx_set_do_return(ast->context, false);
    ctx_popb_scope(ast->context);
    return result;
}

static string_t *
trv_obj_to_str(ast_t *ast, const object_t *obj) {
    assert(obj);

    switch (obj->type) {
    default: return obj_to_str(obj); break;
    case OBJ_TYPE_IDENTIFIER: {
        object_t *var = pull_in_ref_by(ast, obj);
        if (!var) {
            ast_set_error_detail(ast, "\"%s\" is not defined in object to string", str_getc(obj->identifier));
            return NULL;
        }
        return obj_to_str(var);
    } break;
    }

    assert(0 && "impossible. failed to ast obj to str");
    return NULL;
}

static object_t *
trv_invoke_puts_func(ast_t *ast, const object_t *drtargs) {
    if (!drtargs) {
        ctx_pushb_buf(ast->context, "\n");
        return obj_new_int(0);
    }

    object_t *args = obj_to_array(drtargs);

    int32_t arrlen = objarr_len(args->objarr);

    for (int32_t i = 0; i < arrlen-1; ++i) {
        object_t *obj = objarr_get(args->objarr, i);
        assert(obj);
        object_t *copy = trv_copy_object_value(ast, obj);
        string_t *s = trv_obj_to_str(ast, copy);
        obj_del(copy);
        if (!s) {
            continue;
        }
        str_pushb(s, ' ');
        ctx_pushb_buf(ast->context, str_getc(s));
        str_del(s);
    }
    if (arrlen) {
        object_t *obj = objarr_get(args->objarr, arrlen-1);
        assert(obj);
        object_t *copy = trv_copy_object_value(ast, obj);
        string_t *s = trv_obj_to_str(ast, copy);
        obj_del(copy);
        if (!s) {
            goto done;
        }
        ctx_pushb_buf(ast->context, str_getc(s));
        str_del(s);
    }

done:
    ctx_pushb_buf(ast->context, "\n");
    return obj_new_int(arrlen);
}

static object_t *
trv_invoke_lower_func(ast_t *ast, const object_t *_) {
    const object_t *owner = ast->dot_ref_owner;
    if (!owner) {
        return obj_new_nil();
    }

    switch (owner->type) {
    default:
        return obj_new_nil();
        break;
    case OBJ_TYPE_STRING: {
        string_t *str = str_lower(owner->string);
        return obj_new_str(str);
    } break;
    }

    assert(0 && "impossible. failed to invoke lower func");
    return obj_new_nil();
}

static object_t *
trv_invoke_upper_func(ast_t *ast, const object_t *_) {
    const object_t *owner = ast->dot_ref_owner;
    if (!owner) {
        return obj_new_nil();
    }

    switch (owner->type) {
    default:
        return obj_new_nil();
        break;
    case OBJ_TYPE_STRING: {
        string_t *str = str_upper(owner->string);
        return obj_new_str(str);
    } break;
    }

    assert(0 && "impossible. failed to invoke upper func");
    return obj_new_nil();
}

static object_t *
trv_caller(ast_t *ast, const node_t *node, int dep) {
    tready();
    node_caller_t *caller = node->real;
    assert(caller && node->type == NODE_TYPE_CALLER);

    node_identifier_t *identifier = caller->identifier->real;
    const char *name = identifier->identifier;

    tcheck("call _trv_traverse");
    object_t *args = _trv_traverse(ast, caller->test_list, dep+1);
    object_t *result = NULL;

/*
    if (cstrarr_len(names) == 2 &&
        cstr_eq(cstrarr_getc(names, 0), "alias") &&
        cstr_eq(cstrarr_getc(names, 1), "set")) {
        tcheck("call trv_invoke_alias_set_func");
        result = trv_invoke_alias_set_func(ast, args);
        if (ast_has_error(ast)) {
            obj_del(args);
            return_trav(NULL);
        }
    } else if (cstrarr_len(names) == 2 &&
        cstr_eq(cstrarr_getc(names, 0), "opts") &&
        cstr_eq(cstrarr_getc(names, 1), "get")) {
        tcheck("call trv_invoke_opts_get_func");
        result = trv_invoke_opts_get_func(ast, args);
        if (ast_has_error(ast)) {
            obj_del(args);
            return_trav(NULL);
        }
    } else if (cstrarr_len(names) == 1 &&
        cstr_eq(cstrarr_getc(names, 0), "puts")) {
        result = trv_invoke_puts_func(ast, args);
        if (ast_has_error(ast)) {
            obj_del(args);
            return_trav(NULL);
        }        
    } else if (cstrarr_len(names) == 1) {
        const char *name = cstrarr_getc(names, 0);
        result = trv_invoke_func_obj(ast, name, args, dep+1);
        if (ast_has_error(ast)) {
            obj_del(args);
            return_trav(NULL);
        }
    } else {
        string_t *s = str_new();
        for (int i = 0; i < cstrarr_len(names); ++i) {
            str_app(s, cstrarr_getc(names, i));
            str_pushb(s, '.');
        }
        str_popb(s);
        ast_set_error_detail(ast, "\"%s\" is not callable", str_getc(s));
        str_del(s);
        obj_del(args);
        return_trav(NULL);
    }
*/
    if (cstr_eq(name, "puts")) {
        result = trv_invoke_puts_func(ast, args);
        if (ast_has_error(ast)) {
            obj_del(args);
            return_trav(NULL);
        }        
    } else if (cstr_eq(name, "lower")) {
        result = trv_invoke_lower_func(ast, args);
        if (ast_has_error(ast)) {
            obj_del(args);
            return_trav(NULL);
        }
    } else if (cstr_eq(name, "upper")) {
        result = trv_invoke_upper_func(ast, args);
        if (ast_has_error(ast)) {
            obj_del(args);
            return_trav(NULL);
        }
    } else {
        result = trv_invoke_func_obj(ast, name, args, dep+1);
        if (ast_has_error(ast)) {
            obj_del(args);
            return_trav(NULL);
        }
    }

    obj_del(args);
    if (!result) {
        return_trav(obj_new_nil());
    }
    return_trav(result);
}

static object_t *
trv_def(ast_t *ast, const node_t *node, int dep) {
    tready();
    node_def_t *def = node->real;
    assert(def && node->type == NODE_TYPE_DEF);

    tcheck("call _trv_traverse with func_def")
    object_t *result = _trv_traverse(ast, def->func_def, dep+1);
    return_trav(result);
}

static object_t *
trv_func_def(ast_t *ast, const node_t *node, int dep) {
    tready();
    node_func_def_t *func_def = node->real;
    assert(func_def && node->type == NODE_TYPE_FUNC_DEF);

    tcheck("call _trv_traverse with identifier");
    object_t *name = _trv_traverse(ast, func_def->identifier, dep+1);
    if (!name) {
        if (ast_has_error(ast)) {
            return_trav(NULL);
        }
        ast_set_error_detail(ast, "failed to traverse name in traverse func def");
        return_trav(NULL);
    }
    assert(name->type == OBJ_TYPE_IDENTIFIER);

    object_t *def_args = _trv_traverse(ast, func_def->func_def_params, dep+1);
    assert(def_args);
    assert(def_args->type == OBJ_TYPE_ARRAY);

    node_t *ref_suite = NULL;
    if (func_def->elems) {
        ref_suite = func_def->elems;
    } else if (func_def->blocks) {
        ref_suite = func_def->blocks;
    }

    object_t *func_obj = obj_new_func(name, def_args, ref_suite);
    tcheck("set func at varmap");
    move_var(ast, str_getc(name->identifier), func_obj, dep+1);

    return_trav(NULL);
}

static object_t *
trv_func_def_params(ast_t *ast, const node_t *node, int dep) {
    tready();
    node_func_def_params_t *func_def_params = node->real;
    assert(func_def_params && node->type == NODE_TYPE_FUNC_DEF_PARAMS);

    tcheck("call _trv_traverse with func_def_args");
    return _trv_traverse(ast, func_def_params->func_def_args, dep+1);
}

static object_t *
trv_func_def_args(ast_t *ast, const node_t *node, int dep) {
    tready();
    node_func_def_args_t *func_def_args = node->real;
    assert(func_def_args && node->type == NODE_TYPE_FUNC_DEF_ARGS);

    object_array_t *args = objarr_new();

    for (int32_t i = 0; i < nodearr_len(func_def_args->identifiers); ++i) {
        node_t *n = nodearr_get(func_def_args->identifiers, i);
        assert(n);
        assert(n->type == NODE_TYPE_IDENTIFIER);
        node_identifier_t *nidn = n->real;

        object_t *oidn = obj_new_cidentifier(nidn->identifier);
        objarr_moveb(args, oidn);
    }

    return obj_new_array(args);
}

static object_t *
_trv_traverse(ast_t *ast, const node_t *node, int dep) {
    tready();
    if (!node) {
        return_trav(NULL); 
    }

    switch (node->type) {
    default: {
        err_die("impossible. unsupported node type %d in traverse", node_getc_type(node));
    } break;
    case NODE_TYPE_PROGRAM: {
        tcheck("call trv_program");
        object_t *obj = trv_program(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_BLOCKS: {
        tcheck("call trv_blocks");
        object_t *obj = trv_blocks(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_CODE_BLOCK: {
        tcheck("call trv_code_block");
        object_t *obj = trv_code_block(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_REF_BLOCK: {
        tcheck("call trv_ref_block");
        object_t *obj = trv_ref_block(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_TEXT_BLOCK: {
        tcheck("call trv_text_block");
        object_t *obj = trv_text_block(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_ELEMS: {
        tcheck("call trv_elems");
        object_t *obj = trv_elems(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_FORMULA: {
        tcheck("call trv_formula");
        object_t *obj = trv_formula(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_ASSIGN_LIST: {
        tcheck("call trv_assign_list");
        object_t *obj = trv_assign_list(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_ASSIGN: {
        tcheck("call trv_assign");
        object_t *obj = trv_assign(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_SIMPLE_ASSIGN: {
        tcheck("call trv_simple_assign");
        object_t *obj = trv_simple_assign(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_MULTI_ASSIGN: {
        tcheck("call trv_multi_assign");
        object_t *obj = trv_multi_assign(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_DEF: {
        tcheck("call trv_def");
        object_t *obj = trv_def(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_FUNC_DEF: {
        tcheck("call trv_func_def");
        object_t *obj = trv_func_def(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_FUNC_DEF_PARAMS: {
        tcheck("call trv_func_def_params");
        object_t *obj = trv_func_def_params(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_FUNC_DEF_ARGS: {
        tcheck("call trv_func_def_args");
        object_t *obj = trv_func_def_args(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_STMT: {
        tcheck("call trv_stmt");
        object_t *obj = trv_stmt(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_IMPORT_STMT: {
        tcheck("call trv_import_stmt");
        object_t *obj = trv_import_stmt(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_IF_STMT: {
        tcheck("call trv_if_stmt");
        object_t *obj = trv_if_stmt(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_ELIF_STMT: {
        tcheck("call trv_if_stmt");
        object_t *obj = trv_if_stmt(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_ELSE_STMT: {
        tcheck("call trv_else_stmt");
        object_t *obj = trv_else_stmt(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_FOR_STMT: {
        tcheck("call trv_for_stmt");
        object_t *obj = trv_for_stmt(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_BREAK_STMT: {
        tcheck("call trv_break_stmt");
        object_t *obj = trv_break_stmt(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_CONTINUE_STMT: {
        tcheck("call trv_continue_stmt");
        object_t *obj = trv_continue_stmt(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_RETURN_STMT: {
        tcheck("call trv_return_stmt");
        object_t *obj = trv_return_stmt(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_TEST_LIST: {
        tcheck("call trv_test_list");
        object_t *obj = trv_test_list(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_TEST: {
        tcheck("call trv_test");
        object_t *obj = trv_test(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_OR_TEST: {
        tcheck("call trv_or_test");
        object_t *obj = trv_or_test(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_AND_TEST: {
        tcheck("call trv_and_test");
        object_t *obj = trv_and_test(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_NOT_TEST: {
        tcheck("call trv_not_test");
        object_t *obj = trv_not_test(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_COMPARISON: {
        tcheck("call trv_comparison");
        object_t *obj = trv_comparison(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_EXPR: {
        tcheck("call trv_expr");
        object_t *obj = trv_expr(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_TERM: {
        tcheck("call trv_term");
        object_t *obj = trv_term(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_DOT: {
        tcheck("call trv_dot");
        object_t *obj = trv_dot(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_INDEX: {
        tcheck("call trv_index");
        object_t *obj = trv_index(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_ASSCALC: {
        tcheck("call trv_asscalc");
        object_t *obj = trv_asscalc(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_FACTOR: {
        tcheck("call trv_factor");
        object_t *obj = trv_factor(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_ATOM: {
        tcheck("call trv_atom");
        object_t *obj = trv_atom(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_NIL: {
        tcheck("call trv_nil");
        object_t *obj = trv_nil(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_FALSE: {
        tcheck("call trv_false");
        object_t *obj = trv_false(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_TRUE: {
        tcheck("call trv_true");
        object_t *obj = trv_true(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_DIGIT: {
        tcheck("call trv_digit");
        object_t *obj = trv_digit(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_STRING: {
        tcheck("call trv_string");
        object_t *obj = trv_string(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_ARRAY: {
        tcheck("call trv_array");
        object_t *obj = trv_array(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_ARRAY_ELEMS: {
        tcheck("call trv_array_elems");
        object_t *obj = trv_array_elems(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_DICT: {
        tcheck("call trv_dict");
        object_t *obj = trv_dict(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_DICT_ELEMS: {
        tcheck("call trv_dict_elems");
        object_t *obj = trv_dict_elems(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_DICT_ELEM: {
        tcheck("call trv_dict_elem");
        object_t *obj = trv_dict_elem(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_IDENTIFIER: {
        tcheck("call trv_identifier");
        object_t *obj = trv_identifier(ast, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_CALLER: {
        tcheck("call trv_caller");
        object_t *obj = trv_caller(ast, node, dep+1);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to traverse");
    return_trav(NULL);
}

void
trv_traverse(ast_t *ast, context_t *context) {
    ast->context = context;
    ctx_clear(ast->context);
    _trv_traverse(ast, ast->root, 0);
}

#undef tready
#undef return_trav
#undef tcheck
#undef viss
#undef vissf