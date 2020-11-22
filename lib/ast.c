#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "ast.h"
#include "type.h"
// #include "table.h"

struct ast {
    union {
        int   int_data;
        float float_data;
    };
    int id;
    NodeKind kind;
    Type type;
    struct ast** children;
    int children_length;

    // Scope* scope; TODO remover name
    char name[128];
};

typedef struct {
    Type type;
    NodeKind lnk;
    NodeKind rnk;
} Unif;


// Create
int next_ast_id = 0;
AST* new_ast(Type type, NodeKind kind, ...) {

    AST* ast = malloc(sizeof(AST));

    ast->id = next_ast_id++;
    ast->kind = kind;
    ast->type = type;
    ast->children = NULL;
    ast->children_length = 0;
    ast->name[0] = '\0'; // TODO Remover

    // Set data filed
    va_list ap;
    va_start(ap, kind);
    if(has_float_data(ast)){
        ast->float_data = va_arg(ap, double);
    }
    else{
        ast->int_data = va_arg(ap, int);
    }
    va_end(ap);

    return ast;
}

AST* clone_ast(AST* source){
    AST* clone = new_ast(source->type, source->kind, 0);
    if(has_float_data(source))
        clone->float_data = source->float_data;
    else
        clone->int_data = source->int_data;
    set_ast_name(clone, source->name);
    return clone;
}

AST* new_ast_subtree(Type type, NodeKind kind, int child_count, ...){

    AST* parent = new_ast(type, kind, 0);

    va_list ap;
    va_start(ap, child_count);
    for (int i=0; i<child_count; i++) {
        add_ast_child(parent, va_arg(ap, AST*));
    }
    va_end(ap);

    return parent;
}


// Modify
void add_ast_child(AST* parent, AST* child){
    if (parent->children_length%AST_CHILDREN_BLOCK_SIZE == 0) {
        int new_size = AST_CHILDREN_BLOCK_SIZE + parent->children_length;
        parent->children = realloc(parent->children, new_size*sizeof(AST*));
        if(!parent->children){printf("add_ast_child: Could not allocate memory\n"); exit(EXIT_FAILURE);}
    }
    parent->children[parent->children_length] = child;
    parent->children_length++;
}


// Test
int has_float_data(AST* ast){
    switch (ast->kind) {
        case FLOAT_VAL_NODE:
        case DOUBLE_VAL_NODE: return 1;
        default: return 0;
    }  
}

int has_data(AST* ast){
    switch(ast->kind) {
        case CHAR_VAL_NODE:
        case INT_VAL_NODE:
        case FLOAT_VAL_NODE:
        case DOUBLE_VAL_NODE:
        case STR_VAL_NODE:
        case VAR_DECL_NODE:
        case VAR_USE_NODE:
            return 1;
        default:
            return 0;
    }
}


// Output
void gen_ast_dot(AST* ast){
    if(!ast){printf("gen_ast_dor: Empty AST\n\n"); return;}
    FILE* ast_file = fopen("ast.dot", "w+"); // TODO Why w+
    fprintf(ast_file, "digraph {\ngraph [ordering=\"out\"];\n");
    gen_node_dot(ast, ast_file);
    fprintf(ast_file, "}\n");
    fclose(ast_file);
}

void gen_node_dot(AST* node, FILE* ast_file){

    fprintf(ast_file, "node%d[label=\"", node->id);
    
    // Has a type
    if (node->type != NO_TYPE) {
        fprintf(ast_file, "(%s) ", get_type_str(node->type));
    }


    if (node->kind == VAR_DECL_NODE || node->kind == VAR_USE_NODE) {
        fprintf(ast_file, "%s@%d", get_ast_name(node), node->int_data);
    }
    else if(node->kind == STR_VAL_NODE){
        fprintf(ast_file, "str@%d", node->int_data);
    }
    else if (node->kind == CHAR_VAL_NODE || node->kind == INT_VAL_NODE) {
        fprintf(ast_file, "%s: %d", get_ast_name(node), node->int_data);
    } 
    else if(node->kind == FLOAT_VAL_NODE || node->kind == DOUBLE_VAL_NODE){
        fprintf(ast_file, "%s: %f", get_ast_name(node), node->float_data);
    }
    else {
        fprintf(ast_file, "%s", get_kind_str(node->kind));
    }

    fprintf(ast_file, "\"];\n");

    for (int i=0; i<node->children_length; i++) {
        AST* child = node->children[i];
        if(child){
            gen_node_dot(child, ast_file);
            fprintf(ast_file, "node%d -> node%d;\n", node->id, child->id);
        }
    }
}

