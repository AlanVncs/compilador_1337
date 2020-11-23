#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "table.h"
#include "ast.h"

struct scope {
    int id;

    AST** var_decls;
    int var_decls_length;

    AST** func_decls;
    int func_decls_length;

    struct scope* parent;

    struct scope** children;
    int children_length;
};

// Create | Delete
int next_scope_id = 0;
Scope* new_scope() {
    Scope* scope = malloc(sizeof(Scope));

    scope->id = next_scope_id++;
    
    scope->var_decls = NULL;
    scope->var_decls_length = 0;

    scope->func_decls = NULL;
    scope->func_decls_length = 0;

    scope->parent = NULL;

    scope->children = NULL;
    scope->children_length = 0;
    return scope;
}


// Modify
int add_scope(Scope* parent, Scope* child) {
    // Aloca mais espaço caso necessário
    if (parent->children_length % SCOPE_CHILDREN_BLOCK_SIZE == 0) {
        parent->children = realloc(parent->children, (parent->children_length + SCOPE_CHILDREN_BLOCK_SIZE) * sizeof(Scope*));
        if (parent->children == NULL) { printf("add_scope: Could not allocate memory\n"); exit(EXIT_FAILURE); }
    }
    parent->children[parent->children_length] = child;
    parent->children[parent->children_length]->parent = parent;
    return parent->children_length++;
}

int add_scope_var(Scope* scope, AST* var_ast) {
    // Aloca mais espaço se necessário
    if (scope->var_decls_length % SCOPE_DECL_BLOCK_SIZE == 0) {
        scope->var_decls = realloc(scope->var_decls, (SCOPE_DECL_BLOCK_SIZE + scope->var_decls_length) * sizeof(AST*));
        if (scope->var_decls == NULL) { printf("add_scope_var: Could not allocate memory\n"); exit(EXIT_FAILURE); }
    }
    scope->var_decls[scope->var_decls_length] = var_ast;
    return scope->var_decls_length++;
}

int add_scope_func(Scope* scope, AST* func_ast) {
    // Aloca mais espaço se necessário
    if (scope->func_decls_length % SCOPE_DECL_BLOCK_SIZE == 0) {
        scope->func_decls = realloc(scope->func_decls, (SCOPE_DECL_BLOCK_SIZE + scope->func_decls_length) * sizeof(AST*));
        if (scope->func_decls == NULL) { printf("add_scope_func: Could not allocate memory\n"); exit(EXIT_FAILURE); }
    }
    scope->func_decls[scope->func_decls_length] = func_ast;
    return scope->func_decls_length++;
}


// Output
void gen_scope_dot(Scope* scope){
    FILE* scope_file = fopen("table.dot", "w");
    fprintf(scope_file, "digraph {\n");
    fprintf(scope_file, "\tgraph [ordering=\"out\"];\n");
    gen_scope_node_dot(scope, scope_file);
    fprintf(scope_file, "}\n");
	fclose(scope_file);
}

void gen_scope_node_dot(Scope* scope, FILE* scope_file){
    fprintf(scope_file, "\t%d[shape=record label=\n", scope->id);
    fprintf(scope_file, "\t\t<<table border=\"0\">\n");
    fprintf(scope_file, "\t\t\t<tr><td colspan=\"4\"><b>Scope ID: %d</b></td></tr>\n", scope->id);
    fprintf(scope_file, "\t\t\t<tr><td></td></tr>\n");
    if(scope->var_decls_length || scope->func_decls_length) gen_decl_dot(scope, scope_file);
    fprintf(scope_file, "\t\t</table>>\n");
    fprintf(scope_file, "\t];\n");
    // Filhos
    for(int i=0; i<scope->children_length; i++){
        Scope* child = scope->children[i];
        gen_scope_node_dot(child, scope_file);
        fprintf(scope_file, "\t%d -> %d\n", scope->id, child->id);
    }
}

