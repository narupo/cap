#include "lang/ast.h"

enum {
    ERR_DETAIL_SIZE = 1024,
};

struct ast {
    token_t **tokens; // token list with null at the last
    token_t **ptr; // pointer to tokens
    node_t *root; // pointer to root
    context_t *context; // context. update when traverse tree
    char error_detail[ERR_DETAIL_SIZE]; // error detail
    bool debug; // if do debug to true
};

/*********
* macros *
*********/

#define declare(T, var) \
    T* var = calloc(1, sizeof(T)); \
    if (!var) { \
        err_die("failed to alloc. LINE %d", __LINE__); \
    } \

#define ready() \
    if (self->debug) { \
        token_t *t = *self->ptr; \
        fprintf(stderr, "debug: %d: %*s: %3d: %s: %s\n", __LINE__, 20, __func__, dep, token_type_to_str(t), ast_get_error_detail(self)); \
        fflush(stderr); \
    } \
    ast_skip_newlines(self); \
    if (!*self->ptr) { \
        return NULL; \
    } \

#define return_this(ret) \
    if (self->debug) { \
        fprintf(stderr, "debug: %d: %*s: %3d: return %p: %s\n", __LINE__, 20, __func__, dep, ret, ast_get_error_detail(self)); \
        fflush(stderr); \
    } \
    return ret; \

#define check(msg) \
    if (self->debug) { \
        fprintf(stderr, "debug: %d: %*s: %3d: %s: %s: %s\n", __LINE__, 20, __func__, dep, msg, token_type_to_str(*self->ptr), ast_get_error_detail(self)); \
    } \

#define vissf(fmt, ...) \
    if (self->debug) fprintf(stderr, "vissf: %d: " fmt "\n", __LINE__, __VA_ARGS__); \

#define viss(fmt) \
    if (self->debug) fprintf(stderr, "viss: %d: " fmt "\n", __LINE__); \

/*************
* prototypes *
*************/

static node_t *
ast_elems(ast_t *self, int dep);

static node_t *
ast_blocks(ast_t *self, int dep);

static node_t *
ast_test(ast_t *self, int dep);

static node_t *
ast_test_list(ast_t *self, int dep);

/************
* functions *
************/

void
ast_del_nodes(const ast_t *self, node_t *node) {
    if (node == NULL) {
        return;
    }

    switch (node->type) {
    default: {
        // err_die("impossible. not supported node type '%d'", node->type);
    } break;
    }
}

void
ast_del(ast_t *self) {
    if (self == NULL) {
        return;
    }
    ast_del_nodes(self, self->root);
    free(self);
}

ast_t *
ast_new(void) {
    ast_t *self = mem_ecalloc(1, sizeof(*self));
    return self;
}

const node_t *
ast_getc_root(const ast_t *self) {
    return self->root;
}

static void
ast_set_error_detail(ast_t *self, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(self->error_detail, sizeof self->error_detail, fmt, ap);
    va_end(ap);
}

static void
ast_show_debug(const ast_t *self, const char *funcname) {
    if (self->debug) {
        token_t *t = *self->ptr;
        printf("debug: %s: token type[%d]\n", funcname, (t ? t->type : -1));
    }
}

static void
ast_skip_newlines(ast_t *self) {
    for (; *self->ptr; ) {
        token_t *t = *self->ptr++;
        if (t->type != TOKEN_TYPE_NEWLINE) {
            self->ptr--;
            return;
        }
    }
}

static node_t *
ast_assign_list(ast_t *self, int dep) {
    ready();
    declare(node_assign_list_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        ast_del_nodes(self, cur->test_list); \
        ast_del_nodes(self, cur->assign_list); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_this(NULL); \
    } \

    check("call ast_test_list");
    cur->test_list = ast_test_list(self, dep+1);
    if (!cur->test_list) {
        return_cleanup("");
    }

    if (!*self->ptr) {
        return_this(node_new(NODE_TYPE_ASSIGN_LIST, cur));
    }

    token_t *t = *self->ptr++;
    if (t->type != TOKEN_TYPE_OP_ASS) {
        self->ptr--;
        return_this(node_new(NODE_TYPE_ASSIGN_LIST, cur));
    }
    check("read =");

    check("call ast_assign_list");
    cur->assign_list = ast_assign_list(self, dep+1);
    if (!cur->assign_list) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }
        return_this(node_new(NODE_TYPE_ASSIGN_LIST, cur));
    }

    return_this(node_new(NODE_TYPE_ASSIGN_LIST, cur));
}

