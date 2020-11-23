
#ifndef TABLE_H
#define TABLE_H

#define SCOPE_CHILDREN_BLOCK_SIZE 10 // Valor ideal: Número médio de escopos dentro de um escopo qualquer
#define SCOPE_DECL_BLOCK_SIZE 20     // Valor ideal: número médio de variáveis dentro de um escopo qualquer

#include "type.h"

typedef struct scope Scope;

// Create
Scope* new_scope();


// Modify
int add_scope(Scope* parent, Scope* child);
int add_scope_var(Scope* scope, AST* var_ast);
int add_scope_func(Scope* scope, AST* func_ast);


// Output
void gen_scope_dot(Scope* scope);
void gen_scope_node_dot(Scope* scope, FILE* scope_file);
void gen_decl_dot(Scope* scope, FILE* scope_file);



// Scope* createScope();
// Entry* createEntry(char* name, Type type, VarType var_type, int line);
// int addScope(Scope* parent, Scope* child);
// int addVar(Scope* scope, char* name, Type type, VarType var_type, int line);
// int lookupVar(Scope* scope, char* name);
// int lookupVar_outter_scope(Scope *scope, char *name);
// Scope *get_decl_scope(Scope *scope, char *name);
// void printTable(Scope* scope);
// void dotPrintTable(Scope* scope);
// void printDot(Scope* scope);
// void printDotScope(Scope* scope, FILE* file);
// void printDotTable(Scope* scope, FILE* file);
// char *get_name(Scope *, int);
// Type get_type(Scope *scope, int i);
// void destroy_scope(Scope *scope);


#endif