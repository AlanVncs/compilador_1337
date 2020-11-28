%output "parser.c"          // File name of generated parser.
%defines "parser.h"         // Produces a 'parser.h'
%define parse.error verbose // Give proper messages when a syntax error is found.
%define parse.lac full      // Enable LAC to improve syntax error handling.
%define parse.trace

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lib/ast.h"
#include "lib/type.h"
#include "lib/table.h"

extern int yylex(void);
extern void yyerror(char const* str);
extern void yylex_destroy();
extern int yylineno;
extern char* yytext;

char str_to_char(char* str);
AST* build_assign_ast(Op op, AST* l_ast, AST* r_ast);
Scope* build_scope_tree();


char last_id[VARIABLE_MAX_SIZE];

AST* root_ast = NULL;
AST* last_param_list = NULL;

Type last_decl_type;
Op last_assign_op;

%}

%code requires {#include "lib/ast.h"}
%define api.value.type {AST*}

%token	IDENTIFIER C_CONSTANT I_CONSTANT F_CONSTANT STRING_LITERAL FUNC_NAME SIZEOF
%token	PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token	AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token	SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token	XOR_ASSIGN OR_ASSIGN
%token	TYPEDEF_NAME ENUMERATION_CONSTANT

%token	TYPEDEF EXTERN STATIC AUTO REGISTER INLINE
%token	CONST RESTRICT VOLATILE
%token	BOOL CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE VOID
%token	COMPLEX IMAGINARY 
%token	STRUCT UNION ENUM ELLIPSIS

%token	CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%token	ALIGNAS ALIGNOF ATOMIC GENERIC NORETURN STATIC_ASSERT THREAD_LOCAL

%precedence ATOMIC
%precedence '('

%precedence ')'
%precedence ELSE

%start program_start
%%

program_start:
    translation_unit { root_ast = $$; }
;

primary_expression:
    IDENTIFIER         { $$=new_ast(NO_TYPE, last_id, yylineno, VAR_USE_NODE, 0); }
  | constant           { $$=$1; }
  | string             { $$=$1; }
  | '(' expression ')' { $$=$2; }
  | generic_selection
;

constant:
    C_CONSTANT { $$=new_ast(CHAR_TYPE, NULL, yylineno, CHAR_VAL_NODE, str_to_char(yytext)); }
  | I_CONSTANT { $$=new_ast(INT_TYPE, NULL, yylineno, INT_VAL_NODE, atoi(yytext)); }
  | F_CONSTANT { $$=new_ast(FLOAT_TYPE, NULL, yylineno, FLOAT_VAL_NODE, atof(yytext)); }
  | ENUMERATION_CONSTANT 
;

enumeration_constant:		/* before it has been defined as such */
    IDENTIFIER
;

string:
    STRING_LITERAL { $$=new_ast(NO_TYPE, NULL, yylineno, STR_VAL_NODE, 0); /* TODO Incluir na tabela e pegar o ID */ }
  | FUNC_NAME
;

generic_selection:
    GENERIC '(' assignment_expression ',' generic_assoc_list ')'
;

generic_assoc_list:
    generic_association
  | generic_assoc_list ',' generic_association
;

generic_association:
    type_name ':' assignment_expression
  | DEFAULT ':' assignment_expression
;

postfix_expression:
    primary_expression                                   { $$=$1; }
  | postfix_expression '[' expression ']'                { /* Acredito que seja Uso de vetor */ }
  | postfix_expression '(' ')'                           { $$=new_ast_subtree(NO_TYPE, NULL, yylineno, FUNCTION_CALL_NODE, 2, $1, new_ast(NO_TYPE, NULL, yylineno, ARGUMENT_LIST_NODE, 0)); }
  | postfix_expression '(' argument_expression_list ')'  { $$=new_ast_subtree(NO_TYPE, NULL, yylineno, FUNCTION_CALL_NODE, 2, $1, $3); }
  | postfix_expression '.' IDENTIFIER                    { /* Acesso a um campo de uma struct */ }
  | postfix_expression PTR_OP IDENTIFIER                 { /* Acesso a um campo de uma struct pointer */ }
  | postfix_expression INC_OP                            { AST* inc=build_assign_ast(OP_ADD_ASSIGN, $1, new_ast(INT_TYPE, NULL, yylineno, INT_VAL_NODE, 1)); $$=new_ast_subtree(NO_TYPE, NULL, yylineno, MINUS_NODE, 2, inc, new_ast(INT_TYPE, NULL, yylineno, INT_VAL_NODE, 1)); }
  | postfix_expression DEC_OP                            { AST* inc=build_assign_ast(OP_SUB_ASSIGN, $1, new_ast(INT_TYPE, NULL, yylineno, INT_VAL_NODE, 1)); $$=new_ast_subtree(NO_TYPE, NULL, yylineno, PLUS_NODE, 2, inc, new_ast(INT_TYPE, NULL, yylineno, INT_VAL_NODE, 1)); }
  | '(' type_name ')' '{' initializer_list '}'
  | '(' type_name ')' '{' initializer_list ',' '}'
;

argument_expression_list:
    assignment_expression                               { $$=new_ast_subtree(NO_TYPE, NULL, yylineno, ARGUMENT_LIST_NODE, 1, $1); }
  | argument_expression_list ',' assignment_expression  { $$=add_ast_child($1, $3); }
;

unary_expression:
    postfix_expression             { $$=$1; }
  | INC_OP unary_expression        { $$=build_assign_ast(OP_ADD_ASSIGN, $2, new_ast(INT_TYPE, NULL, yylineno, INT_VAL_NODE, 1)); }
  | DEC_OP unary_expression        { $$=build_assign_ast(OP_SUB_ASSIGN, $2, new_ast(INT_TYPE, NULL, yylineno, INT_VAL_NODE, 1)); }
  | unary_operator cast_expression { $$=add_ast_child($1, $2); }
  | SIZEOF unary_expression        { $$=new_ast_subtree(NO_TYPE, NULL, yylineno, SIZEOF_NODE, 1, $2); }
  | SIZEOF '(' type_name ')'       { $$=new_ast(last_decl_type, NULL, yylineno, SIZEOF_NODE, 0); }
  | ALIGNOF '(' type_name ')'      /* Ignorado */
;

unary_operator:
    '&'     { $$=new_ast(NO_TYPE, NULL, yylineno, ADDRESS_NODE, 0); }
  | '*'     { $$=new_ast(NO_TYPE, NULL, yylineno, DEREFERENCE_NODE, 0); }
  | '+'     { $$=new_ast(NO_TYPE, NULL, yylineno, POSITIVE_NODE, 0); }
  | '-'     { $$=new_ast(NO_TYPE, NULL, yylineno, NEGATIVE_NODE, 0); }
  | '~'     { $$=new_ast(NO_TYPE, NULL, yylineno, BW_NOT_NODE, 0); }
  | '!'     { $$=new_ast(NO_TYPE, NULL, yylineno, NOT_NODE, 0); }
;

cast_expression:
    unary_expression                  { $$=$1; }
  | '(' type_name ')' cast_expression { $$=new_ast_subtree(last_decl_type, NULL, yylineno, CAST_NODE, 1, $4); }
;

multiplicative_expression:
    cast_expression                               { $$=$1; }
  | multiplicative_expression '*' cast_expression { $$=new_ast_subtree(NO_TYPE, NULL, yylineno, TIMES_NODE, 2, $1, $3); }
  | multiplicative_expression '/' cast_expression { $$=new_ast_subtree(NO_TYPE, NULL, yylineno, OVER_NODE, 2, $1, $3); }
  | multiplicative_expression '%' cast_expression { $$=new_ast_subtree(NO_TYPE, NULL, yylineno, MOD_NODE, 2, $1, $3); }
;

additive_expression:
    multiplicative_expression                         { $$=$1; }
  | additive_expression '+' multiplicative_expression { $$=new_ast_subtree(NO_TYPE, NULL, yylineno, PLUS_NODE, 2, $1, $3); }
  | additive_expression '-' multiplicative_expression { $$=new_ast_subtree(NO_TYPE, NULL, yylineno, MINUS_NODE, 2, $1, $3); }
;

shift_expression:
    additive_expression                           { $$=$1; }
  | shift_expression LEFT_OP additive_expression  { $$=new_ast_subtree(NO_TYPE, NULL, yylineno, BW_LSL_NODE, 2, $1, $3); }
  | shift_expression RIGHT_OP additive_expression { $$=new_ast_subtree(NO_TYPE, NULL, yylineno, BW_LSR_NODE, 2, $1, $3); }
;

relational_expression:
    shift_expression                             { $$=$1; }
  | relational_expression '<' shift_expression   { $$=new_ast_subtree(NO_TYPE, NULL, yylineno, LT_NODE, 2, $1, $3); }
  | relational_expression '>' shift_expression   { $$=new_ast_subtree(NO_TYPE, NULL, yylineno, GT_NODE, 2, $1, $3); }
  | relational_expression LE_OP shift_expression { $$=new_ast_subtree(NO_TYPE, NULL, yylineno, LE_NODE, 2, $1, $3); }
  | relational_expression GE_OP shift_expression { $$=new_ast_subtree(NO_TYPE, NULL, yylineno, GE_NODE, 2, $1, $3); }
;

equality_expression:
    relational_expression                           { $$=$1; }
  | equality_expression EQ_OP relational_expression { $$=new_ast_subtree(NO_TYPE, NULL, yylineno, EQ_NODE, 2, $1, $3); }
  | equality_expression NE_OP relational_expression { $$=new_ast_subtree(NO_TYPE, NULL, yylineno, NE_NODE, 2, $1, $3); }
;

and_expression:
    equality_expression                    { $$=$1; }
  | and_expression '&' equality_expression { $$=new_ast_subtree(NO_TYPE, NULL, yylineno, BW_AND_NODE, 2, $1, $3); }
;

exclusive_or_expression:
    and_expression                             { $$=$1; }
  | exclusive_or_expression '^' and_expression { $$=new_ast_subtree(NO_TYPE, NULL, yylineno, BW_XOR_NODE, 2, $1, $3); }
;

inclusive_or_expression:
    exclusive_or_expression                             { $$=$1; }
  | inclusive_or_expression '|' exclusive_or_expression { $$=new_ast_subtree(NO_TYPE, NULL, yylineno, BW_OR_NODE, 2, $1, $3); }
;

logical_and_expression:
    inclusive_or_expression                               { $$=$1; }
  | logical_and_expression AND_OP inclusive_or_expression { $$=new_ast_subtree(NO_TYPE, NULL, yylineno, AND_NODE, 2, $1, $3); }
;

logical_or_expression:
    logical_and_expression                             { $$=$1; }
  | logical_or_expression OR_OP logical_and_expression { $$=new_ast_subtree(NO_TYPE, NULL, yylineno, OR_NODE, 2, $1, $3); }
;

conditional_expression:
    logical_or_expression                                           { $$=$1; }
  | logical_or_expression '?' expression ':' conditional_expression { $$=new_ast_subtree(NO_TYPE, NULL, yylineno, TERN_OP_NODE, 3, $1, $3, $5); }
;

assignment_expression:
    conditional_expression                                     { $$=$1; }
  | unary_expression assignment_operator assignment_expression { $$=build_assign_ast(last_assign_op, $1, $3); }
;

assignment_operator:
    '='             { last_assign_op=OP_ASSIGN; } 
  | MUL_ASSIGN      { last_assign_op=OP_MUL_ASSIGN; }
  | DIV_ASSIGN      { last_assign_op=OP_DIV_ASSIGN; }
  | MOD_ASSIGN      { last_assign_op=OP_MOD_ASSIGN; }
  | ADD_ASSIGN      { last_assign_op=OP_ADD_ASSIGN; }
  | SUB_ASSIGN      { last_assign_op=OP_SUB_ASSIGN; }
  | LEFT_ASSIGN     { last_assign_op=OP_LSL_ASSIGN; }
  | RIGHT_ASSIGN    { last_assign_op=OP_LSR_ASSIGN; }
  | AND_ASSIGN      { last_assign_op=OP_AND_ASSIGN; }
  | XOR_ASSIGN      { last_assign_op=OP_XOR_ASSIGN; }
  | OR_ASSIGN       { last_assign_op=OP_OR_ASSIGN; }
;

expression:
    assignment_expression                { $$=new_ast_subtree(NO_TYPE, NULL, yylineno, EXPRESSION_NODE, 1, $1); }
  | expression ',' assignment_expression { $$=add_ast_child($1, $3); }
;

constant_expression:
    conditional_expression	{ $$=$1; }
;

declaration:
    declaration_specifiers ';'                      { $$=$1; }
  | declaration_specifiers init_declarator_list ';' { $$=$2; }
  | static_assert_declaration                       /* Ignorado */
;

declaration_specifiers:
    storage_class_specifier declaration_specifiers { /* TODO */ }
  | storage_class_specifier                        { /* TODO */ }
  | type_specifier declaration_specifiers          /* Ignorado */
  | type_specifier
  | type_qualifier declaration_specifiers          /* Ignorado */
  | type_qualifier                                 /* Ignorado */
  | function_specifier declaration_specifiers      /* Ignorado */
  | function_specifier                             /* Ignorado */
  | alignment_specifier declaration_specifiers     /* Ignorado */
  | alignment_specifier                            /* Ignorado */
;

init_declarator_list:
    init_declarator                          { $$=new_ast_subtree(get_ast_type($1), NULL, yylineno, INIT_DECL_LIST_NODE, 1, $1); }
  | init_declarator_list ',' init_declarator { $$=add_ast_child($1, $3); }
;

init_declarator:
    declarator '=' initializer { $$=new_ast_subtree(get_ast_type($1), NULL, yylineno, VAR_DECL_INIT_NODE, 2, $1, $3); }
  | declarator                 { $$=$1; }
;

storage_class_specifier:
    TYPEDEF	     /* identifiers must be flagged as TYPEDEF_NAME */
  | EXTERN       /* Ignorado */
  | STATIC       /* Ignorado */
  | THREAD_LOCAL /* Ignorado */
  | AUTO         /* Ignorado */
  | REGISTER     /* Ignorado */
;

type_specifier:
    VOID                      { last_decl_type=VOID_TYPE; }
  | CHAR                      { last_decl_type=CHAR_TYPE; }
  | INT                       { last_decl_type=INT_TYPE; }
  | FLOAT                     { last_decl_type=FLOAT_TYPE; }
  | DOUBLE                    { last_decl_type=DOUBLE_TYPE; }
  | struct_or_union_specifier /* Ignorado */
  | enum_specifier            /* Ignorado */
  | TYPEDEF_NAME              /* Ignorado */ /* after it has been defined as such */
  | SHORT                     /* Ignorado */
  | LONG                      /* Ignorado */
  | SIGNED                    /* Ignorado */
  | UNSIGNED                  /* Ignorado */
  | BOOL                      /* Ignorado */
  | COMPLEX                   /* Ignorado */
  | IMAGINARY	  	          /* Ignorado */
  | atomic_type_specifier     /* Ignorado */
;

struct_or_union_specifier:
    struct_or_union '{' struct_declaration_list '}'             /* Ignorado */
  | struct_or_union IDENTIFIER '{' struct_declaration_list '}' /* Ignorado */
  | struct_or_union IDENTIFIER                                 /* Ignorado */
;


struct_or_union: 
    STRUCT
  | UNION  /* Ignorado */
;

struct_declaration_list:
    struct_declaration                         { $$=new_ast_subtree(NO_TYPE, NULL, yylineno, STRUCT_FIELD_LIST_NODE, 1, $1); }
  | struct_declaration_list struct_declaration { $$=add_ast_child($1, $2); }
;

struct_declaration:
    specifier_qualifier_list ';'	                    { $$=$1; } /* for anonymous struct/union */
  | specifier_qualifier_list struct_declarator_list ';' { $$=$2; }
  | static_assert_declaration                           /* Ignorado */
;

specifier_qualifier_list:
    type_specifier specifier_qualifier_list /* Ignorado */
  | type_specifier
  | type_qualifier specifier_qualifier_list /* Ignorado */
  | type_qualifier                          /* Ignorado */
;

struct_declarator_list:
    struct_declarator
  | struct_declarator_list ',' struct_declarator
;

struct_declarator:
    ':' constant_expression            /* Ignorado */
  | declarator ':' constant_expression /* Ignorado */
  | declarator                         { $$=$1; }
;

enum_specifier: /* Declaração de enum */
    ENUM '{' enumerator_list '}'
  | ENUM '{' enumerator_list ',' '}'
  | ENUM IDENTIFIER '{' enumerator_list '}'
  | ENUM IDENTIFIER '{' enumerator_list ',' '}'
  | ENUM IDENTIFIER
;

enumerator_list:
    enumerator
  | enumerator_list ',' enumerator
;

enumerator:	/* identifiers must be flagged as ENUMERATION_CONSTANT */
	enumeration_constant '=' constant_expression
  | enumeration_constant
;

atomic_type_specifier:
    ATOMIC '(' type_name ')'
;

type_qualifier:
    CONST
  | RESTRICT
  | VOLATILE
  | ATOMIC
;

function_specifier:
    INLINE
  | NORETURN
;

alignment_specifier:
    ALIGNAS '(' type_name ')'
  | ALIGNAS '(' constant_expression ')'
;

declarator:
    pointer direct_declarator 
  | direct_declarator         { $$=$1; if(last_param_list){$$=new_ast_subtree(get_ast_type($1), NULL, yylineno, FUNCTION_DECL_NODE, 2, $1, last_param_list); last_param_list=NULL;} }
;

direct_declarator:
    IDENTIFIER                                                                 { $$=new_ast(last_decl_type, last_id, yylineno, VAR_DECL_NODE, 0); }
  | '(' declarator ')'                                                         { $$=$2; /* TODO Analisar melhor para uso de callbacks*/ }
  | direct_declarator '[' ']'                                                  /* Declaração de vetor */
  | direct_declarator '[' '*' ']'                                              /* Ignorado */
  | direct_declarator '[' STATIC type_qualifier_list assignment_expression ']' /* Ignorado */
  | direct_declarator '[' STATIC assignment_expression ']'                     /* Ignorado */
  | direct_declarator '[' type_qualifier_list '*' ']'                          /* Ignorado */
  | direct_declarator '[' type_qualifier_list STATIC assignment_expression ']' /* Ignorado */
  | direct_declarator '[' type_qualifier_list assignment_expression ']'        /* Ignorado */
  | direct_declarator '[' type_qualifier_list ']'                              /* Ignorado */
  | direct_declarator '[' assignment_expression ']'                            /* Declaração de vetor com tamanho definido */
  | direct_declarator '(' parameter_type_list ')'                              { $$=$1; last_param_list=$3; }
  | direct_declarator '(' ')'                                                  { $$=$1; last_param_list=new_ast(NO_TYPE, NULL, yylineno, PARAMETER_LIST_NODE, 0); }
  | direct_declarator '(' identifier_list ')'                      
;

pointer:
    '*' type_qualifier_list pointer
  | '*' type_qualifier_list
  | '*' pointer
  | '*'
;

type_qualifier_list:
    type_qualifier
  | type_qualifier_list type_qualifier
;

parameter_type_list:
    parameter_list ',' ELLIPSIS /* Ignorado */
  | parameter_list              { $$=$1; }
;

parameter_list:
    parameter_declaration                    { $$=new_ast_subtree(NO_TYPE, NULL, yylineno, PARAMETER_LIST_NODE, 1, $1); }
  | parameter_list ',' parameter_declaration { $$=add_ast_child($1, $3);  }
;

parameter_declaration:
    declaration_specifiers declarator          { $$=$2; }
  | declaration_specifiers abstract_declarator
  | declaration_specifiers                     { $$=new_ast(last_decl_type, NULL, yylineno, VAR_DECL_NODE, 0); }
;

identifier_list:
    IDENTIFIER
  | identifier_list ',' IDENTIFIER
;

type_name:
    specifier_qualifier_list abstract_declarator
  | specifier_qualifier_list
;

abstract_declarator:
    pointer direct_abstract_declarator
  | pointer
  | direct_abstract_declarator
;

direct_abstract_declarator:
    '(' abstract_declarator ')'
  | '[' ']'
  | '[' '*' ']'
  | '[' STATIC type_qualifier_list assignment_expression ']'
  | '[' STATIC assignment_expression ']'
  | '[' type_qualifier_list STATIC assignment_expression ']'
  | '[' type_qualifier_list assignment_expression ']'
  | '[' type_qualifier_list ']'
  | '[' assignment_expression ']'
  | direct_abstract_declarator '[' ']'
  | direct_abstract_declarator '[' '*' ']'
  | direct_abstract_declarator '[' STATIC type_qualifier_list assignment_expression ']'
  | direct_abstract_declarator '[' STATIC assignment_expression ']'
  | direct_abstract_declarator '[' type_qualifier_list assignment_expression ']'
  | direct_abstract_declarator '[' type_qualifier_list STATIC assignment_expression ']'
  | direct_abstract_declarator '[' type_qualifier_list ']'
  | direct_abstract_declarator '[' assignment_expression ']'
  | '(' ')'
  | '(' parameter_type_list ')'
  | direct_abstract_declarator '(' ')'
  | direct_abstract_declarator '(' parameter_type_list ')'
;

initializer:
    '{' initializer_list '}'
  | '{' initializer_list ',' '}'
  | assignment_expression
;

initializer_list:
    designation initializer
  | initializer
  | initializer_list ',' designation initializer
  | initializer_list ',' initializer
;

designation:
    designator_list '='
;

designator_list:
    designator
  | designator_list designator
;

designator:
    '[' constant_expression ']'
  | '.' IDENTIFIER
;

static_assert_declaration:
    STATIC_ASSERT '(' constant_expression ',' STRING_LITERAL ')' ';'
;

statement:
    labeled_statement 
  | compound_statement 
  | expression_statement
  | selection_statement
  | iteration_statement
  | jump_statement 
;

labeled_statement:
    IDENTIFIER ':' statement
  | CASE constant_expression ':' statement { $$=new_ast_subtree(NO_TYPE, NULL, yylineno, SWITCH_CASE_NODE, 2, $2, $4); }
  | DEFAULT ':' statement                  { $$=new_ast_subtree(NO_TYPE, NULL, yylineno, SWITCH_DEFAULT_NODE, 1, $3); }
;

compound_statement:
    '{' '}'                 { $$=new_ast(NO_TYPE, NULL, yylineno, COMPOUND_STMT_NODE, 0); }
  | '{' block_item_list '}' { $$=$2; }
;

block_item_list:
    block_item                 { $$=new_ast_subtree(NO_TYPE, NULL, yylineno, COMPOUND_STMT_NODE, 1, $1); }
  | block_item_list block_item { $$=add_ast_child($1, $2); }
;

block_item:
    declaration
  | statement
;

expression_statement:
    ';'            { $$=new_ast(NO_TYPE, NULL, yylineno, EXPRESSION_NODE, 0); }
  | expression ';' { $$=$1; }
;

selection_statement:
    IF '(' expression ')' statement ELSE statement { $$=new_ast_subtree(NO_TYPE, NULL, yylineno, IF_NODE, 3, $3, $5, $7); }
  | IF '(' expression ')' statement                { $$=new_ast_subtree(NO_TYPE, NULL, yylineno, IF_NODE, 2, $3, $5); }
  | SWITCH '(' expression ')' statement            { $$=new_ast_subtree(NO_TYPE, NULL, yylineno, SWITCH_NODE, 2, $3, $5); }
;

iteration_statement:
    WHILE '(' expression ')' statement                                         { $$=new_ast_subtree(NO_TYPE, NULL, yylineno, WHILE_NODE, 2, $3, $5); }
  | DO statement WHILE '(' expression ')' ';'                                  { $$=new_ast_subtree(NO_TYPE, NULL, yylineno, DO_WHILE_NODE, 2, $2, $5); }
  | FOR '(' expression_statement expression_statement ')' statement            { $$=new_ast_subtree(NO_TYPE, NULL, yylineno, FOR_NODE, 3, $3, $4, $6); }
  | FOR '(' expression_statement expression_statement expression ')' statement { $$=new_ast_subtree(NO_TYPE, NULL, yylineno, FOR_NODE, 4, $3, $4, $5, $7); }
  | FOR '(' declaration expression_statement ')' statement                     { $$=new_ast_subtree(NO_TYPE, NULL, yylineno, FOR_NODE, 3, $3, $4, $6); }
  | FOR '(' declaration expression_statement expression ')' statement          { $$=new_ast_subtree(NO_TYPE, NULL, yylineno, FOR_NODE, 4, $3, $4, $5, $7); }
;

jump_statement:            
    GOTO IDENTIFIER ';'
  | CONTINUE ';'          { $$=new_ast(NO_TYPE, NULL, yylineno, CONTINUE_NODE, 0); }
  | BREAK ';'             { $$=new_ast(NO_TYPE, NULL, yylineno, BREAK_NODE, 0); }
  | RETURN ';'            { $$=new_ast(NO_TYPE, NULL, yylineno, RETURN_NODE, 0); }
  | RETURN expression ';' { $$=new_ast_subtree(NO_TYPE, NULL, yylineno, RETURN_NODE, 1, $2); }
;

translation_unit:
    external_declaration                  { $$=new_ast_subtree(NO_TYPE, NULL, yylineno, PROGRAM_START_NODE, 1, $1); }
  | translation_unit external_declaration { $$=add_ast_child($1, $2); }
;

external_declaration:
    function_definition
  | declaration
;

function_definition:
    declaration_specifiers declarator declaration_list compound_statement /* Até agora sem entender */
  | declaration_specifiers declarator compound_statement                  { $$=new_ast_subtree(get_ast_type($2), NULL, get_ast_line($2), FUNCTION_DEF_NODE, 2, $2, $3); }
;

declaration_list:
    declaration
  | declaration_list declaration
;

%%

// TODO Mover lá pra cima
Scope* build_scope_tree();

void build_scope_func_def(AST* func_def, Scope* current_scope);
void build_scope_decl_list(AST* decl_list, Scope* current_scope);
AST* build_scope_function_decl(AST* function_decl, Scope* current_scope);
void build_scope_compound_stmt(AST* compound_stmt, Scope* current_scope, int func_def);
void build_scope_if_stmt(AST* if_stmt, Scope* current_scope);
void build_scope_stmt(AST* stmt, Scope* current_scope);
void build_scope_switch_stmt(AST* switch_stmt, Scope* current_scope);
void build_scope_switch_case_stmt(AST* switch_case_stmt, Scope* current_scope);
void build_scope_switch_default_stmt(AST* switch_default_stmt, Scope* current_scope);
void build_scope_return_stmt(AST* return_stmt, Scope* current_scope);
void build_scope_while_stmt(AST* while_stmt, Scope* current_scope);
void build_scope_do_while_stmt(AST* do_while_stmt, Scope* current_scope);
void build_scope_for_stmt(AST* for_stmt, Scope* current_scope);
void build_scope_var_decl(AST* var_decl, Scope* current_scope);

Type eval(AST* expression, Scope* current_scope);
Type eval_tern_operation(AST* expression, Scope* current_scope);
Type eval_function_call(AST* function_call, Scope* current_scope);
Type eval_var_use(AST* var_use, Scope* current_scope);
Type eval_expression(AST* expression, Scope* current_scope);
Type eval_operation(AST* operation, Scope* current_scope);

AST* build_ast_conv(AST* ast, Type type);
void check_arg_list(AST* function_call, AST* param_list, Scope* scope);

// Erros semânticos
void already_declared(int current_line, char* name, int prev_line);
void redefinition_func(int current_line, char* name, int prev_line);
void conflicting_types(int current_line, char* name, int prev_line);
void redeclared_different(int current_line, char* name, int prev_line);
void undeclared(int current_line, char* name);
void too_x_argmuments(int current_line, char* name, int diff);
void not_a_function(int current_line, char* name, int prev_line);
void incompatible_types_op(int current_line, char* operator, char* l_type, char* r_type);

int main(void) {
    yyparse();
    gen_scope_dot(build_scope_tree());
    gen_ast_dot(root_ast);
    delete_ast(root_ast);
    yylex_destroy();
    return 0;
}

int sym_type(char* id){
    // TODO Checar o tipo do ID e retornar TYPEDEF_NAME | ENUMERATION_CONSTANT | IDENTIFIER
    return IDENTIFIER;
}

char str_to_char(char* str){
    
    int length;
    if(str && (length = strlen(str)) > 2){
        if(length == 3) return str[1];
        if(length == 4){
            switch(str[2]){
                case '0': return '\0';
                case 'b': return '\b';
                case 'f': return '\f';
                case 'n': return '\n';
                case 'r': return '\r';
                case 't': return '\t';
                case 'v': return '\v';
            }
        }
    }
    printf("str_to_char: ????\n"); exit(EXIT_FAILURE);
}

// Recria a árvore de acordo com o operador de atribuição
AST* build_assign_ast(Op op, AST* l_ast, AST* r_ast){
    AST* subtree;
    switch(op){
        case OP_ASSIGN:        return new_ast_subtree(NO_TYPE, NULL, yylineno, ASSIGN_NODE, 2, l_ast, r_ast);                   // =
        case OP_MUL_ASSIGN: subtree = new_ast_subtree(NO_TYPE, NULL, yylineno, TIMES_NODE, 2, clone_ast(l_ast), r_ast); break;  // *=
        case OP_DIV_ASSIGN: subtree = new_ast_subtree(NO_TYPE, NULL, yylineno, OVER_NODE, 2, clone_ast(l_ast), r_ast); break;   // /=
        case OP_MOD_ASSIGN: subtree = new_ast_subtree(NO_TYPE, NULL, yylineno, MOD_NODE, 2, clone_ast(l_ast), r_ast); break;    // %=
        case OP_ADD_ASSIGN: subtree = new_ast_subtree(NO_TYPE, NULL, yylineno, PLUS_NODE, 2, clone_ast(l_ast), r_ast); break;   // +=
        case OP_SUB_ASSIGN: subtree = new_ast_subtree(NO_TYPE, NULL, yylineno, MINUS_NODE, 2, clone_ast(l_ast), r_ast); break;  // -=
        case OP_LSL_ASSIGN: subtree = new_ast_subtree(NO_TYPE, NULL, yylineno, BW_LSL_NODE, 2, clone_ast(l_ast), r_ast); break; // <<=
        case OP_LSR_ASSIGN: subtree = new_ast_subtree(NO_TYPE, NULL, yylineno, BW_LSR_NODE, 2, clone_ast(l_ast), r_ast); break; // >>=
        case OP_AND_ASSIGN: subtree = new_ast_subtree(NO_TYPE, NULL, yylineno, BW_AND_NODE, 2, clone_ast(l_ast), r_ast); break; // &=
        case OP_XOR_ASSIGN: subtree = new_ast_subtree(NO_TYPE, NULL, yylineno, BW_XOR_NODE, 2, clone_ast(l_ast), r_ast); break; // ^=
        case OP_OR_ASSIGN:  subtree = new_ast_subtree(NO_TYPE, NULL, yylineno, BW_OR_NODE, 2, clone_ast(l_ast), r_ast); break;  // |=

        default: 
            printf("build_assign_ast: Line %d\n", yylineno);
            exit(EXIT_FAILURE);
    }
    return new_ast_subtree(NO_TYPE, NULL, yylineno, ASSIGN_NODE, 2, l_ast, subtree);
}

Scope* build_scope_tree(){

    Scope* scope = new_scope();

    int root_children_length = get_ast_length(root_ast);
    for(int i=0; i<root_children_length; i++){
        AST* child = get_ast_child(root_ast, i);
        switch(get_ast_kind(child)){
            case FUNCTION_DEF_NODE:
                build_scope_func_def(child, scope);
                break;
            
            case INIT_DECL_LIST_NODE:
                build_scope_decl_list(child, scope);
                break;

            default: printf("build_scope_tree: Unknown kind\n"); 
        }
    }

    return scope;
}

// init_decl_list ou param_list
void build_scope_decl_list(AST* decl_list, Scope* current_scope){
    int ast_children_length = get_ast_length(decl_list);
    for(int i=0; i<ast_children_length; i++){
        AST* child = get_ast_child(decl_list, i);
        switch(get_ast_kind(child)){
            
            case FUNCTION_DECL_NODE:
                build_scope_function_decl(child, current_scope);
                break;
            
            case VAR_DECL_NODE:
                build_scope_var_decl(child, current_scope);
                break;
            
            case VAR_DECL_INIT_NODE: {
                AST* var_decl = get_ast_child(child, 0);
                AST* init_expr = get_ast_child(child, 1);
                build_scope_var_decl(var_decl, current_scope);
                eval(init_expr, current_scope);
                AST* conv_init_expr = build_ast_conv(init_expr, get_ast_type(var_decl));
                set_ast_child(child, 1, conv_init_expr);
                break;
            }

            default: printf("build_scope_decl_list: Unknown kind\n"); 
        }
    }
}

void build_scope_var_decl(AST* var_decl, Scope* current_scope){
    AST* found_ast = lookup_scope_ast(current_scope, var_decl);
    if(found_ast){
        if(get_ast_kind(found_ast) == VAR_DECL_NODE)
            already_declared(get_ast_line(var_decl), get_ast_name(var_decl), get_ast_line(found_ast));
        else
            redeclared_different(get_ast_line(var_decl), get_ast_name(var_decl), get_ast_line(found_ast));
    }
    add_scope_ast(current_scope, var_decl);
}

// Declara ou redeclara (Se não estiver definida) uma função
AST* build_scope_function_decl(AST* function_decl, Scope* current_scope){

    AST* found_ast = lookup_scope_ast(current_scope, function_decl);
    AST* found_function_decl = found_ast;

    switch(get_ast_kind(found_ast)){
        case NONE:
            add_scope_ast(current_scope, function_decl);
            return function_decl;

        case VAR_DECL_NODE:
            redeclared_different(get_ast_line(function_decl), get_ast_name(function_decl), get_ast_line(found_ast));

        case FUNCTION_DEF_NODE:
            found_function_decl = get_ast_child(found_ast, 0);
            break;

        case FUNCTION_DECL_NODE: 
            replace_scope_ast(current_scope, function_decl, found_function_decl);
            found_ast = function_decl;
            break;

        default: printf("build_scope_function_decl: Error\n"); exit(EXIT_FAILURE);
    }

    if(compare_func_decl(function_decl, found_function_decl))
        conflicting_types(get_ast_line(function_decl), get_ast_name(function_decl), get_ast_line(found_function_decl));
    
    return found_ast;
}

void build_scope_func_def(AST* function_def, Scope* current_scope){

    // Declaração
    AST* function_decl = get_ast_child(function_def, 0);
    AST* found_ast = build_scope_function_decl(function_decl, current_scope);
    NodeKind found_kind = get_ast_kind(found_ast);

    if(found_ast){    
        if(found_kind == FUNCTION_DEF_NODE){
            redefinition_func(get_ast_line(function_def), get_ast_name(function_def), get_ast_line(found_ast));
        }
        replace_scope_ast(current_scope, function_def, found_ast);
    }
    

    // Compound statement
    current_scope = new_child_scope(current_scope);
    AST* param_list = get_ast_child(function_decl, 1);
    build_scope_decl_list(param_list, current_scope);
    

    AST* compound_stmt = get_ast_child(function_def, 1);
    build_scope_compound_stmt(compound_stmt, current_scope, 1);
    
}

void build_scope_stmt(AST* stmt, Scope* current_scope){
    
    if(!stmt) return;

    NodeKind stmt_kind = get_ast_kind(stmt);
    switch(stmt_kind){

        case COMPOUND_STMT_NODE:
            build_scope_compound_stmt(stmt, current_scope, 0);
            break;

        case IF_NODE:
            build_scope_if_stmt(stmt, current_scope);
            break;

        case SWITCH_NODE:
            build_scope_switch_stmt(stmt, current_scope);
            break;

        case SWITCH_CASE_NODE:
            build_scope_switch_case_stmt(stmt, current_scope);
            break;

        case SWITCH_DEFAULT_NODE:
            build_scope_switch_default_stmt(stmt, current_scope);
            break;
        
        case CONTINUE_NODE: break;

        case BREAK_NODE: break;

        case RETURN_NODE:
            build_scope_return_stmt(stmt, current_scope);
            break;

        case WHILE_NODE:
            build_scope_while_stmt(stmt, current_scope);
            break;

        case DO_WHILE_NODE:
            build_scope_do_while_stmt(stmt, current_scope);
            break;

        case FOR_NODE:
            build_scope_for_stmt(stmt, current_scope);
            break;
        
        case EXPRESSION_NODE:
            eval(stmt, current_scope);
            return;

        default: printf("build_scope_stmt: Error\n");
    }
 
}

void build_scope_compound_stmt(AST* compound_stmt, Scope* current_scope, int func_def){
    
    // Se é um compound de uma função, então o escopo já foi aberto
    if(!func_def) current_scope = new_child_scope(current_scope);

    int length = get_ast_length(compound_stmt);
    for(int i=0; i<length; i++){
        AST* child_ast = get_ast_child(compound_stmt, i);
        NodeKind child_ast_kind = get_ast_kind(child_ast);
        switch(child_ast_kind){
            // Declaration
            case INIT_DECL_LIST_NODE:
                build_scope_decl_list(child_ast, current_scope);
                break;

            // Statement
            default: build_scope_stmt(child_ast, current_scope);
        }
    }
}

void build_scope_return_stmt(AST* return_stmt, Scope* current_scope){
    AST* return_expr = get_ast_child(return_stmt, 0);
    eval(return_expr, current_scope);
    Type function_def_type = get_function_def_type(return_stmt);
    set_ast_child(return_stmt, 0, build_ast_conv(return_expr, function_def_type));
}

void build_scope_if_stmt(AST* if_stmt, Scope* current_scope){
    AST* test_expr = get_ast_child(if_stmt, 0);
    AST* then_stmt = get_ast_child(if_stmt, 1);
    AST* else_stmt = get_ast_child(if_stmt, 2);
    eval(test_expr, current_scope);
    set_ast_child(if_stmt, 0, build_ast_conv(test_expr, INT_TYPE));

    build_scope_stmt(then_stmt, current_scope);
    build_scope_stmt(else_stmt, current_scope);
}

void build_scope_switch_stmt(AST* switch_stmt, Scope* current_scope){
    AST* test_expr = get_ast_child(switch_stmt, 0);
    AST* stmt = get_ast_child(switch_stmt, 1);

    eval(test_expr, current_scope);
    set_ast_child(switch_stmt, 0, build_ast_conv(test_expr, INT_TYPE));

    build_scope_stmt(stmt, current_scope);
}

void build_scope_switch_case_stmt(AST* switch_case_stmt, Scope* current_scope){
    AST* constant_expression = get_ast_child(switch_case_stmt, 0);
    eval(constant_expression, current_scope);
    AST* stmt = get_ast_child(switch_case_stmt, 1);
    build_scope_stmt(stmt, current_scope);
}

void build_scope_switch_default_stmt(AST* switch_default_stmt, Scope* current_scope){
    AST* stmt = get_ast_child(switch_default_stmt, 0);
    build_scope_stmt(stmt, current_scope);
}

void build_scope_while_stmt(AST* while_stmt, Scope* current_scope){

    AST* test_expr = get_ast_child(while_stmt, 0);
    eval(test_expr, current_scope);
    set_ast_child(while_stmt, 0, build_ast_conv(test_expr, INT_TYPE));

    AST* stmt = get_ast_child(while_stmt, 1);
    build_scope_stmt(stmt, current_scope);
}

void build_scope_do_while_stmt(AST* do_while_stmt, Scope* current_scope){

    AST* stmt = get_ast_child(do_while_stmt, 0);
    build_scope_stmt(stmt, current_scope);

    AST* test_expr = get_ast_child(do_while_stmt, 1);
    eval(test_expr, current_scope);
    set_ast_child(do_while_stmt, 1, build_ast_conv(test_expr, INT_TYPE));
}

void build_scope_for_stmt(AST* for_stmt, Scope* current_scope){
    
    AST* child_0 = get_ast_child(for_stmt, 0);
    AST* child_1 = get_ast_child(for_stmt, 1);
    AST* child_2 = get_ast_child(for_stmt, 2);
    AST* child_3 = get_ast_child(for_stmt, 3);

    
    if(get_ast_kind(child_0) == INIT_DECL_LIST_NODE){

        current_scope = new_child_scope(current_scope);
        build_scope_decl_list(child_0, current_scope);

        if(child_3){ // (declaration expression_statement expression) statement
            eval(child_1, current_scope);
            eval(child_2, current_scope);
            build_scope_stmt(child_3, current_scope);
        }
        else{ // (declaration expression_statement) statement
            eval(child_1, current_scope);
            build_scope_stmt(child_2, current_scope);
        }
    }
    else{
        eval(child_0, current_scope);
        eval(child_1, current_scope);

        if(child_3){ // (expression_statement expression_statement expression) statement
            eval(child_2, current_scope);
            build_scope_stmt(child_3, current_scope);
        }
        else{ // (expression_statement expression_statement) statement
            build_scope_stmt(child_2, current_scope);
        }
    }
    set_ast_child(for_stmt, 1, build_ast_conv(child_1, INT_TYPE));
}






Type eval(AST* expression, Scope* current_scope){

    /* case CAST_NODE: */
    /* case ADDRESS_NODE: break;     // Implementar junto com ponteiros */
    /* case DEREFERENCE_NODE: break; // Implementar junto com ponteiros */
    /* case POSITIVE_NODE: break; */
    /* case NEGATIVE_NODE: break; */
    /* case BW_NOT_NODE: break; */
    /* case NOT_NODE: break; */
    /* case SIZEOF_NODE: break; */

    if(!expression) return ERR_TYPE;

    // TODO Expression list
    NodeKind kind = get_ast_kind(expression);
    switch(kind){
        case TERN_OP_NODE: return eval_tern_operation(expression, current_scope);
            
        case FUNCTION_CALL_NODE: return eval_function_call(expression, current_scope);

        case VAR_USE_NODE: return eval_var_use(expression, current_scope);

        case EXPRESSION_NODE: eval_expression(expression, current_scope);

        case INT_VAL_NODE:
        case CHAR_VAL_NODE:
        case FLOAT_VAL_NODE: return get_ast_type(expression);
        
        default: eval_operation(expression, current_scope); return get_ast_type(expression);
    }   
}

Type eval_tern_operation(AST* tern_operation, Scope* current_scope){
    
    AST* test_expr = get_ast_child(tern_operation, 0);
    AST* if_expr = get_ast_child(tern_operation, 1);
    AST* else_expr = get_ast_child(tern_operation, 2);

    eval(test_expr, current_scope);
    Type if_expr_type = eval(if_expr, current_scope);
    Type else_expr_type = eval(else_expr, current_scope);

    Type max_type = get_type_max(if_expr_type, else_expr_type);
    
    // Converte as expressões se necessário
    set_ast_child(tern_operation, 0, build_ast_conv(test_expr, INT_TYPE));
    set_ast_child(tern_operation, 1, build_ast_conv(if_expr, max_type));
    set_ast_child(tern_operation, 2, build_ast_conv(else_expr, max_type));

    set_ast_type(tern_operation, max_type);

    return max_type;
}

Type eval_function_call(AST* function_call, Scope* current_scope){

    AST* var_use = get_ast_child(function_call, 0);

    AST* found_ast = lookup_outter_scope_ast(current_scope, var_use);
    Type found_type = get_ast_type(found_ast);

    // Função não existe
    if(!found_ast) undeclared(get_ast_line(function_call), get_ast_name(function_call));

    // Variável sendo usada como função
    if(get_ast_kind(found_ast) == VAR_DECL_NODE) not_a_function(get_ast_line(function_call), get_ast_name(function_call), get_ast_line(found_ast));
    
    AST* found_function_decl = found_ast;
    if(get_ast_kind(found_ast) == FUNCTION_DEF_NODE) found_function_decl = get_ast_child(found_ast, 0);

    AST* param_list = get_ast_child(found_function_decl, 1);
    check_arg_list(function_call, param_list, current_scope);

    set_ast_type(var_use, found_type);
    set_ast_type(function_call, found_type);
    return found_type;
}

Type eval_var_use(AST* var_use, Scope* current_scope){
    AST* found_ast = lookup_outter_scope_ast(current_scope, var_use);
    if(!found_ast) undeclared(get_ast_line(var_use), get_ast_name(var_use));
    Type found_type = get_ast_type(found_ast);
    set_ast_type(var_use, found_type);
    return found_type;
}

Type eval_expression(AST* expression, Scope* current_scope){
    int length = get_ast_length(expression);
    if(!length) return NO_TYPE;
    for(int i=0; i<length; i++){
        eval(get_ast_child(expression, i), current_scope);
    }
    AST* last_child = get_ast_child(expression, length-1);
    set_ast_type(expression, get_ast_type(last_child));
    return get_ast_type(last_child);
}

// Operações com dois operandos (+ - * / % ...)
Type eval_operation(AST* operation, Scope* current_scope){

    AST* l_ast = get_ast_child(operation, 0);
    AST* r_ast = get_ast_child(operation, 1);

    eval(l_ast, current_scope);
    eval(r_ast, current_scope);
    
    Unif unif = get_ast_unif(operation);
    if(unif.operation_type == ERR_TYPE) incompatible_types_op(get_ast_line(operation), get_op_str(get_ast_op(operation)), get_type_str(get_ast_type(l_ast)), get_type_str(get_ast_type(r_ast)));

    set_ast_child(operation, 0, build_ast_conv(l_ast, unif.conv_type));
    set_ast_child(operation, 1, build_ast_conv(r_ast, unif.conv_type));

    set_ast_type(operation, unif.operation_type);

    return unif.operation_type;
}

// Constrói, quando necessário, o nó de conversão
AST* build_ast_conv(AST* ast, Type type){
    Type ast_type = get_ast_type(ast);
    if(ast_type == type || ast_type == NO_TYPE) return ast;
    return new_ast_subtree(type, NULL, get_ast_line(ast), get_conv_node(ast_type, type), 1, ast);
}

// Verifica a compatibilidade dos argumentos e parâmetros de uma função
// Constrói os node de conversão para os argumentos
void check_arg_list(AST* function_call, AST* param_list, Scope* scope){

    AST* arg_list = get_ast_child(function_call, 1);

    // Quantidade de argumentos / parâmetros
    int arg_length   = get_ast_length(arg_list);
    int param_length = get_ast_length(param_list);

    // Quantidade de argumentos diferente da quantidade de parâmetros
    if(arg_length != param_length){
        char* function_name = get_ast_name(function_call);
        too_x_argmuments(get_ast_line(arg_list), function_name, arg_length - param_length);
    }

    // Cria nos de conversão para os argumentos da função
    for(int i=0; i<arg_length; i++){
        AST* arg   = get_ast_child(arg_list, i);
        AST* param = get_ast_child(param_list, i);
        eval(arg, scope);
        set_ast_child(arg_list, i, build_ast_conv(arg, get_ast_type(param)));
    }
}


// Erros semânticos
void already_declared(int current_line, char* name, int prev_line){
    printf("SEMANTIC ERROR (%d): variable '%s' already declared at line %d.\n", current_line, name, prev_line);
    exit(0);
}

void redefinition_func(int current_line, char* name, int prev_line){
    printf("SEMANTIC ERROR (%d): redefinition of ‘%s’ (previous definition of '%s' at line %d).\n", current_line, name, name, prev_line);
    exit(0);
}

void conflicting_types(int current_line, char* name, int prev_line){
    printf("SEMANTIC ERROR (%d): conflicting types for '%s' (previous declaration of '%s' at line %d).\n", current_line, name, name, prev_line);
    exit(0);
}

void redeclared_different(int current_line, char* name, int prev_line){
    printf("SEMANTIC ERROR (%d): '%s' redeclared as different kind of symbol (previous declaration of '%s' at line %d).\n", current_line, name, name, prev_line);
    exit(0);
}

void undeclared(int current_line, char* name){
    printf("SEMANTIC ERROR (%d): '%s' was not declared.\n", current_line, name);
    exit(0);
}

void too_x_argmuments(int current_line, char* name, int diff){
    if(diff > 0) printf("SEMANTIC ERROR (%d): too many arguments to function %s.\n", current_line, name);
    else         printf("SEMANTIC ERROR (%d): too few arguments to function %s.\n", current_line, name);
    exit(0);
}

void not_a_function(int current_line, char* name, int prev_line){
    printf("SEMANTIC ERROR (%d): called object ‘%s’ is not a function or function pointer (previous declaration of '%s' at line %d).\n", current_line, name, name, prev_line);
    exit(0);
}

void incompatible_types_op(int current_line, char* operator, char* l_type, char* r_type){
    printf("SEMANTIC ERROR (%d): incompatible types for operator '%s', LHS is '%s' and RHS is '%s'.\n", current_line, operator, l_type, r_type);
    exit(0);
}

