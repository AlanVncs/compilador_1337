#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "ast.h"
#include "type.h"

struct ast {
    union {
        int   int_data;
        float float_data;
    };
    int id;
    NodeKind kind;
    Type type;
    char* name;
    int line;
    struct ast** children;
    int children_length;
    struct ast* parent;
};


// Create | Delete
int next_ast_id = 0;
AST* new_ast(Type type, char* name, int line, NodeKind kind, ...) {

    AST* ast = malloc(sizeof(AST));

    ast->id = next_ast_id++;
    ast->type = type;
    ast->name = NULL;
    if(name){
        ast->name = malloc((strlen(name)+1)*sizeof(char));
        strcpy(ast->name, name);
    }
    ast->line = line;
    ast->kind = kind;
    ast->children = NULL;
    ast->children_length = 0;
    ast->parent = NULL;

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
    AST* clone = new_ast(source->type, source->name, source->line, source->kind, 0);
    if(has_float_data(source))
        clone->float_data = source->float_data;
    else
        clone->int_data = source->int_data;
    return clone;
}

AST* new_ast_subtree(Type type, char* name, int line, NodeKind kind, int child_count, ...){

    AST* parent = new_ast(type, name, line, kind, 0);

    va_list ap;
    va_start(ap, child_count);
    for (int i=0; i<child_count; i++) {
        add_ast_child(parent, va_arg(ap, AST*));
    }
    va_end(ap);

    return parent;
}

void delete_ast(AST *ast) {
    if (ast == NULL) return;
    for (int i=0; i<ast->children_length; i++) {
        delete_ast(ast->children[i]);
    }
    free(ast->name);
    free(ast->children);
    free(ast);
}


// Modify
AST* add_ast_child(AST* parent, AST* child){

    // TODO Remover: Esta verificação é inútil quando o compilador está funcionando corretamente
    if(!child){printf("add_ast_child: 'child' não deveria chegar nulo\n"); exit(EXIT_FAILURE);}

    if (parent->children_length%AST_CHILDREN_BLOCK_SIZE == 0) {
        int new_size = AST_CHILDREN_BLOCK_SIZE + parent->children_length;
        parent->children = realloc(parent->children, new_size*sizeof(AST*));
        if(!parent->children){printf("add_ast_child: Could not allocate memory\n"); exit(EXIT_FAILURE);}
    }
    parent->children[parent->children_length++] = child;
    child->parent = parent;
    return parent;
}


// Test
int has_float_data(AST* ast){
    switch (ast->kind) {
        case FLOAT_VAL_NODE: return 1;
        default: return 0;
    }  
}

int has_data(AST* ast){
    switch(ast->kind) {
        case CHAR_VAL_NODE:
        case INT_VAL_NODE:
        case FLOAT_VAL_NODE:
        case STR_VAL_NODE:
        case VAR_DECL_NODE:
        case VAR_USE_NODE:
            return 1;
        default:
            return 0;
    }
}

int compare_func_decl(AST* func_decl_a, AST* func_decl_b){

    // Compare type
    if(func_decl_a->type != func_decl_b->type) return 1;

    // Compare name
    if(strcmp(get_ast_name(func_decl_a), get_ast_name(func_decl_b))) return 1;

    // Compare param_list
    return compare_param_list(func_decl_a->children[1], func_decl_b->children[1]);
}

int compare_param_list(AST* param_list_a, AST* param_list_b){

    int length_a = param_list_a->children_length;
    int length_b = param_list_b->children_length;

    if(length_a != length_b) return 1;

    // Compare types
    for(int i=0; i<length_a; i++){
        AST* param_a = param_list_a->children[i];
        AST* param_b = param_list_b->children[i];
        if(param_a->type != param_b->type) return 1;
    }
    
    return 0;
}