static node_t *
ast_formula(ast_t *self, int dep) {
    ready();
    declare(node_formula_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        ast_del_nodes(self, cur->assign_list); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_this(NULL); \
    } \

    check("call ast_assign_list");
    cur->assign_list = ast_assign_list(self, dep+1);
    if (!cur->assign_list) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    return_this(node_new(NODE_TYPE_FORMULA, cur));
}

static node_t *
ast_test_list(ast_t *self, int dep) {
    ready();
    declare(node_test_list_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        ast_del_nodes(self, cur->test); \
        ast_del_nodes(self, cur->test_list); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_this(NULL); \
    } \

    cur->test = ast_test(self, dep+1);
    if (!cur->test) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    token_t *t = *self->ptr++;
    if (t->type != TOKEN_TYPE_COMMA) {
        self->ptr--;
        return node_new(NODE_TYPE_TEST_LIST, cur);
    }

    cur->test_list = ast_test_list(self, dep+1);
    if (!cur->test_list) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }
        return_cleanup("syntax error. not found test list in test list");
    }

    return node_new(NODE_TYPE_TEST_LIST, cur);
}

static node_t *
ast_for_stmt(ast_t *self, int dep) {
    ready();
    declare(node_for_stmt_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        ast_del_nodes(self, cur->init_test_list); \
        ast_del_nodes(self, cur->test); \
        ast_del_nodes(self, cur->update_test_list); \
        ast_del_nodes(self, cur->elems); \
        ast_del_nodes(self, cur->blocks); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_this(NULL); \
    } \

    token_t *t = *self->ptr++;
    if (t->type != TOKEN_TYPE_STMT_FOR) {
        return_cleanup("");
    }
    check("read for");

    if (!*self->ptr) {
        return_cleanup("syntax error. reached EOF in for statement");
    }

    t = *self->ptr++;
    if (t->type == TOKEN_TYPE_COLON) {
        check("read colon");

        if (!*self->ptr) {
            return_cleanup("syntax error. reached EOF in for statement (2)");
        }

        t = *self->ptr++;
        if (t->type == TOKEN_TYPE_RBRACEAT) {
            cur->blocks = ast_blocks(self, dep+1);
            // allow null
            if (ast_has_error(self)) {
                return_cleanup("");
            }

            if (!*self->ptr) {
                return_cleanup("syntax error. reached EOF in for statement (2a)");
            }

            t = *self->ptr++;
            if (t->type != TOKEN_TYPE_LBRACEAT) {
                return_cleanup("syntax error. not found {@ in for statement");
            }

        } else {
            self->ptr--;

            // for : <elems> end
            check("call ast_elems");
            cur->elems = ast_elems(self, dep+1);
            // allow null
            if (ast_has_error(self)) {
                return_cleanup("");
            }            
        }

        if (!*self->ptr) {
            return_cleanup("syntax error. reached EOF in for statement (3)");
        }

        t = *self->ptr++;
        if (t->type != TOKEN_TYPE_STMT_END) {
            return_cleanup("syntax error. not found end in for statement");
        }
        check("read end");
    } else {
        self->ptr--;
        check("call ast_test_list");

        check("call ast_test");
        cur->test = ast_test(self, dep+1);
        if (ast_has_error(self)) {
            return_cleanup("");
        }

        if (!cur->test) {
            check("call ast_test_list");
            cur->init_test_list = ast_test_list(self, dep+1);
            if (!cur->init_test_list) {
                if (ast_has_error(self)) {
                    return_cleanup("");
                }
                return_cleanup("syntax error. not found initialize test in for statement");            
            }
        }

        t = *self->ptr++;
        if (t->type == TOKEN_TYPE_SEMICOLON) {
            check("read semicolon");
            // for <test_list> ; test ; test_list : elems end

            if (cur->test) {
                // test move to init_test_list
                ast_del_nodes(self, cur->init_test_list);
                declare(node_test_list_t, test_list);
                test_list->test = cur->test;
                cur->test = NULL;
                cur->init_test_list = node_new(NODE_TYPE_TEST_LIST, test_list);
            }

            if (!cur->init_test_list) {
                return_cleanup("syntax error. not found initialize test in for statement (2)");
            }

            check("call ast_test");
            cur->test = ast_test(self, dep+1);
            // allow empty
            if (ast_has_error(self)) {
                return_cleanup("");
            }

            if (!*self->ptr) {
                return_cleanup("syntax error. reached EOF in for statement (4)");
            }

            t = *self->ptr++;
            if (t->type != TOKEN_TYPE_SEMICOLON) {
                return_cleanup("syntax error. not found semicolon (2)");
            }
            check("read semicolon");

            check("call ast_test_list");
            cur->update_test_list = ast_test_list(self, dep+1);
            // allow empty
            if (ast_has_error(self)) {
                return_cleanup("");
            }

            if (!*self->ptr) {
                return_cleanup("syntax error. reached EOF in for statement (5)");
            }
        } else if (t->type == TOKEN_TYPE_COLON) {
            self->ptr--;
            // for <test> : elems end
            // pass
        } else {
            return_cleanup("syntax error. unsupported character in for statement");
        }

        t = *self->ptr++;
        if (t->type != TOKEN_TYPE_COLON) {
            return_cleanup("syntax error. not found colon in for statement")
        }
        check("read colon");

        if (!*self->ptr) {
            return_cleanup("syntax error. reached EOF in for statement (6)")
        }

        t = *self->ptr++;
        if (t->type == TOKEN_TYPE_RBRACEAT) {
            check("read @}");
            cur->blocks = ast_blocks(self, dep+1);
            // allow null
            if (ast_has_error(self)) {
                return_cleanup("");
            }

            if (!*self->ptr) {
                return_cleanup("syntax error. reached EOF in for statement (6)");
            }

            t = *self->ptr++;
            if (t->type != TOKEN_TYPE_LBRACEAT) {
                return_cleanup("syntax error. not found {@ in for statement");
            }
        } else {
            self->ptr--;
        }

        check("call ast_elems");
        cur->elems = ast_elems(self, dep+1);
        // allow empty
        if (ast_has_error(self)) {
            return_cleanup("");
        }

        if (!*self->ptr) {
            return_cleanup("syntax error. reached EOF in for statement (5)");
        }

        t = *self->ptr++;
        if (t->type != TOKEN_TYPE_STMT_END) {
            return_cleanup("syntax error. not found end in for statement");
        }
        check("read end");
    }

    return_this(node_new(NODE_TYPE_FOR_STMT, cur));
}