void print_ast(AST* ast){
    if(!ast){printf("NULL AST\n\n\n"); return;}

    printf("-------------------\n");
    printf("        AST\n");
    printf("Type: %s\n", get_type_str(ast->type));
    printf("Kind: %s\n", get_kind_str(ast->kind));
    printf("Children length: %d\n", ast->children_length);
    printf("Data: ");
    if (ast->kind == VAR_DECL_NODE || ast->kind == VAR_USE_NODE) {
        printf("%s@%d\n", get_ast_name(ast), ast->int_data);
    }
    else if(ast->kind == STR_VAL_NODE){
        printf("str@%d\n", ast->int_data);
    }
    else if (ast->kind == CHAR_VAL_NODE || ast->kind == INT_VAL_NODE) {
        printf("%s:%d\n", get_ast_name(ast), ast->int_data);
    } 
    else if(ast->kind == FLOAT_VAL_NODE || ast->kind == DOUBLE_VAL_NODE){
        printf("%s:%f\n", get_ast_name(ast), ast->float_data);
    }
    else {
        printf("NO DATA\n");
    }
    printf("-------------------\n\n\n");
}


// Get
char* get_kind_str(NodeKind kind){
    // TODO
    switch(kind) {
        case PROGRAM_START_NODE:   return "program_start";
        case INIT_DECL_LIST_NODE:  return "init_decl_list";
        case FUNCTION_DECL_NODE:   return "function_decl";
        case FUNCTION_DEF_NODE:    return "function_def";
        case PARAMETER_LIST_NODE:  return "parameter_list";
        case ARGUMENT_LIST_NODE:   return "argument_list";
        case COMPOUND_STMT_NODE:   return "compound_stmt";
        
        // case STRUCT_DECL_NODE:     return "struct_decl";
        // case STRUCT_FIELDS_NODE:   return "struct_fields";

        case IF_NODE:              return "if_stmt";
        case SWITCH_NODE:          return "switch_stmt";
        case FOR_NODE:             return "for_stmt";
        case WHILE_NODE:           return "while_stmt";
        case DO_WHILE_NODE:        return "do_while_stmt";
        case SWITCH_CASE_NODE:     return "switch_case";
        case SWITCH_DEFAULT_NODE:  return "switch_default";

        case LABEL_DECL_NODE:      return "label_decl";
        case VAR_DECL_NODE:        return "var_decl";
        case VAR_USE_NODE:         return "var_use";

        case CHAR_VAL_NODE:        return "char_val";
        case INT_VAL_NODE:         return "int_val";
        case FLOAT_VAL_NODE:       return "float_val";
        case DOUBLE_VAL_NODE:      return "double_val";
        case STR_VAL_NODE:         return "str_val";

        case FUNCTION_CALL_NODE:   return "function_call";
        case GOTO_NODE:            return "goto";
        case CONTINUE_NODE:        return "continue";
        case BREAK_NODE:           return "break";
        case RETURN_NODE:          return "return";

        case TIMES_NODE:           return "*";
        case OVER_NODE:            return "/";
        case MOD_NODE:             return "%";
        case PLUS_NODE:            return "+";
        case MINUS_NODE:           return "-";
        case BW_LSL_NODE:          return "<<";
        case BW_LSR_NODE:          return ">>";
        case LT_NODE:              return "<";
        case GT_NODE:              return ">";
        case LE_NODE:              return "<=";
        case GE_NODE:              return ">=";
        case EQ_NODE:              return "==";
        case NE_NODE:              return "!=";
        case BW_AND_NODE:          return "&";
        case BW_XOR_NODE:          return "^";
        case BW_OR_NODE:           return "|";
        case AND_NODE:             return "&&";
        case OR_NODE:              return "||";
        case TERN_OP_NOPE:         return "?:";
        case ASSIGN_NODE:          return "=";

        // case C2I_NODE:              return "c2i";
        // case C2F_NODE:              return "c2f";
        // case C2D_NODE:              return "c2d";
        // case I2C_NODE:              return "i2c";
        // case I2F_NODE:              return "i2f";
        // case I2D_NODE:              return "i2d";
        // case F2C_NODE:              return "f2c";
        // case F2I_NODE:              return "f2i";
        // case F2D_NODE:              return "f2d";
        // case D2C_NODE:              return "d2c";
        // case D2I_NODE:              return "d2i";
        // case D2F_NODE:              return "d2f";
        
        default:
            printf("get_str_kind: NodeKind <%d> does not exists!\n\n", kind);
            exit(EXIT_FAILURE);
    }
}

