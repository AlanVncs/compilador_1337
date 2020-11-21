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
// #include "lib/table.h"

extern int yylex(void);
extern void yyerror(char const* str);
extern void yylex_destroy();
extern int yylineno;
extern char* yytext;

char last_id[129];

AST* root_ast = NULL;
AST* last_param_list = NULL;

Type last_decl_type;
Op last_assign_op;

%}

%code requires {#include "lib/ast.h"}
%define api.value.type {AST*}

%token	IDENTIFIER I_CONSTANT F_CONSTANT STRING_LITERAL FUNC_NAME SIZEOF
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
	IDENTIFIER         { $$=new_ast(NO_TYPE, VAR_USE_NODE, 0); set_ast_name($$, last_id); }
  | constant
  | string
  | '(' expression ')' { $$=$2; }
  | generic_selection
;

constant:
	I_CONSTANT           { $$=new_ast(INT_TYPE, INT_VAL_NODE, atoi(yytext)); }
  | F_CONSTANT           { $$=new_ast(FLOAT_TYPE, FLOAT_VAL_NODE, atof(yytext)); }
  | ENUMERATION_CONSTANT
  ;

enumeration_constant:		/* before it has been defined as such */
    IDENTIFIER
;

string:
    STRING_LITERAL { $$=new_ast(NO_TYPE, STR_VAL_NODE, 0); /* TODO Incluir na tabela e pegar o ID */ }
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
    primary_expression                                   { }
  | postfix_expression '[' expression ']'                { /* Acredito que seja Uso de vetor */ }
  | postfix_expression '(' ')'                           { /* TODO Function call */ }
  | postfix_expression '(' argument_expression_list ')'  { /* TODO Function call */ }
  | postfix_expression '.' IDENTIFIER                    { /* Acesso a um campo de uma struct */ }
  | postfix_expression PTR_OP IDENTIFIER                 { /* Acesso a um campo de uma struct pointer */ }
  | postfix_expression INC_OP                            { /* TODO Retornar valor / Criar Assign node com filhos PLUS_NODE e INT_VAL_NODE(1) */ }
  | postfix_expression DEC_OP                            { /* TODO Retornar valor / Criar Assign node com filhos MINUS_NODE e INT_VAL_NODE(1) */ }
  | '(' type_name ')' '{' initializer_list '}'
  | '(' type_name ')' '{' initializer_list ',' '}'
;

argument_expression_list:
    assignment_expression                               { }
  | argument_expression_list ',' assignment_expression  { }
;

unary_expression:
    postfix_expression              { }
  | INC_OP unary_expression         { /* TODO Criar Assign node com filhos PLUS_NODE e INT_VAL_NODE(1) / Retornar valor */ }
  | DEC_OP unary_expression         { /* TODO Criar Assign node com filhos MINUS_NODE e INT_VAL_NODE(1) / Retornar valor */ }
  | unary_operator cast_expression  { /* TODO Agir de acorodo com o operador*/ }
  | SIZEOF unary_expression
  | SIZEOF '(' type_name ')'
  | ALIGNOF '(' type_name ')'
;

unary_operator:
    '&'     { }
  | '*'     { }
  | '+'     { }
  | '-'     { }
  | '~'     { }
  | '!'     { }
;

cast_expression:
    unary_expression                                                { }
  | '(' type_name ')' cast_expression                               { /* TODO No de conversão*/ }
;

multiplicative_expression:
    cast_expression
  | multiplicative_expression '*' cast_expression                   { $$=new_ast_subtree(NO_TYPE, TIMES_NODE, 2, $1, $3); }
  | multiplicative_expression '/' cast_expression                   { $$=new_ast_subtree(NO_TYPE, OVER_NODE, 2, $1, $3); }
  | multiplicative_expression '%' cast_expression                   { $$=new_ast_subtree(NO_TYPE, MOD_NODE, 2, $1, $3); }
;

additive_expression:
    multiplicative_expression
  | additive_expression '+' multiplicative_expression               { $$=new_ast_subtree(NO_TYPE, PLUS_NODE, 2, $1, $3); }
  | additive_expression '-' multiplicative_expression               { $$=new_ast_subtree(NO_TYPE, MINUS_NODE, 2, $1, $3); }
;

shift_expression:
    additive_expression
  | shift_expression LEFT_OP additive_expression                    { $$=new_ast_subtree(NO_TYPE, BW_LSL_NODE, 2, $1, $3); }
  | shift_expression RIGHT_OP additive_expression                   { $$=new_ast_subtree(NO_TYPE, BW_LSR_NODE, 2, $1, $3); }
;

relational_expression:
    shift_expression
  | relational_expression '<' shift_expression                      { $$=new_ast_subtree(NO_TYPE, LT_NODE, 2, $1, $3); }
  | relational_expression '>' shift_expression                      { $$=new_ast_subtree(NO_TYPE, GT_NODE, 2, $1, $3); }
  | relational_expression LE_OP shift_expression                    { $$=new_ast_subtree(NO_TYPE, LE_NODE, 2, $1, $3); }
  | relational_expression GE_OP shift_expression                    { $$=new_ast_subtree(NO_TYPE, GE_NODE, 2, $1, $3); }
;

equality_expression:
    relational_expression
  | equality_expression EQ_OP relational_expression                 { $$=new_ast_subtree(NO_TYPE, EQ_NODE, 2, $1, $3); }
  | equality_expression NE_OP relational_expression                 { $$=new_ast_subtree(NO_TYPE, NE_NODE, 2, $1, $3); }
;

and_expression:
    equality_expression
  | and_expression '&' equality_expression                          { $$=new_ast_subtree(NO_TYPE, BW_AND_NODE, 2, $1, $3); }
;

exclusive_or_expression:
    and_expression
  | exclusive_or_expression '^' and_expression                      { $$=new_ast_subtree(NO_TYPE, BW_XOR_NODE, 2, $1, $3); }
;

inclusive_or_expression:
    exclusive_or_expression
  | inclusive_or_expression '|' exclusive_or_expression             { $$=new_ast_subtree(NO_TYPE, BW_OR_NODE, 2, $1, $3); }
;

logical_and_expression:
    inclusive_or_expression
  | logical_and_expression AND_OP inclusive_or_expression           { $$=new_ast_subtree(NO_TYPE, AND_NODE, 2, $1, $3); }
;

logical_or_expression:
    logical_and_expression
  | logical_or_expression OR_OP logical_and_expression              { $$=new_ast_subtree(NO_TYPE, OR_NODE, 2, $1, $3); }
;

conditional_expression:
    logical_or_expression
  | logical_or_expression '?' expression ':' conditional_expression { $$=new_ast_subtree(NO_TYPE, TERN_OP_NOPE, 3, $1, $3, $5); }
;

assignment_expression:
    conditional_expression
  | unary_expression assignment_operator assignment_expression      { $$=new_ast_subtree(NO_TYPE, ASSIGN_NODE, 2, $1, $3); /* TODO Agir de acordo com o operador */ }
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
    assignment_expression
  | expression ',' assignment_expression
;

constant_expression:
    conditional_expression	/* with constraints */
;

declaration:
    declaration_specifiers ';'
  | declaration_specifiers init_declarator_list ';' { $$=$2; }
  | static_assert_declaration
;

declaration_specifiers:
    storage_class_specifier declaration_specifiers
  | storage_class_specifier
  | type_specifier declaration_specifiers
  | type_specifier
  | type_qualifier declaration_specifiers
  | type_qualifier
  | function_specifier declaration_specifiers
  | function_specifier
  | alignment_specifier declaration_specifiers
  | alignment_specifier
;

init_declarator_list:
    init_declarator                          { $$=new_ast_subtree(last_decl_type, INIT_DECL_LIST_NODE, 1, $1); }
  | init_declarator_list ',' init_declarator { add_ast_child($1, $3); $$=$1; }
;

init_declarator:
    declarator '=' initializer              { }
  | declarator                              { }
;

storage_class_specifier:
    TYPEDEF	/* identifiers must be flagged as TYPEDEF_NAME */
  | EXTERN
  | STATIC
  | THREAD_LOCAL
  | AUTO
  | REGISTER
;

type_specifier:
    VOID                        { last_decl_type=VOID_TYPE; }
  | CHAR                        { last_decl_type=CHAR_TYPE; }
  | SHORT
  | INT                         { last_decl_type=INT_TYPE; }
  | LONG
  | FLOAT                       { last_decl_type=FLOAT_TYPE; }
  | DOUBLE                      { last_decl_type=DOUBLE_TYPE; }
  | SIGNED
  | UNSIGNED
  | BOOL
  | COMPLEX
  | IMAGINARY	  	/* non-mandated extension */
  | atomic_type_specifier
  | struct_or_union_specifier
  | enum_specifier
  | TYPEDEF_NAME		/* after it has been defined as such */
;

struct_or_union_specifier:
    struct_or_union '{' struct_declaration_list '}'            { }
  | struct_or_union IDENTIFIER '{' struct_declaration_list '}' { }
  | struct_or_union IDENTIFIER
;

struct_or_union: 
    STRUCT                                              { }
  | UNION
;

struct_declaration_list:
    struct_declaration                                  { }
  | struct_declaration_list struct_declaration          { }
;

struct_declaration:
    specifier_qualifier_list ';'	                    { } /* for anonymous struct/union */
  | specifier_qualifier_list struct_declarator_list ';' { }
  | static_assert_declaration
;

specifier_qualifier_list:
    type_specifier specifier_qualifier_list
  | type_specifier
  | type_qualifier specifier_qualifier_list
  | type_qualifier
;

struct_declarator_list:
	  struct_declarator                             { }
	| struct_declarator_list ',' struct_declarator  { }
	;

struct_declarator:
    ':' constant_expression
  | declarator ':' constant_expression
  | declarator                                      { }
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
  | direct_declarator         { $$=$1; if(last_param_list){$$=new_ast_subtree(last_decl_type, FUNCTION_DECL_NODE, 2, $1, last_param_list); last_param_list=NULL;} }
;

direct_declarator:
    IDENTIFIER                                                                 { $$=new_ast(last_decl_type, VAR_DECL_NODE, 0); set_ast_name($$, last_id); }
  | '(' declarator ')'
  | direct_declarator '[' ']'
  | direct_declarator '[' '*' ']'
  | direct_declarator '[' STATIC type_qualifier_list assignment_expression ']'
  | direct_declarator '[' STATIC assignment_expression ']'
  | direct_declarator '[' type_qualifier_list '*' ']'
  | direct_declarator '[' type_qualifier_list STATIC assignment_expression ']'
  | direct_declarator '[' type_qualifier_list assignment_expression ']'
  | direct_declarator '[' type_qualifier_list ']'
  | direct_declarator '[' assignment_expression ']'
  | direct_declarator '(' parameter_type_list ')'                              { $$=$1; last_param_list=$3; }
  | direct_declarator '(' ')'                                                  { $$=$1; last_param_list=new_ast(NO_TYPE, PARAMETER_LIST_NODE, 0); }
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
    parameter_list ',' ELLIPSIS
  | parameter_list
;

parameter_list:
    parameter_declaration                    { $$=new_ast_subtree(NO_TYPE, PARAMETER_LIST_NODE, 1, $1); }
  | parameter_list ',' parameter_declaration { add_ast_child($1, $3); $$=$1;  }
;

parameter_declaration:
    declaration_specifiers declarator           { $$=$2; }
  | declaration_specifiers abstract_declarator
  | declaration_specifiers
;

identifier_list:
    IDENTIFIER                      { }
  | identifier_list ',' IDENTIFIER  { }
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
    IDENTIFIER { $$=new_ast(LABEL_TYPE, VAR_DECL_NODE, 0); set_ast_name($$, last_id);} ':' statement { $$=new_ast_subtree(NO_TYPE, LABEL_DECL_NODE, 2, $2, $4); }
  | CASE constant_expression ':' statement                                                           { $$=new_ast_subtree(NO_TYPE, SWITCH_CASE_NODE, 2, $2, $4); }
  | DEFAULT ':' statement                                                                            { $$=new_ast_subtree(NO_TYPE, SWITCH_DEFAULT_NODE, 1, $3); }
;

compound_statement:
    '{' '}'                 { $$=new_ast(NO_TYPE, COMPOUND_STMT_NODE, 0); }
  | '{' block_item_list '}' { $$=$2; }
;

block_item_list:
    block_item                               { $$=new_ast_subtree(NO_TYPE, COMPOUND_STMT_NODE, 1, $1); }
  | block_item_list block_item               { add_ast_child($1, $2); $$=$1; }
;

block_item:
    declaration
  | statement
;

expression_statement:
    ';'
  | expression ';'                           { $$=$1; }
;

selection_statement:
    IF '(' expression ')' statement ELSE statement { $$=new_ast_subtree(NO_TYPE, IF_NODE, 3, $3, $5, $7); }
  | IF '(' expression ')' statement                { $$=new_ast_subtree(NO_TYPE, IF_NODE, 2, $3, $5); }
  | SWITCH '(' expression ')' statement            { $$=new_ast_subtree(NO_TYPE, SWITCH_NODE, 2, $3, $5); }
;

iteration_statement:
    WHILE '(' expression ')' statement                                         { $$=new_ast_subtree(NO_TYPE, WHILE_NODE, 2, $3, $5); }
  | DO statement WHILE '(' expression ')' ';'                                  { $$=new_ast_subtree(NO_TYPE, DO_WHILE_NODE, 2, $2, $5); }
  | FOR '(' expression_statement expression_statement ')' statement            { $$=new_ast_subtree(NO_TYPE, FOR_NODE, 3, $3, $4, $6); }
  | FOR '(' expression_statement expression_statement expression ')' statement { $$=new_ast_subtree(NO_TYPE, FOR_NODE, 4, $3, $4, $5, $7); }
  | FOR '(' declaration expression_statement ')' statement                     { $$=new_ast_subtree(NO_TYPE, FOR_NODE, 3, $3, $4, $6); }
  | FOR '(' declaration expression_statement expression ')' statement          { $$=new_ast_subtree(NO_TYPE, FOR_NODE, 4, $3, $4, $5, $7); }
;

jump_statement:            
    GOTO IDENTIFIER ';'   { AST* label=new_ast(LABEL_TYPE, VAR_USE_NODE, 0); set_ast_name(label, last_id); $$=new_ast_subtree(NO_TYPE, GOTO_NODE, 1, label); }
  | CONTINUE ';'          { $$=new_ast(NO_TYPE, CONTINUE_NODE, 0); }
  | BREAK ';'             { $$=new_ast(NO_TYPE, BREAK_NODE, 0); }
  | RETURN ';'            { $$=new_ast(NO_TYPE, RETURN_NODE, 0); }
  | RETURN expression ';' { $$=new_ast_subtree(NO_TYPE, RETURN_NODE, 1, $2); }
;

translation_unit:
    external_declaration                  { $$=new_ast_subtree(NO_TYPE, PROGRAM_START_NODE, 1, $1); }
  | translation_unit external_declaration { add_ast_child($1, $2); $$=$1; }
;

external_declaration:
    function_definition                   { }
  | declaration                           { }
;

function_definition:
    declaration_specifiers declarator declaration_list compound_statement
  | declaration_specifiers declarator compound_statement                  { $$=new_ast_subtree(last_decl_type, FUNCTION_DEF_NODE, 2, $2, $3); }
;

declaration_list:
    declaration
  | declaration_list declaration
;

%%

int main(void) {
    if (yyparse() == 0) printf("PARSE SUCCESSFUL!\n");
    gen_ast_dot(root_ast);
    yylex_destroy();
    return 0;
}

int sym_type(char* id){
    // TODO Implementar da maneira correta!!!
    return IDENTIFIER;
}