static node_t *
ast_comparison(ast_t *self, int dep) {
    ready();
    declare(node_comparison_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        ast_del_nodes(self, cur->expr); \
        ast_del_nodes(self, cur->comp_op); \
        ast_del_nodes(self, cur->comparison); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_this(NULL); \
    } \

    token_t *t = *self->ptr++;
    if (t->type != TOKEN_TYPE_INTEGER) {
        return_cleanup("");
    }
    check("read integer");

    return_this(node_new(NODE_TYPE_COMPARISON, cur));
}

static node_t *
ast_not_test(ast_t *self, int dep) {
    ready();
    declare(node_not_test_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        ast_del_nodes(self, cur->not_test); \
        ast_del_nodes(self, cur->comparison); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_this(NULL); \
    } \

    token_t *t = *self->ptr++;
    if (t->type == TOKEN_TYPE_OP_NOT) {
        check("call ast_not_test");
        cur->not_test = ast_not_test(self, dep+1);
        if (!cur->not_test) {
            if (ast_has_error(self)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found operand in not operator");
        }
    } else {
        self->ptr--;

        check("call ast_comparison");
        cur->comparison = ast_comparison(self, dep+1);
        if (!cur->comparison) {
            if (ast_has_error(self)) {
                return_cleanup("");
            }
            return_cleanup(""); // not error
        }
    }

    return_this(node_new(NODE_TYPE_NOT_TEST, cur));
}

static node_t *
ast_and_test(ast_t *self, int dep) {
    ready();
    declare(node_and_test_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        ast_del_nodes(self, cur->not_test); \
        ast_del_nodes(self, cur->and_test); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_this(NULL); \
    } \

    check("call ast_not_test");
    cur->not_test = ast_not_test(self, dep+1);
    if (!cur->not_test) {
        return_cleanup("");
    }

    if (!*self->ptr) {
        return_this(node_new(NODE_TYPE_AND_TEST, cur));
    }

    token_t *t = *self->ptr++;
    if (t->type != TOKEN_TYPE_OP_AND) {
        self->ptr--;
        return_this(node_new(NODE_TYPE_AND_TEST, cur));
    }
    check("read 'and'");

    check("call ast_and_test");
    cur->and_test = ast_and_test(self, dep+1);
    if (!cur->and_test) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }
        return_cleanup("syntax error. not found rhs operand in 'and' operator");
    }

    return_this(node_new(NODE_TYPE_AND_TEST, cur));
}