char* get_op_str(Op op){
    switch(op){
        case OP_PLUS:       return "+";
        case OP_MINUS:      return "-";
        case OP_TIMES:      return "/";
        case OP_OVER:       return "*";
        case OP_MOD:        return "%";

        case OP_LT:         return "<";
        case OP_GT:         return ">";
        case OP_LE:         return "<=";
        case OP_GE:         return ">=";
        case OP_EQ:         return "==";
        case OP_NE:         return "!=";

        case OP_AND:        return "&&";
        case OP_OR:         return "||";
        case OP_NOT:        return "!";

        case OP_BW_XOR:     return "^";
        case OP_BW_AND:     return "&";
        case OP_BW_OR:      return "|";
        case OP_BW_LSR:     return ">>";
        case OP_BW_LSL:     return "<<";
        case OP_BW_NOT:     return "~";

        case OP_ASSIGN:     return "=";
        case OP_MUL_ASSIGN: return "*=";
        case OP_DIV_ASSIGN: return "/=";
        case OP_MOD_ASSIGN: return "%=";
        case OP_ADD_ASSIGN: return "+=";
        case OP_SUB_ASSIGN: return "-=";
        case OP_LSL_ASSIGN: return "<<=";
        case OP_LSR_ASSIGN: return ">>=";
        case OP_AND_ASSIGN: return "&=";
        case OP_XOR_ASSIGN: return "^=";
        case OP_OR_ASSIGN:  return "|=";

        default:
            printf("op2text: Unknown operator (%d)\n", op);
            exit(EXIT_FAILURE);
    }
}

char* get_ast_name(AST* ast){
    return ast->name;
}


// Set
void set_ast_name(AST* ast, char* name){
    strcpy(ast->name, name);
}






// void free_tree(AST *tree);
// NodeKind get_kind(AST *node);
// int get_data(AST *node);
// void set_float_data(AST *node, float data);
// float get_float_data(AST *node);
// Type get_node_type(AST *node);
// int get_child_count(AST *node);
// void set_scope(AST *node, Scope *scope);
// Unif get_kinds(Op op, Type lt, Type rt);