void gen_decl_dot(Scope* scope, FILE* scope_file){
    fprintf(scope_file, "\t\t\t<tr>\n");
    fprintf(scope_file, "\t\t\t\t<td><i>Name</i></td>\n");
    fprintf(scope_file, "\t\t\t\t<td><i>Type</i></td>\n");
    fprintf(scope_file, "\t\t\t\t<td><i>Kind</i></td>\n");
    fprintf(scope_file, "\t\t\t\t<td><i>Line</i></td>\n");
    fprintf(scope_file, "\t\t\t</tr>\n");

    // variables
    for(int i=0; i<scope->var_decls_length; i++){
        AST* ast = scope->var_decls[i];
        fprintf(scope_file, "\t\t\t<tr>\n");
        fprintf(scope_file, "\t\t\t\t<td>%s</td>\n", get_ast_name(ast));
        fprintf(scope_file, "\t\t\t\t<td>%s</td>\n", get_type_str(get_ast_type(ast)));
        fprintf(scope_file, "\t\t\t\t<td>variable</td>\n");
        fprintf(scope_file, "\t\t\t\t<td>%d</td>\n", get_ast_line(ast));
        fprintf(scope_file, "\t\t\t</tr>\n");
    }

    // functions
    for(int i=0; i<scope->func_decls_length; i++){
        AST* ast = scope->func_decls[i];
        fprintf(scope_file, "\t\t\t<tr>\n");
        fprintf(scope_file, "\t\t\t\t<td>%s</td>\n", get_ast_name(ast));
        fprintf(scope_file, "\t\t\t\t<td>%s</td>\n", get_type_str(get_ast_type(ast)));
        fprintf(scope_file, "\t\t\t\t<td>function</td>\n");
        fprintf(scope_file, "\t\t\t\t<td>%d</td>\n", get_ast_line(ast));
        fprintf(scope_file, "\t\t\t</tr>\n");
    }
}


// Get

// typedef enum {
//     VAR_ID,
//     FUNC_ID,
//     STRUCT_ID,
//     ENUM_ID
// } VarType;

// typedef struct {
//     char name[VARIABLE_MAX_SIZE];
//     Type type;
//     int line;
// } VarEntry;

// typedef struct {
//     char name[VARIABLE_MAX_SIZE];
//     Type type;
//     int line;
// } VarEntry;





// void destroy_entry(Entry *entry){
//     if (entry)
//         free(entry);
// }

// void destroy_table(Scope *scope){
//     int entryNum=scope->table_length;
//     // int scopeNum=scope->children_length;
//     for (int i=0; i<entryNum; i++){
//         destroy_entry(scope->table[i]);
//     }
//     free(scope->table);
// }

// void destroy_scope(Scope *scope){
//     if (!scope) return;
//     destroy_table(scope);
//     int scopeNum=scope->children_length;
//     for (int i=0; i<scopeNum; i++)
//         destroy_scope(scope->children[i]);
//     free(scope->children);
//     free(scope);
// }

// int lookupVar(Scope* scope, char* name) {
//     if (scope==NULL) return -1;
//     for (int i = 0; i < scope->table_length; i++) {
//         if (strcmp(scope->table[i]->name, name) == 0) {
//             return i;
//         }
//     }
//     return -1;
// }

// VarLoc get_var_loc(Scope *scope, char *name){
//     int idx;
//     while (idx=lookupVar(scope, name), scope != NULL){
//         if (idx!=-1) return (VarLoc){idx, scope};
//         scope=scope->parent;
//     }
//     return (VarLoc){-1, NULL};
// }

// int lookupVar_outter_scope(Scope *scope, char *name){
//     VarLoc varLoc=get_var_loc(scope, name);
//     return varLoc.varIdx;
// }

// Scope *get_decl_scope(Scope *scope, char *name){
//     VarLoc varLoc=get_var_loc(scope, name);
//     return varLoc.varDeclScope;
// }

// char *get_name(Scope *scope, int i){
//     return scope->table[i]->name;
// }

// Type get_type(Scope *scope, int i){
//     return scope->table[i]->type;
// }

// void printTable(Scope* scope) {
//     if(!scope) return;
//     printf("\nScope ID %d:\n", scope->id);
//     for (int i=0; i<scope->table_length; i++) {
//         Entry* entry = scope->table[i];
//         printf("Entry %d -- name: %s, line: %d, type: %s\n", i, entry->name, entry->line, type2text(entry->type));
//     }
//     printf("\n\n");
// }


// TODO String Table

// TODO Free