static node_t *
ast_or_test(ast_t *self, int dep) {
    ready();
    declare(node_or_test_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        ast_del_nodes(self, cur->and_test); \
        ast_del_nodes(self, cur->or_test); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_this(NULL); \
    } \

    check("call ast_and_test");
    cur->and_test = ast_and_test(self, dep+1);
    if (!cur->and_test) {
        return_cleanup("");
    }

    if (!*self->ptr) {
        return_this(node_new(NODE_TYPE_OR_TEST, cur));
    }

    token_t *t = *self->ptr++;
    if (t->type != TOKEN_TYPE_OP_OR) {
        self->ptr--;
        return_this(node_new(NODE_TYPE_OR_TEST, cur));
    }
    check("read 'or'")

    check("call ast_or_test");
    cur->or_test = ast_or_test(self, dep+1);
    if (!*self->ptr) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }
        return_cleanup("syntax error. not found rhs operand in 'or' operator");        
    }

    return_this(node_new(NODE_TYPE_OR_TEST, cur));
}

static node_t *
ast_test(ast_t *self, int dep) {
    ready();
    declare(node_test_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        ast_del_nodes(self, cur->or_test); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_this(NULL); \
    } \

    cur->or_test = ast_or_test(self, dep+1);
    if (!cur->or_test) {
        return_cleanup("");
    }

    return_this(node_new(NODE_TYPE_TEST, cur));
}

static node_t *
ast_else_stmt(ast_t *self, int dep) {
    ready();
    declare(node_else_stmt_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        ast_del_nodes(self, cur->elems); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_this(NULL); \
    } \

    token_t *t = *self->ptr++;
    if (t->type != TOKEN_TYPE_STMT_ELSE) {
        return_cleanup("");
    }

    if (!*self->ptr) {
        return_cleanup("syntax error. reached EOF in else statement");
    }

    t = *self->ptr++;
    if (t->type != TOKEN_TYPE_COLON) {
        return_cleanup("syntax error. not found colon in else statement");
    }

    ast_skip_newlines(self);

    check("call ast_elems");
    cur->elems = ast_elems(self, dep+1);
    if (!cur->elems) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }
    }

    if (!*self->ptr) {
        return_cleanup("syntax error. reached EOF in else statement (2)");
    }

    t = *self->ptr++;
    if (t->type != TOKEN_TYPE_STMT_END) {
        return_cleanup("syntax error. not found end in else statement");
    }

    return_this(node_new(NODE_TYPE_ELSE_STMT, cur));
}

