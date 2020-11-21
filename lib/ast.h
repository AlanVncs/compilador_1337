#ifndef AST_H
#define AST_H

#include "type.h"

#define AST_CHILDREN_BLOCK_SIZE 15

typedef enum {
    PROGRAM_START_NODE,
    INIT_DECL_LIST_NODE,
    PARAMETER_LIST_NODE,
    FUNCTION_DECL_NODE,
    FUNCTION_DEF_NODE,
    COMPOUND_STMT_NODE,

    // STRUCT_DECL_NODE,
    // ARG_LIST_NODE,
    // STRUCT_FIELDS_NODE,

    IF_NODE,
    WHILE_NODE,
    DO_WHILE_NODE,
    FOR_NODE,
    SWITCH_NODE,
    SWITCH_CASE_NODE,
    SWITCH_DEFAULT_NODE,

    LABEL_DECL_NODE,

    VAR_DECL_NODE,
    VAR_USE_NODE,

    CHAR_VAL_NODE,
    INT_VAL_NODE,
    FLOAT_VAL_NODE,
    DOUBLE_VAL_NODE,
    STR_VAL_NODE,

    // FUNC_CALL_NODE,
    GOTO_NODE,
    CONTINUE_NODE,
    BREAK_NODE,
    RETURN_NODE,

    TIMES_NODE,   // *
    OVER_NODE,    // /
    MOD_NODE,     // %
    PLUS_NODE,    // +
    MINUS_NODE,   // -
    BW_LSL_NODE,  // <<
    BW_LSR_NODE,  // >>
    LT_NODE,      // <
    GT_NODE,      // >
    LE_NODE,      // <=
    GE_NODE,      // >=
    EQ_NODE,      // ==
    NE_NODE,      // !=
    BW_AND_NODE,  // &
    BW_XOR_NODE,  // ^
    BW_OR_NODE,   // |
    AND_NODE,     // &&
    OR_NODE,      // ||
    TERN_OP_NOPE, // ?:
    ASSIGN_NODE,  // = += -= *= /= %= ^= |= &= >>= <<=

    // NOT_NODE, // !
    // BW_NOT_NODE, // ~

    // C2I_NODE,
    // C2F_NODE,
    // C2D_NODE,
    // I2C_NODE,
    // I2F_NODE,
    // I2D_NODE,
    // F2C_NODE,
    // F2I_NODE,
    // F2D_NODE,
    // D2C_NODE,
    // D2I_NODE,
    // D2F_NODE,

    NONE

} NodeKind;

typedef enum {
    OP_PLUS,         // +
    OP_MINUS,        // -
    OP_TIMES,        // /
    OP_OVER,         // *
    OP_MOD,          // %
    
    OP_LT,           // <
    OP_GT,           // >
    OP_LE,           // <=
    OP_GE,           // >=
    OP_EQ,           // ==
    OP_NE,           // !=

    OP_AND,          // &&
    OP_OR,           // ||
    OP_NOT,          // !

    OP_BW_XOR,       // ^
    OP_BW_AND,       // &
    OP_BW_OR,        // |
    OP_BW_LSR,       // >>
    OP_BW_LSL,       // <<
    OP_BW_NOT,       // ~

    OP_ASSIGN,       // =
    OP_MUL_ASSIGN,   // *=
    OP_DIV_ASSIGN,   // /=
    OP_MOD_ASSIGN,   // %=
    OP_ADD_ASSIGN,   // +=
    OP_SUB_ASSIGN,   // -=
    OP_LSL_ASSIGN,   // <<=
    OP_LSR_ASSIGN,   // >>=
    OP_AND_ASSIGN,   // &=
    OP_XOR_ASSIGN,   // ^=
    OP_OR_ASSIGN     // |=

} Op;

typedef struct {
    Type type;
    NodeKind lnk;
    NodeKind rnk;
} Unif;

struct data;
typedef struct ast AST;

AST* new_ast(Type type, NodeKind kind, ...);
void add_ast_child(AST *parent, AST *child);
AST* new_ast_subtree(Type type, NodeKind kind, int child_count, ...);
int has_float_data(AST* ast);
int has_data(AST* ast);
void gen_ast_dot(AST *ast);
void gen_node_dot(AST *node, FILE* ast_file);
void print_ast(AST *ast);

// Getters
char* get_kind_str(NodeKind kind);
char* get_op_str(Op op);
char* get_ast_name(AST* ast);

// Setters
void set_ast_name(AST* ast, char* name);


#endif