/*

AST* get_first_child(AST* ast){
    return ast->child[0];
}

// Remove a variavle de uma var_decl_list
Entry *remove_var(Scope *scope, int idx){

    Entry *removed = scope->table[idx];
    int length = scope->table_length;

    for(int i=idx; i<length-1; i++){
        scope->table[i] = scope->table[i+1];
    }
    scope->table_length--;
    
    return removed;
}

AST* new_node(NodeKind kind, int data, Type type, Scope* scope) {
    AST* node = malloc(sizeof * node);
    node->kind = kind;
    node->data.as_int = data;
    node->type = type;
    node->count = 0;
    node->scope=scope;

    for (int i = 0; i < CHILDREN_LIMIT; i++) {
        node->child[i] = NULL;
    }
    // Inicializacao da lista encadeada aqui
    return node;
}

void add_child(AST *parent, AST *child) {
    if (parent->count == CHILDREN_LIMIT) {
        fprintf(stderr, "Cannot add another child!\n");
        exit(1);
    }
    parent->child[parent->count] = child;
    parent->count++;
}

AST* new_subtree(NodeKind kind, Type type, int child_count, ...) {
    if (child_count > CHILDREN_LIMIT) {
        fprintf(stderr, "Too many children as arguments!\n");
        exit(1);
    }

    AST* node = new_node(kind, 0, type, NULL);
    va_list ap;
    va_start(ap, child_count);
    for (int i = 0; i < child_count; i++) {
        add_child(node, va_arg(ap, AST*));
    }
    va_end(ap);
    return node;
}

NodeKind get_kind(AST *node) {
    return node->kind;
}

int get_data(AST *node) {
    return node->data.as_int;
}

void set_float_data(AST *node, float data) {
    node->data.as_float = data;
}

float get_float_data(AST *node) {
    return node->data.as_float;
}

Type get_node_type(AST *node) {
    return node->type;
}

int get_child_count(AST *node) {
    return node->count;
}

Scope* get_scope(AST *node){
    return node->scope;
}

void set_scope(AST *node, Scope *scope){
    node->scope = scope;
}

void free_tree(AST *tree) {
    if (tree == NULL) return;
    for (int i = 0; i < tree->count; i++) {
        free_tree(tree->child[i]);
    }
    free(tree);
}

//----------------------------------------------

int nr;

char* kind2str(NodeKind kind) {
    switch(kind) {
        case VAR_DECL_NODE:         return "var_decl";
        case VAR_USE_NODE:          return "var_use";
        case TRANSLATION_UNIT_NODE: return "translation_unit";
        case DECLARATION_NODE:      return "declaration_node";
        case FUNCTION_DEF_NODE:     return "function_definition";
        case EXTERNAL_DECL_NODE:    return "external_decl";
        case COMPOUND_STMT_NODE:    return "compound_stmt";
        case PARAMETER_DECL_NODE:   return "param_decl";
        case DIRECT_DECL_NODE:      return "direct_decl";
        case PARAMETER_LIST_NODE:   return "param_list";
        case PARAM_TYPE_LIST_NODE:  return "param_type_list";
        case INIT_DECL_LIST_NODE:   return "init_decl_list";
        case BLOCK_ITEM_LIST_NODE:  return "block_list";
        case STRUCT_DECL_NODE:      return "struct_decl";
        case STMT_NODE:             return "stataments";
        case IF_NODE:               return "if";
        case WHILE_NODE:            return "while";
        case DO_WHILE_NODE:         return "do_while";
        case FOR_NODE:              return "for";
        case EXPR_NODE:             return "expression";
        case EMPTY_STMT_NODE:       return ";";
        case ARG_LIST_NODE:         return "args";
        case FUNC_CALL_NODE:        return "function_call";
        case STRUCT_FIELDS_NODE:    return "struct_fields";
        case GOTO_NODE:             return "goto";
        case CONTINUE_NODE:         return "continue";
        case BREAK_NODE:            return "break";
        case RETURN_NODE:           return "return";
        case EMPTY_BLOCK_NODE:      return "{}";

        case INT_VAL_NODE:          return "";
        case CHAR_VAL_NODE:         return "";
        case FLOAT_VAL_NODE:        return "";
        case DOUBLE_VAL_NODE:       return "";

        case ASSIGN_NODE:           return "=";
        case MINUS_NODE:            return "-";
        case OVER_NODE:             return "/";
        case PLUS_NODE:             return "+";
        case TIMES_NODE:            return "*";
        case MOD_NODE:              return "%";
        case LOG_EQ_NODE:           return "==";
        case LOG_NE_NODE:           return "!=";
        case LOG_LT_NODE:           return "<";
        case LOG_GT_NODE:           return ">";
        case LOG_LE_NODE:           return "<=";
        case LOG_GE_NODE:           return ">=";
        case LOG_OR_NODE:           return "||";
        case LOG_AND_NODE:          return "&&";
        case LOG_NOT_NODE:          return "!";
        case BIT_OR_NODE:           return "|";
        case BIT_XOR_NODE:          return "^";
        case BIT_AND_NODE:          return "&";
        case BIT_LSH_NODE:          return "<<";
        case BIT_RSH_NODE:          return ">>";
        case BIT_NOT_NODE:          return "~";

        case C2I_NODE:              return "c2i";
        case C2F_NODE:              return "c2f";
        case C2D_NODE:              return "c2d";
        case I2C_NODE:              return "i2c";
        case I2F_NODE:              return "i2f";
        case I2D_NODE:              return "i2d";
        case F2C_NODE:              return "f2c";
        case F2I_NODE:              return "f2i";
        case F2D_NODE:              return "f2d";
        case D2C_NODE:              return "d2c";
        case D2I_NODE:              return "d2i";
        case D2F_NODE:              return "d2f";
        
        default:                    return "ERROR!!";
    }
}

int has_data(NodeKind kind) {
    switch(kind) {
        case INT_VAL_NODE:
        case DOUBLE_VAL_NODE:
        case FLOAT_VAL_NODE:
        case CHAR_VAL_NODE:
        case VAR_DECL_NODE:
        case VAR_USE_NODE:
            return 1;
        default:
            return 0;
    }
}

int print_node_dot(AST *node, FILE* ast_file) {
    int my_nr = nr++;

    fprintf(ast_file, "node%d[label=\"", my_nr);
    if (node->type != NO_TYPE) {
        fprintf(ast_file, "(%s) ", type2text(node->type));
    }
    if (node->kind == VAR_DECL_NODE || node->kind == VAR_USE_NODE) {
        fprintf(ast_file, "%s@", get_name(node->scope, node->data.as_int));
    } else {
        fprintf(ast_file, "%s", kind2str(node->kind));
    }
    if (has_data(node->kind)) {
        if (node->kind == FLOAT_VAL_NODE) {
            fprintf(ast_file, "%.2f", node->data.as_float);
        } else if (node->kind == CHAR_VAL_NODE) {
            fprintf(ast_file, "@%d", node->data.as_int);
        } else {
            fprintf(ast_file, "%d", node->data.as_int);
        }
    }
    fprintf(ast_file, "\"];\n");

    for (int i = 0; i < node->count; i++) {
        int child_nr = print_node_dot(node->child[i], ast_file);
        fprintf(ast_file, "node%d -> node%d;\n", my_nr, child_nr);
    }
    return my_nr;
}

void print_dot(AST *tree) {
    FILE* ast_file = fopen("ast.dot", "w+");
    nr = 0;
    fprintf(ast_file, "digraph {\ngraph [ordering=\"out\"];\n");
    print_node_dot(tree, ast_file);
    fprintf(ast_file, "}\n");
    fclose(ast_file);
}

void show_ast(AST *ast){
    printf("-------------------\n");
    printf("        AST\n");
    printf("Type: %s\n", type2text(ast->type));
    printf("Kind: %s\n", kind2str(ast->kind));
    printf("Count: %d\n", ast->count);
    if(ast->scope){
        int idx = get_data(ast);
        printf("Table idx: %d\n", idx);
        printf("Scope ID: %d\n", ast->scope->id);
        printf("Name: %s\n", ast->scope->table[idx]->name);
        printf("Type: %s\n", type2text(ast->scope->table[idx]->type));
        printf("Line: %d\n", ast->scope->table[idx]->line);
    }
    else{
        printf("Scope: NULL\n");
    }
    printf("-------------------\n\n\n");
}

 
//  + - * /
static const Unif tableA[4][4] = {
    //          int    char    float    double
    //    int   int    int     float    double
    //   char   int    char    float    double
    //  float   float  float   float    double
    // double   double double  double   double
    {{INT_TYPE, NONE, NONE},        {INT_TYPE, NONE, C2I_NODE},    {FLOAT_TYPE, I2F_NODE, NONE},  {DOUBLE_TYPE, I2D_NODE, NONE}},
    {{INT_TYPE, C2I_NODE, NONE},    {CHAR_TYPE, NONE, NONE},       {FLOAT_TYPE, C2F_NODE, NONE},  {DOUBLE_TYPE, C2D_NODE, NONE}},
    {{FLOAT_TYPE, NONE, I2F_NODE},  {FLOAT_TYPE, NONE, C2F_NODE},  {FLOAT_TYPE, NONE, NONE},      {DOUBLE_TYPE, F2D_NODE, NONE}},
    {{DOUBLE_TYPE, NONE, I2D_NODE}, {DOUBLE_TYPE, NONE, C2D_NODE}, {DOUBLE_TYPE, NONE, F2D_NODE}, {DOUBLE_TYPE, NONE, NONE}}
};

// < > && || <= >= == !=
static const Unif tableB[4][4] = {
    //          int    char    float    double
    //    int   int    int     int      int
    //   char   int    int     int      int
    //  float   int    int     int      int
    // double   int    int     int      int
    {{INT_TYPE, NONE, NONE},     {INT_TYPE, NONE, C2I_NODE}, {INT_TYPE, I2F_NODE, NONE}, {INT_TYPE, I2D_NODE, NONE}},
    {{INT_TYPE, C2I_NODE, NONE}, {INT_TYPE, NONE, NONE},     {INT_TYPE, C2F_NODE, NONE}, {INT_TYPE, C2D_NODE, NONE}},
    {{INT_TYPE, NONE, I2F_NODE}, {INT_TYPE, NONE, C2F_NODE}, {INT_TYPE, NONE, NONE},     {INT_TYPE, F2D_NODE, NONE}},
    {{INT_TYPE, NONE, I2D_NODE}, {INT_TYPE, NONE, C2D_NODE},  {INT_TYPE, NONE, F2D_NODE}, {INT_TYPE, NONE, NONE}}
};

// ^ & | >> << %
static const Unif tableC[4][4] = {
    //          int    char    float    double
    //    int   int    int     err      err
    //   char   int    int     err      err
    //  float   err    err     err      err
    // double   err    err     err      err
    {{INT_TYPE, NONE, NONE},     {INT_TYPE, NONE, C2I_NODE}, {ERR_TYPE, NONE, NONE}, {ERR_TYPE, NONE, NONE}},
    {{INT_TYPE, C2I_NODE, NONE}, {INT_TYPE, NONE, NONE},     {ERR_TYPE, NONE, NONE}, {ERR_TYPE, NONE, NONE}},
    {{ERR_TYPE, NONE, NONE},     {ERR_TYPE, NONE, NONE},     {ERR_TYPE, NONE, NONE}, {ERR_TYPE, NONE, NONE}},
    {{ERR_TYPE, NONE, NONE},     {ERR_TYPE, NONE, NONE},     {ERR_TYPE, NONE, NONE}, {ERR_TYPE, NONE, NONE}}
};

// =
static const Unif tableD[4][4] = {
    //          int    char    float    double
    //    int   int    int     int      int
    //   char   char   char    char     char
    //  float   float  float   float    float
    // double   double double  double   double
    {{INT_TYPE, NONE, NONE},        {INT_TYPE, NONE, C2I_NODE},    {INT_TYPE, NONE, F2I_NODE},    {INT_TYPE, NONE, D2I_NODE}},
    {{CHAR_TYPE, NONE, I2C_NODE},   {CHAR_TYPE, NONE, NONE},       {CHAR_TYPE, NONE, F2C_NODE},   {CHAR_TYPE, NONE, D2C_NODE}},
    {{FLOAT_TYPE, NONE, I2F_NODE},  {FLOAT_TYPE, NONE, C2F_NODE},  {FLOAT_TYPE, NONE, NONE},      {FLOAT_TYPE, NONE, D2F_NODE}},
    {{DOUBLE_TYPE, NONE, I2D_NODE}, {DOUBLE_TYPE, NONE, C2D_NODE}, {DOUBLE_TYPE, NONE, F2D_NODE}, {DOUBLE_TYPE, NONE, NONE}}
};

Unif get_kinds(Op op, Type lt, Type rt){
    switch(op){
        case OP_PLUS: 
        case OP_MINUS:
        case OP_TIMES:
        case OP_OVER:   return tableA[lt][rt];

        case OP_LT:
        case OP_GT:
        case OP_AND:
        case OP_OR:
        case OP_LE:
        case OP_GE:
        case OP_NE:
        case OP_EQ:     return tableB[lt][rt];

        case OP_XOR:
        case OP_BAND:
        case OP_BOR:
        case OP_RSH:
        case OP_LSH:
        case OP_MOD:    return tableC[lt][rt];

        case OP_ASSIGN: return tableD[lt][rt];

        default:
            printf("get_kinds: Error - Unknown operator %s\n", op2text(op));
            exit(EXIT_FAILURE);
    }
}

char* op2text(Op op){
    switch(op){
        case OP_PLUS:         return "+";
        case OP_MINUS:        return "-";
        case OP_TIMES:        return "*";
        case OP_OVER:         return "/";
        case OP_MOD:          return "%";

        case OP_LT:           return "<";
        case OP_GT:           return ">";
        case OP_AND:          return "&&";
        case OP_OR:           return "||";
        case OP_LE:           return "<=";
        case OP_GE:           return ">=";
        case OP_NE:           return "!=";
        case OP_EQ:           return "==";

        case OP_XOR:          return "^";
        case OP_BAND:         return "&";
        case OP_BOR:          return "|";
        case OP_RSH:          return ">>";
        case OP_LSH:          return "<<";
        case OP_BNOT:         return "~"; 
        case OP_NOT:          return "!";  

        case OP_ASSIGN:       return "=";
        case OP_MUL_ASSIGN:   return "*=";
        case OP_DIV_ASSIGN:   return "/=";
        case OP_MOD_ASSIGN:   return "%=";
        case OP_ADD_ASSIGN:   return "+=";
        case OP_SUB_ASSIGN:   return "-=";
        case OP_LEFT_ASSIGN:  return "<<=";
        case OP_RIGHT_ASSIGN: return ">>=";
        case OP_AND_ASSIGN:   return "&=";
        case OP_XOR_ASSIGN:   return "^=";
        case OP_OR_ASSIGN:    return "|=";

        default:
            printf("op2text: Unknown operator (%d)\n", op);
            exit(EXIT_FAILURE);
    }
}


// "!"  // Criar node de negação
// "~"  // Criar node de negação bitwise
// "++" // Criar node de soma dentro de um node de atribuição
// "--" //       ||      sub                ||


// "?" ":"
// "."
// "->"
//
*/