static node_t *
ast_if_stmt(ast_t *self, int type, int dep) {
    ready();
    declare(node_if_stmt_t, cur);
    token_t **save_ptr = self->ptr;
    node_type_t node_type = NODE_TYPE_IF_STMT;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        ast_del_nodes(self, cur->test); \
        ast_del_nodes(self, cur->elems); \
        ast_del_nodes(self, cur->blocks); \
        ast_del_nodes(self, cur->elif_stmt); \
        ast_del_nodes(self, cur->else_stmt); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_this(NULL); \
    } \

    token_t *t = *self->ptr++;
    if (type == 0) {
        if (t->type != TOKEN_TYPE_STMT_IF) {
            return_cleanup("");
        }
        node_type = NODE_TYPE_IF_STMT;
        check("read if");
    } else if (type == 1) {
        if (t->type != TOKEN_TYPE_STMT_ELIF) {
            return_cleanup("");
        }
        node_type = NODE_TYPE_ELIF_STMT;
        check("read elif");
    } else {
        err_die("invalid type in if stmt");
    }

    check("call ast_test");
    cur->test = ast_test(self, dep+1);
    if (!cur->test) {
        self->ptr = save_ptr;
        if (ast_has_error(self)) {
            return_cleanup("");
        }

        return_cleanup("syntax error. not found test in if statement");
    }

    if (!*self->ptr) {
        return_cleanup("syntax error. reached EOF in if statement");
    }

    t = *self->ptr++;
    if (t->type != TOKEN_TYPE_COLON) {
        return_cleanup("syntax error. not found colon in if statement");
    }
    check("read colon");

    ast_skip_newlines(self);

    if (!*self->ptr) {
        return_cleanup("syntax error. reached EOF in if statement (2)");
    }

    t = *self->ptr++;
    if (t->type == TOKEN_TYPE_RBRACEAT) {
        check("read @}");

        check("call ast_blocks");
        cur->blocks = ast_blocks(self, dep+1);
        check("ABABABA");
        // block allow null
        if (ast_has_error(self)) {
            return_cleanup("");
        }

        if (!*self->ptr) {
            return_cleanup("syntax error. reached EOF in if statement (3)");
        }

        t = *self->ptr++;
        if (t->type != TOKEN_TYPE_LBRACEAT) {
            return_cleanup("syntax error. not found \"{@\" in if statement");
        }

        check("call ast_elif_stmt");
        cur->elif_stmt = ast_if_stmt(self, 1, dep+1);
        if (!cur->elif_stmt) {
            if (ast_has_error(self)) {
                return_cleanup("");
            }

            check("call ast_else_stmt");
            cur->else_stmt = ast_else_stmt(self, dep+1);
            if (!cur->else_stmt) {
                if (ast_has_error(self)) {
                    return_cleanup("");
                }

                if (!*self->ptr) {
                    return_cleanup("syntax error. reached EOF in if statement (4)");
                }

                t = *self->ptr++;
                if (t->type != TOKEN_TYPE_STMT_END) {
                    return_cleanup("syntax error. not found end in if statement");
                }
            }
        }
    } else {
        self->ptr--;

        // elems allow null
        check("call ast_elems");
        cur->elems = ast_elems(self, dep+1);
        if (ast_has_error(self)) {
            return_cleanup("");
        }

        check("call ast_if_stmt");
        cur->elif_stmt = ast_if_stmt(self, 1, dep+1);
        if (!cur->elif_stmt) {
            if (ast_has_error(self)) {
                return_cleanup("");
            }

            check("call ast_else_stmt");
            cur->else_stmt = ast_else_stmt(self, dep+1);
            if (!cur->else_stmt) {
                if (ast_has_error(self)) {
                    return_cleanup("");
                }

                if (!*self->ptr) {
                    return_cleanup("syntax error. reached EOF in if statement (4)");
                }

                t = *self->ptr++;
                if (t->type != TOKEN_TYPE_STMT_END) {
                    return_cleanup("syntax error. not found end in if statement (2)");
                }
            }
        }
    }

    return_this(node_new(node_type, cur));
}

static node_t *
ast_identifier(ast_t *self, int dep) {
    ready();
    declare(node_identifier_t, cur);
    token_t **save_ptr = self->ptr;

    token_t *t = *self->ptr++;
    if (t->type != TOKEN_TYPE_IDENTIFIER) {
        self->ptr = save_ptr;
        free(cur);
        return_this(NULL);
    }

    // move text
    cur->identifier = t->text;
    t->text = NULL;

    return_this(node_new(NODE_TYPE_IDENTIFIER, cur));
}