// Output
void gen_ast_dot(AST* ast){
    if(!ast){printf("gen_ast_dor: Empty AST\n\n"); return;}
    FILE* ast_file = fopen("ast.dot", "w");
    fprintf(ast_file, "digraph {\ngraph [ordering=\"out\"];\n");
    gen_ast_node_dot(ast, ast_file);
    fprintf(ast_file, "}\n");
    fclose(ast_file);
}

void gen_ast_node_dot(AST* node, FILE* ast_file){

    fprintf(ast_file, "node%d[label=\"", node->id);
    
    if (node->kind == VAR_DECL_NODE) {
        char* node_name = get_ast_name(node);
        if(node_name)
            fprintf(ast_file, "%s %s", get_type_str(get_ast_type(node)), node_name);
        else
            fprintf(ast_file, "%s", get_type_str(get_ast_type(node)));
    }
    else if(node->kind == VAR_USE_NODE){
        fprintf(ast_file, "%s (%s)", get_ast_name(node), get_type_str(get_ast_type(node)));
    }
    else if(node->kind == STR_VAL_NODE){
        fprintf(ast_file, "@%d (string)", node->int_data);
    }
    else if(node->kind == CHAR_VAL_NODE){
        fprintf(ast_file, "'%c' (char)", node->int_data);
    }
    else if (node->kind == INT_VAL_NODE) {
        fprintf(ast_file, "%d (int)", node->int_data);
    } 
    else if(node->kind == FLOAT_VAL_NODE){
        fprintf(ast_file, "%.2f (float)", node->float_data);
    }
    else {
        // Has a type
        if (node->type != NO_TYPE) {
            fprintf(ast_file, "(%s) ", get_type_str(node->type));
        }
        fprintf(ast_file, "%s", get_kind_str(node->kind));
    }

    fprintf(ast_file, "\"];\n");

    for (int i=0; i<node->children_length; i++) {
        AST* child = node->children[i];
        if(child){
            gen_ast_node_dot(child, ast_file);
            fprintf(ast_file, "node%d -> node%d;\n", node->id, child->id);
        }
    }
}

void print_ast(AST* ast){
    if(!ast){printf("NULL AST\n\n"); return;}

    printf("-------------------\n");
    printf("        AST\n");
    printf("ID: %d\n", get_ast_id(ast));
    printf("Kind: %s\n", get_kind_str(get_ast_kind(ast)));
    printf("Type: %s\n", get_type_str(get_ast_type(ast)));
    printf("Name: %s\n", get_ast_name(ast));
    printf("Line: %d\n", get_ast_line(ast));
    printf("Children length: %d\n", get_ast_length(ast));
    printf("Data: ");
    if (ast->kind == VAR_DECL_NODE || ast->kind == VAR_USE_NODE) {
        printf("%s@%d\n", get_ast_name(ast), ast->int_data);
    }
    else if(ast->kind == STR_VAL_NODE){
        printf("str@%d\n", ast->int_data);
    }
    else if(ast->kind == CHAR_VAL_NODE){
        printf("'%c' (char)\n", ast->int_data);
    }
    else if (ast->kind == INT_VAL_NODE) {
        printf("%d (int)\n", ast->int_data);
    } 
    else if(ast->kind == FLOAT_VAL_NODE){
        printf("%f (float)\n", ast->float_data);
    }
    else {
        printf("NO DATA\n");
    }
    printf("-------------------\n\n\n");
}


// Get
int get_ast_id(AST* ast){
    return ast->id;
}

NodeKind get_ast_kind(AST* ast){
    if(!ast) return NONE;
    return ast->kind;
}

Type get_ast_type(AST* ast){
    if(!ast) return ERR_TYPE;
    return ast->type;
}

char* get_ast_name(AST* ast){
    if(ast->kind == FUNCTION_DEF_NODE || ast->kind == FUNCTION_DECL_NODE || ast->kind == FUNCTION_CALL_NODE){
        return get_ast_name(ast->children[0]);
    }
    return ast->name;
}

int get_ast_line(AST* ast){
    return ast->line;
}

