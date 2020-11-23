
#ifndef TABLE_H
#define TABLE_H

#define SCOPE_CHILDREN_BLOCK_SIZE 10 // Valor ideal: Número médio de escopos dentro de um escopo qualquer
#define SCOPE_ENTRY_BLOCK_SIZE 20    // Valor ideal: número médio de variáveis dentro de um escopo qualquer

#define VARIABLE_MAX_SIZE 129        // Tamanho máximo de uma variável

#include "type.h"

// typedef struct scope Scope;

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