static node_t *
ast_identifier_chain(ast_t *self, int dep) {
    ready();
    declare(node_identifier_chain_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        ast_del_nodes(self, cur->identifier); \
        ast_del_nodes(self, cur->identifier_chain); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_this(NULL); \
    } \

    check("call ast_identifier");
    cur->identifier = ast_identifier(self, dep+1);
    if (!cur->identifier) {
        return_cleanup("");
    }

    token_t *t = *self->ptr++;
    if (!t) {
        return_cleanup("syntax error. reached EOF in identifier chain");
    }

    if (t->type != TOKEN_TYPE_DOT_OPE) {
        self->ptr--;
        return_this(node_new(NODE_TYPE_IDENTIFIER_CHAIN, cur));
    }

    check("call ast_identifier_chain");
    cur->identifier_chain = ast_identifier_chain(self, dep+1);
    if (!cur->identifier_chain) {
        self->ptr = save_ptr;
        if (ast_has_error(self)) {
            return_cleanup("");
        }

        return_cleanup("syntax error. not found identifier after \".\"");
    }

    return_this(node_new(NODE_TYPE_IDENTIFIER_CHAIN, cur));
}

static node_t *
ast_import_stmt(ast_t *self, int dep) {
    ready();
    declare(node_import_stmt_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        ast_del_nodes(self, cur->identifier_chain); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_this(NULL); \
    } \

    token_t *t = *self->ptr++;
    if (t->type != TOKEN_TYPE_STMT_IMPORT) {
        return_cleanup("")
    }

    check("call ast_identifier_chain");
    cur->identifier_chain = ast_identifier_chain(self, dep+1);
    if (!cur->identifier_chain) {
        if (ast_has_error(self)) {
            return_cleanup("")
        }

        return_cleanup("syntax error. not found import module");
    }

    t = *self->ptr;
    if (!(t->type == TOKEN_TYPE_NEWLINE ||
          t->type == TOKEN_TYPE_RBRACEAT)) {
        return_cleanup("syntax error. invalid token at end of import statement");
    }
    if (t->type == TOKEN_TYPE_NEWLINE) {
        ast_skip_newlines(self);
    }

    return_this(node_new(NODE_TYPE_IMPORT_STMT, cur));
}

static node_t *
ast_stmt(ast_t *self, int dep) {
    ready();
    declare(node_stmt_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        ast_del_nodes(self, cur->import_stmt); \
        ast_del_nodes(self, cur->if_stmt); \
        ast_del_nodes(self, cur->for_stmt); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_this(NULL); \
    } \

    check("call ast_import_stmt");
    cur->import_stmt = ast_import_stmt(self, dep+1);
    if (!cur->import_stmt) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }

        check("call ast_if_stmt");
        cur->if_stmt = ast_if_stmt(self, 0, dep+1);
        if (!cur->if_stmt) {
            if (ast_has_error(self)) {
                return_cleanup("");
            }

            check("call ast_for_stmt");
            cur->for_stmt = ast_for_stmt(self, dep+1);
            if (!cur->for_stmt) {
                if (ast_has_error(self)) {
                    return_cleanup("");
                }

                return_cleanup("");
            }
        }
    }

    return_this(node_new(NODE_TYPE_STMT, cur));
}

static node_t *
ast_elems(ast_t *self, int dep) {
    ready();
    declare(node_elems_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        ast_del_nodes(self, cur->stmt); \
        ast_del_nodes(self, cur->formula); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_this(NULL); \
    } \

    check("call ast_stmt");
    cur->stmt = ast_stmt(self, dep+1);
    if (!cur->stmt) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }

        check("call ast_formula");
        cur->formula = ast_formula(self, dep+1);
        if (!cur->formula) {
            if (ast_has_error(self)) {
                return_cleanup("");
            }
            // empty elems
            return_cleanup(""); // not error
        }
    }

    check("call ast_elems");
    cur->elems = ast_elems(self, dep+1);
    if (!cur->elems) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }
        return_this(node_new(NODE_TYPE_ELEMS, cur));
    }

    return_this(node_new(NODE_TYPE_ELEMS, cur));
}

static node_t *
ast_text_block(ast_t *self, int dep) {
    ready();
    declare(node_text_block_t, cur);
    token_t **save_ptr = self->ptr;

    token_t *t = *self->ptr++;
    if (t->type != TOKEN_TYPE_TEXT_BLOCK) {
        self->ptr = save_ptr;
        free(cur);
        return_this(NULL);
    }
    check("read text block");

    // move text
    cur->text = t->text;
    t->text = NULL;

    return_this(node_new(NODE_TYPE_TEXT_BLOCK, cur));
}

