#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "table.h"

struct scope {
    int id;

    AST** table;
    int table_length;

    struct scope* parent;

    struct scope** children;
    int children_length;
};

// Create | Delete
int next_scope_id = 0;
Scope* new_scope() {
    Scope* scope = malloc(sizeof(Scope));

    scope->id = next_scope_id++;
    
    scope->table = NULL;
    scope->table_length = 0;

    scope->parent = NULL;

    scope->children = NULL;
    scope->children_length = 0;
    return scope;
}

Scope* new_child_scope(Scope* parent){
    Scope* child = new_scope();
    add_scope(parent, child);
    return child;
}

void delete_scope(Scope* scope){
    for(int i=0; i<scope->children_length; i++){
        delete_scope(scope->children[i]);
    }
    free(scope->table);
    free(scope->children);
    free(scope);
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

int add_scope_ast(Scope* scope, AST* ast) {
    // Aloca mais espaço se necessário
    if (scope->table_length % SCOPE_DECL_BLOCK_SIZE == 0) {
        scope->table = realloc(scope->table, (SCOPE_DECL_BLOCK_SIZE + scope->table_length) * sizeof(AST*));
        if (scope->table == NULL) { printf("add_scope_var: Could not allocate memory\n"); exit(EXIT_FAILURE); }
    }
    scope->table[scope->table_length] = ast;
    return scope->table_length++;
}

void replace_scope_ast(Scope* scope, AST* new_ast, AST* old_ast){
    int length = scope->table_length;
    for(int i=0; i<length; i++){
        int id = get_ast_id(old_ast);
        if(id == get_ast_id(scope->table[i])){
            scope->table[i] = new_ast;
            return;
        }
    }
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
    if(scope->table_length || scope->table_length) gen_decl_dot(scope, scope_file);
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

    for(int i=0; i<scope->table_length; i++){
        AST* ast = scope->table[i];
        fprintf(scope_file, "\t\t\t<tr>\n");
        fprintf(scope_file, "\t\t\t\t<td>%s</td>\n", get_ast_name(ast));
        fprintf(scope_file, "\t\t\t\t<td>%s</td>\n", get_type_str(get_ast_type(ast)));
        fprintf(scope_file, "\t\t\t\t<td>%s</td>\n", get_var_kind(ast));
        fprintf(scope_file, "\t\t\t\t<td>%d</td>\n", get_ast_line(ast));
        fprintf(scope_file, "\t\t\t</tr>\n");
    }
}

void print_scope(Scope* scope){
    printf("Start scope: %d -------------\n", scope->id);
    for(int i=0; i<scope->table_length; i++){
        print_ast(scope->table[i]);
    }
    printf("--------------- End scope: %d\n\n", scope->id);
}



// Get
AST* lookup_scope_ast(Scope* scope, AST* ast){
    if(!scope) return NULL;
    for(int i=0; i<scope->table_length; i++){
        AST* found_ast = scope->table[i];
        if(strcmp(get_ast_name(ast), get_ast_name(found_ast)) == 0) {
            return found_ast;
        }
    }
    return NULL;
}

AST* lookup_outter_scope_ast(Scope* scope, AST* ast){
    
    if(!scope) return NULL;

    AST* found_ast = lookup_scope_ast(scope, ast);
    if(!found_ast) return lookup_outter_scope_ast(scope->parent, ast);
    return found_ast;
}



// TODO String Table