AST* get_ast_child(AST* ast, int i){
    if(i >= ast->children_length) return NULL;
    return ast->children[i];
}

int get_ast_length(AST* ast){
    return ast->children_length;
}

char* get_kind_str(NodeKind kind){
    // TODO
    switch(kind) {
        case PROGRAM_START_NODE:     return "program_start";
        case INIT_DECL_LIST_NODE:    return "init_decl_list";
        case FUNCTION_DECL_NODE:     return "function_decl";
        case FUNCTION_DEF_NODE:      return "function_def";
        case PARAMETER_LIST_NODE:    return "parameter_list";
        case STRUCT_FIELD_LIST_NODE: return "struct_field_list";
        case ARGUMENT_LIST_NODE:     return "argument_list";
        case COMPOUND_STMT_NODE:     return "compound_stmt";

        case IF_NODE:               return "if_stmt";
        case SWITCH_NODE:           return "switch_stmt";
        case FOR_NODE:              return "for_stmt";
        case WHILE_NODE:            return "while_stmt";
        case DO_WHILE_NODE:         return "do_while_stmt";
        case SWITCH_CASE_NODE:      return "switch_case";
        case SWITCH_DEFAULT_NODE:   return "switch_default";

        case STRUCT_VAR_DECL_NODE:  return "struct_var_decl";
        case STRUCT_DECL_NODE:      return "struc_decl";

        case VAR_DECL_INIT_NODE:    return "var_decl_init";
        case VAR_DECL_NODE:         return "var_decl";
        case VAR_USE_NODE:          return "var_use";

        case EXPRESSION_NODE:  return "expression";

        case CHAR_VAL_NODE:         return "char_val";
        case INT_VAL_NODE:          return "int_val";
        case FLOAT_VAL_NODE:        return "float_val";
        case STR_VAL_NODE:          return "str_val";

        case FUNCTION_CALL_NODE:    return "function_call";
        case CONTINUE_NODE:         return "continue";
        case BREAK_NODE:            return "break";
        case RETURN_NODE:           return "return";

        case CAST_NODE:             return "cast";

        case SIZEOF_NODE:           return "sizeof";
        case TIMES_NODE:            return "*";
        case OVER_NODE:             return "/";
        case MOD_NODE:              return "%";
        case PLUS_NODE:             return "+";
        case MINUS_NODE:            return "-";
        case BW_LSL_NODE:           return "<<";
        case BW_LSR_NODE:           return ">>";
        case LT_NODE:               return "<";
        case GT_NODE:               return ">";
        case LE_NODE:               return "<=";
        case GE_NODE:               return ">=";
        case EQ_NODE:               return "==";
        case NE_NODE:               return "!=";
        case BW_AND_NODE:           return "&";
        case BW_XOR_NODE:           return "^";
        case BW_OR_NODE:            return "|";
        case AND_NODE:              return "&&";
        case OR_NODE:               return "||";
        case TERN_OP_NODE:          return "?:";
        case ASSIGN_NODE:           return "=";

        case ADDRESS_NODE:          return "&";
        case DEREFERENCE_NODE:      return "*";
        case POSITIVE_NODE:         return "+";
        case NEGATIVE_NODE:         return "-";
        case BW_NOT_NODE:           return "~";
        case NOT_NODE:              return "!";

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
        
        default:
            printf("get_kind_str: NodeKind <%d> does not exists!\n\n", kind);
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

char* get_var_kind(AST* ast){
    switch (ast->kind){
        case FUNCTION_DEF_NODE:  return "func_def";
        case FUNCTION_DECL_NODE: return "func_decl";
        case VAR_DECL_NODE:      return "variable";
        
        default: return "not_var";
    }
}

NodeKind get_conv_node(Type old_type, Type new_type){

    static NodeKind table[4][4] = {
    //       char       int     float    double
        {    NONE, C2I_NODE, C2F_NODE, C2D_NODE},
        {I2C_NODE,     NONE, I2F_NODE, I2D_NODE},
        {F2C_NODE, F2I_NODE,     NONE, F2D_NODE},
        {D2C_NODE, D2I_NODE, D2F_NODE,     NONE}
    };

    return table[old_type][new_type];
}

Type get_function_def_type(AST* return_stmt){
    AST* parent;
    for(parent = return_stmt->parent; parent->kind != FUNCTION_DEF_NODE; parent = parent->parent);

    return parent->type;
}

Op get_ast_op(AST* operation){

    if(!operation) printf("get_ast_op: operation null\n");

    switch (operation->kind){
        case PLUS_NODE:   return OP_PLUS;   // +
        case MINUS_NODE:  return OP_MINUS;  // -
        case OVER_NODE:   return OP_OVER;   // *
        case TIMES_NODE:  return OP_TIMES;  // /

        case LT_NODE:     return OP_LT;     // <
        case GT_NODE:     return OP_GT;     // >
        case AND_NODE:    return OP_AND;    // &&
        case OR_NODE:     return OP_OR;     // ||
        case LE_NODE:     return OP_LE;     // <=
        case GE_NODE:     return OP_GE;     // >=
        case EQ_NODE:     return OP_EQ;     // ==
        case NE_NODE:     return OP_NE;     // !=

        case BW_XOR_NODE: return OP_BW_XOR; // ^
        case BW_AND_NODE: return OP_BW_AND; // &
        case BW_OR_NODE:  return OP_BW_OR;  // |
        case BW_LSR_NODE: return OP_BW_LSR; // >>
        case BW_LSL_NODE: return OP_BW_LSL; // <<
        case MOD_NODE:    return OP_MOD;    // %

        case ASSIGN_NODE: return OP_ASSIGN; // =
        
        default: printf("get_ast_op: Error"); exit(EXIT_FAILURE); break;
    }
}

Unif get_ast_unif(AST* operation){
    
    //  + - * /
    static const Unif tableA[4][4] = {
        //          char   int    float    double
        //   char   char   int    float    double
        //    int   int    int    float    double
        //  float   float  float  float    double
        // double   double double double   double
        {{CHAR_TYPE, CHAR_TYPE},     {INT_TYPE, INT_TYPE},       {FLOAT_TYPE, FLOAT_TYPE},   {DOUBLE_TYPE, DOUBLE_TYPE}},
        {{INT_TYPE, INT_TYPE},       {INT_TYPE, INT_TYPE},       {FLOAT_TYPE, FLOAT_TYPE},   {DOUBLE_TYPE, DOUBLE_TYPE}},
        {{FLOAT_TYPE, FLOAT_TYPE},   {FLOAT_TYPE, FLOAT_TYPE},   {FLOAT_TYPE, FLOAT_TYPE},   {DOUBLE_TYPE, DOUBLE_TYPE}},
        {{DOUBLE_TYPE, DOUBLE_TYPE}, {DOUBLE_TYPE, DOUBLE_TYPE}, {DOUBLE_TYPE, DOUBLE_TYPE}, {DOUBLE_TYPE, DOUBLE_TYPE}}
    };

    // < > && || <= >= == !=
    static const Unif tableB[4][4] = {
        //          char   int     float    double
        //   char   int    int     int      int
        //    int   int    int     int      int
        //  float   int    int     int      int
        // double   int    int     int      int
        {{INT_TYPE, CHAR_TYPE},   {INT_TYPE, INT_TYPE},    {INT_TYPE, FLOAT_TYPE},  {INT_TYPE, DOUBLE_TYPE}},
        {{INT_TYPE, INT_TYPE},    {INT_TYPE, INT_TYPE},    {INT_TYPE, FLOAT_TYPE},  {INT_TYPE, DOUBLE_TYPE}},
        {{INT_TYPE, FLOAT_TYPE},  {INT_TYPE, FLOAT_TYPE},  {INT_TYPE, FLOAT_TYPE},  {INT_TYPE, DOUBLE_TYPE}},
        {{INT_TYPE, DOUBLE_TYPE}, {INT_TYPE, DOUBLE_TYPE}, {INT_TYPE, DOUBLE_TYPE}, {INT_TYPE, DOUBLE_TYPE}}
    };

    // ^ & | >> << %
    static const Unif tableC[4][4] = {
        //          char    int    float    double
        //   char   int     int    err      err
        //    int   int     int    err      err
        //  float   err     err    err      err
        // double   err     err    err      err
        {{INT_TYPE, INT_TYPE}, {INT_TYPE, INT_TYPE}, {ERR_TYPE, ERR_TYPE}, {ERR_TYPE, ERR_TYPE}},
        {{INT_TYPE, INT_TYPE}, {INT_TYPE, INT_TYPE}, {ERR_TYPE, ERR_TYPE}, {ERR_TYPE, ERR_TYPE}},
        {{ERR_TYPE, ERR_TYPE}, {ERR_TYPE, ERR_TYPE}, {ERR_TYPE, ERR_TYPE}, {ERR_TYPE, ERR_TYPE}},
        {{ERR_TYPE, ERR_TYPE}, {ERR_TYPE, ERR_TYPE}, {ERR_TYPE, ERR_TYPE}, {ERR_TYPE, ERR_TYPE}}
    };

    // =
    static const Unif tableD[4][4] = {
        //          char    int    float    double
        //   char   char    char   char     char
        //    int   int     int    int      int
        //  float   float   float  float    float
        // double   double  double double   double
        {{CHAR_TYPE, CHAR_TYPE},     {CHAR_TYPE, CHAR_TYPE},     {CHAR_TYPE, CHAR_TYPE},     {CHAR_TYPE, CHAR_TYPE}},
        {{INT_TYPE, INT_TYPE},       {INT_TYPE, INT_TYPE},       {INT_TYPE, INT_TYPE},       {INT_TYPE, INT_TYPE}},
        {{FLOAT_TYPE, FLOAT_TYPE},   {FLOAT_TYPE, FLOAT_TYPE},   {FLOAT_TYPE, FLOAT_TYPE},   {FLOAT_TYPE, FLOAT_TYPE}},
        {{DOUBLE_TYPE, DOUBLE_TYPE}, {DOUBLE_TYPE, DOUBLE_TYPE}, {DOUBLE_TYPE, DOUBLE_TYPE}, {DOUBLE_TYPE, DOUBLE_TYPE}}
    };

    AST* l_ast = operation->children[0];
    Type l_type = l_ast->type;

    AST* r_ast = operation->children[1];
    Type r_type = r_ast->type;
    
    switch(operation->kind){
        case PLUS_NODE: 
        case MINUS_NODE:
        case OVER_NODE:
        case TIMES_NODE:   return tableA[l_type][r_type];

        case LT_NODE:
        case GT_NODE:
        case AND_NODE:
        case OR_NODE:
        case LE_NODE:
        case GE_NODE:
        case EQ_NODE:
        case NE_NODE:     return tableB[l_type][r_type];

        case BW_XOR_NODE:
        case BW_AND_NODE:
        case BW_OR_NODE:
        case BW_LSR_NODE:
        case BW_LSL_NODE:
        case MOD_NODE:    return tableC[l_type][r_type];

        case ASSIGN_NODE: return tableD[l_type][r_type];

        default:
            printf("get_ast_unif: Error");
            exit(EXIT_FAILURE);
    }
}


// Set
AST* set_ast_kind(AST* ast, NodeKind kind){
    ast->kind = kind;
    return ast;
}

AST* set_ast_child(AST* parent, int i, AST* child){
    if(i >= get_ast_length(parent)) return parent;
    parent->children[i] = child;
    child->parent = parent;
    return parent;
}

AST* set_ast_type(AST* ast, Type type){
    ast->type = type;
    return ast;
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

//----------------------------------------------

int nr;

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

*/