static node_t *
ast_ref_block(ast_t *self, int dep) {
    ready();
    declare(node_ref_block_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        ast_del_nodes(self, cur->formula); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_this(NULL); \
    } \

    token_t *t = *self->ptr++;
    if (t->type != TOKEN_TYPE_LDOUBLE_BRACE) {
        return_cleanup("");
    }

    check("call ast_formula");
    cur->formula = ast_formula(self, dep+1);
    if (!cur->formula) {
        return_cleanup("");
    }

    t = *self->ptr++;
    if (!t) {
        return_cleanup("syntax error. reached EOF in reference block");
    }

    if (t->type != TOKEN_TYPE_RDOUBLE_BRACE) {
        return_cleanup("syntax error. not found \"#}\"");
    }

    return_this(node_new(NODE_TYPE_REF_BLOCK, cur));
}

static node_t *
ast_code_block(ast_t *self, int dep) {
    ready();
    declare(node_code_block_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        ast_del_nodes(self, cur->elems); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_this(NULL); \
    } \

    token_t *t = *self->ptr++;
    if (t->type != TOKEN_TYPE_LBRACEAT) {
        return_cleanup("");
    }
    check("read {@");

    check("call ast_elems");
    cur->elems = ast_elems(self, dep+1);
    // elems allow null
    if (ast_has_error(self)) {
        return_cleanup("");
    }

    t = *self->ptr++;
    if (!t) {
        return_cleanup("syntax error. reached EOF in code block");
    }

    if (t->type != TOKEN_TYPE_RBRACEAT) {
        return_cleanup("");
        // return_cleanup("syntax error. not found \"@}\"");
    }
    check("read @}");

    ast_skip_newlines(self);
    check("skip newlines");

    return_this(node_new(NODE_TYPE_CODE_BLOCK, cur));
}

static node_t *
ast_blocks(ast_t *self, int dep) {
    ready();
    declare(node_blocks_t, cur);

#undef return_cleanup
#define return_cleanup() { \
        free(cur); \
        return_this(NULL); \
    } \

    check("call ast_code_block");
    cur->code_block = ast_code_block(self, dep+1);
    if (!cur->code_block) {
        if (ast_has_error(self)) {
            return_cleanup();
        }

        check("call ast_ref_block");
        cur->ref_block = ast_ref_block(self, dep+1);
        if (!cur->ref_block) {
            if (ast_has_error(self)) {
                return_cleanup();
            }

            check("call ast_text_block");
            cur->text_block = ast_text_block(self, dep+1);
            if (!cur->text_block) {
                return_cleanup();
            }
        }
    }

    cur->blocks = ast_blocks(self, dep+1);
    // allow null
    if (ast_has_error(self)) {
        return_cleanup();
    }

    return_this(node_new(NODE_TYPE_BLOCKS, cur));
}

static node_t *
ast_program(ast_t *self, int dep) {
    ready();
    declare(node_program_t, cur);

#undef return_cleanup
#define return_cleanup() { \
        free(cur); \
        return_this(NULL); \
    } \

    check("call ast_blocks");
    cur->blocks = ast_blocks(self, dep+1);
    if (!cur->blocks) {
        return_cleanup();
    }

    return_this(node_new(NODE_TYPE_PROGRAM, cur));
}

ast_t *
ast_parse(ast_t *self, token_t *tokens[]) {
    ast_clear(self);
    self->tokens = tokens;
    self->ptr = tokens;
    self->root = ast_program(self, 0);
    return self;
}

static void
_ast_traverse(ast_t *self, node_t *node) {
    if (node == NULL) {
        return; 
    }

    switch (node->type) {
    default: {
        err_die("impossible. unsupported node type %d", node_getc_type(node));
    } break;
    }
}

void
ast_traverse(ast_t *self, context_t *context) {
    self->context = context;
    _ast_traverse(self, self->root);
}

void
ast_clear(ast_t *self) {
    self->error_detail[0] = '\0';
    ast_del_nodes(self, self->root);
    self->root = NULL;
}

const char *
ast_get_error_detail(const ast_t *self) {
    return self->error_detail;
}

bool
ast_has_error(const ast_t *self) {
    return self->error_detail[0] != '\0';
}

void
ast_set_debug(ast_t *self, bool debug) {
    self->debug = debug;
}

#undef call
#undef viss
#undef vissf
#undef ready
#undef declare
