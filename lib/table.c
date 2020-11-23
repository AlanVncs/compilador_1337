#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "table.h"

typedef enum {
    VAR_ID,
    FUNC_ID,
    STRUCT_ID,
    ENUM_ID
} VarType;

typedef struct {
    char name[VARIABLE_MAX_SIZE];
    Type type;
    VarType var_type;
    int line;
} Entry;

struct scope {
    int id;
    Entry** table;
    int table_length;
    struct scope* parent;
    struct scope** children;
    int children_length;
};


// int next_scope_id = 0;

// typedef struct var_loc {
//     int varIdx;
//     Scope *varDeclScope;
// } VarLoc;

// Scope* createScope() {
//     Scope* scope = malloc(sizeof(Scope));
//     scope->id = next_scope_id++;
//     scope->table = NULL;
//     scope->table_length = 0;
//     scope->parent = NULL;
//     scope->children = NULL;
//     scope->children_length = 0;
//     return scope;
// }

// Entry* createEntry(char* name, Type type, VarType var_type, int line) {
//     Entry* entry = malloc(sizeof(Entry));
//     strcpy(entry->name, name);
//     entry->type = type;
//     entry->var_type = var_type;
//     entry->line = line;
//     return entry;
// }

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

// // Adiciona um escopo filho e retorna sua posição
// int addScope(Scope* parent, Scope* child) {

//     // Aloca mais espaço caso o array de filhos (escopos) esteja cheio
//     if (parent->children_length % SCOPE_BLOCK_SIZE == 0) {
//         parent->children = realloc(parent->children, (parent->children_length + SCOPE_BLOCK_SIZE) * sizeof(Scope*));
//         if (parent->children == NULL) { printf("Could not allocate memory\n"); exit(EXIT_FAILURE); }
//     }

//     parent->children[parent->children_length] = child;
//     parent->children[parent->children_length]->parent = parent;
//     parent->children_length++;
//     return parent->children_length - 1;
// }

// // Adiciona uma nova variável ao escopo e retorna o índice desta variável no escopo
// int addVar(Scope* scope, char* name, Type type, VarType var_type, int line) {

//     // Aloca mais espaço caso a tabela esteja cheia
//     if (scope->table_length % VAR_BLOCK_SIZE == 0) {
//         scope->table = realloc(scope->table, (VAR_BLOCK_SIZE + scope->table_length) * sizeof(Entry*));
//         if (scope->table == NULL) { printf("Could not allocate memory\n"); exit(EXIT_FAILURE); }
//     }

//     scope->table[scope->table_length] = createEntry(name, type, var_type, line);
//     scope->table_length++;
//     return scope->table_length - 1;
// }

// // Retorna o índice de uma variável neste escopo ou -1 se a variável não existir
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

// void printDot(Scope* scope){
//     FILE* scope_file = fopen("table.dot", "w");
//     fprintf(scope_file, "digraph {\n");
//     fprintf(scope_file, "\tgraph [ordering=\"out\"];\n");
//     printDotScope(scope, scope_file);
//     fprintf(scope_file, "}\n");
// 	fclose(scope_file);
// }

// void printDotScope(Scope* scope, FILE* scope_file){

//     // Variáveis do escopo
//     fprintf(scope_file, "\t%d[shape=record label=\n", scope->id);
//     fprintf(scope_file, "\t\t<<table border=\"0\">\n");

//     fprintf(scope_file, "\t\t\t<tr><td colspan=\"4\"><b>Scope ID: %d</b></td></tr>\n", scope->id);
//     fprintf(scope_file, "\t\t\t<tr><td></td></tr>\n");
    
//     if(scope->table_length != 0) printDotTable(scope, scope_file);

//     fprintf(scope_file, "\t\t</table>>\n");
//     fprintf(scope_file, "\t];\n");


//     // Filhos
//     for(int i=0; i<scope->children_length; i++){
//         Scope* child = scope->children[i];
//         printDotScope(child, scope_file);
//         fprintf(scope_file, "\t%d -> %d\n", scope->id, child->id);
//     }
// }

// void printDotTable(Scope* scope, FILE* scope_file){

//     fprintf(scope_file, "\t\t\t<tr>\n");
//     fprintf(scope_file, "\t\t\t\t<td><i>Name</i></td>\n");
//     fprintf(scope_file, "\t\t\t\t<td><i>Type</i></td>\n");
//     fprintf(scope_file, "\t\t\t\t<td><i>Kind</i></td>\n");
//     fprintf(scope_file, "\t\t\t\t<td><i>Line</i></td>\n");
//     fprintf(scope_file, "\t\t\t</tr>\n");

//     for(int i=0; i<scope->table_length; i++){
//         Entry* entry = scope->table[i];
//         fprintf(scope_file, "\t\t\t<tr>\n");
//         fprintf(scope_file, "\t\t\t\t<td>%s</td>\n", entry->name);
//         fprintf(scope_file, "\t\t\t\t<td>%s</td>\n", type2text(entry->type));
//         fprintf(scope_file, "\t\t\t\t<td>%s</td>\n", entry->var_type==VAR_ID?"variable":"function");
//         fprintf(scope_file, "\t\t\t\t<td>%d</td>\n", entry->line);
//         fprintf(scope_file, "\t\t\t</tr>\n");
//     }
// }


// TODO String Table

// TODO Free