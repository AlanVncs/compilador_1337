#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "cg.h"

extern FILE *outFile;

static int freereg[4];
static char *reglist[4]= { "%r8", "%r9", "%r10", "%r11" };


// Set all registers as available
void freeall_registers(void){
    freereg[0]= freereg[1]= freereg[2]= freereg[3]= 1;
}

// Allocate a free register. Return the number of
// the register. Die if no available registers.
static int alloc_register(void){
    for (int i=0; i<4; i++) {
        if (freereg[i]) {
            freereg[i]= 0;
            return(i);
        }
    }
    fprintf(stderr, "Out of registers!\n");
    exit(1);
}

// Return a register to the list of available registers.
// Check to see if it's not already there.
static void free_register(int reg){
    if (freereg[reg] != 0) {
        fprintf(stderr, "Error trying to free register %d\n", reg);
        exit(1);
    }
    freereg[reg]= 1;
}

void genpreamble(){
    freeall_registers();
    fputs(
        ".data\n"
        "msg: .asciz \"tst\\n\"\n"
        ".equ len, . - msg\n"
        ".text\n"
        ".globl main\n"
        ".type main, @function\n"
        "main:\n"
        "\tnop\n",                      // for debugging
    outFile);
}

void gensyscalltest(){
    fputs(
        "\tmov \t$1, %rdi\n"
        "\tlea \tmsg(%rip), %rsi\n"
        "\tmov \t$len, %rdx\n"
        "\tmov \t$1, %rax\n"
        "\tsyscall\n",
    outFile);
}

void genpostamble(){
    fputs(
        "\tmov \t$60, %rax\n"
        "\txor \t%rdi, %rdi\n"
        "\tsyscall\n",
    outFile);
}

void genprologue(){
    fputs(
        "\tpush \t%rbp\n"
        "\tmov \t%rsp, %rbp\n",
    outFile
    );
}

void genepilogue(){
    fputs(
        "\tmov \t%rbp, %rsp\n"
        "\tpop \t%rbp\n"
        "\tret\n",
    outFile
    );
}

void genprintint(int r){
    genepilogue();
    fputs("\tmov \trdi, format\n", outFile);
    fprintf(outFile, "\tmove \trsi, %s\n", reglist[r]);
    fputs("\tcall \tprintf\n", outFile);
    genprologue();
}


// Code gen that execs its childs -----------------------------------

int gen_start(AST *ast){
    int childNum=get_ast_length(ast);

    for (int i=0; i<childNum; i++){
        rec_gen(get_ast_child(ast, i));
    }
    return -1;
}

int gen_cpound(AST *ast){
    int childNum=get_ast_length(ast);
    
    for (int i=0; i<childNum; i++){
        rec_gen(get_ast_child(ast, i));
    }
    return -1;
}

int gen_init_decl(AST *ast){
    int childNum=get_ast_length(ast);

    for (int i=0; i<childNum; i++){
        rec_gen(get_ast_child(ast, i));
    }
    return -1;
}

int gen_expr(AST *ast){
    int r=rec_gen(get_ast_child(ast, 0));
    // if (r>0) free_register(r);
    return r;
}

int gen_arg_list(AST *ast){
    int r=rec_gen(get_ast_child(ast, 0));

    return r;
}

//----------------------------------------
// Code gen that initializes values
int gen_load(int value){
    int r= alloc_register();

    fprintf(outFile, "\tmov \t$%d, %s\n", value, reglist[r]);
    return(r);
}

int gen_int_val(AST *ast){
    int data=get_ast_data(ast);
    int r=gen_load(data);
    // fprintf(outFile, "\tmov %s, %d\n", reglist[reg1], data);

    return r;
}

int gen_var_decl(AST *ast){
    // int data=get_ast_data(ast);
    fprintf(outFile, ".comm %s,4,4\n", get_ast_name(ast));

    return -1;
}

int gen_var_decl_init(AST *ast){
    AST *varNode=get_ast_child(ast, 0);
    AST *datNode=get_ast_child(ast, 1);

    char *varName=get_ast_name(varNode);
    int val=get_ast_data(datNode);

    fprintf(outFile, ".comm %s,4,4\n", varName);
    fprintf(outFile, "\tmov \t$%d, %s(%rip)\n", val, varName);    

    return -1;
}


int gen_var_use(AST *ast){
    int r=alloc_register();
    fprintf(outFile, "\tmov \t%s(%rip), %s\n", get_ast_name(ast), reglist[r]);

    return r;
}
//----------------------------------------
// Code gen that executes arithmetic operations
int gen_add(AST *ast){
    int r0=rec_gen(get_ast_child(ast, 0));
    int r1=rec_gen(get_ast_child(ast, 1));
    fprintf(outFile, "\tadd \t%s, %s\n", reglist[r1], reglist[r0]);
    free_register(r1);

    return r0;
}

int gen_assign(AST *ast){
    int r1=rec_gen(get_ast_child(ast, 1));
    int r0=rec_gen(get_ast_child(ast, 0));
    fprintf(outFile, "\tmov \t%s, %s\n",reglist[r1], reglist[r0]);
    AST *nameNode=get_ast_child(ast, 0);
    fprintf(outFile, "\tmov \t%s, %s(%rip)\n", reglist[r0], get_ast_name(nameNode));

    free_register(r1);
    // free_register(r0);

    return r0;
}

int gen_ret(AST *ast){
    int r=rec_gen(get_ast_child(ast, 0));
    fprintf(outFile, "\tmov \t%s, %rax\n", reglist[r]);
    free_register(r);
    return -1;
}


//----------------------------------------
int gen_func_def(AST *ast){
    // AST *leftTree =get_ast_child(ast, 0);
    // AST *nameNode =get_ast_child(leftTree, 0);
    // AST *paramNode=get_ast_child(leftTree, 1);
    char *name=get_ast_name(ast);
    // int childNum=get_ast_length(paramNode);

    fprintf(outFile, ".text\n.globl %s\n.type %s, @function\n", name, name);
        
        
    fprintf(outFile, "%s:\n", name);
    genprologue();
    rec_gen(get_ast_child(ast, 1));
    genepilogue();
    return -1;
}

// One arg
int gen_call_one(int r, AST *ast){
    
    int res=alloc_register();   
    fprintf(outFile, "\tmov \t%s, %rdi\n", reglist[r]);
    fprintf(outFile, "\tcall \t%s\n", get_ast_name(ast));
    fprintf(outFile, "\tmov \t%rax, %s\n", reglist[res]);

    free_register(r);
    
    return res;
}


int gen_call(AST *ast){
    AST *argList =get_ast_child(ast, 1);
    AST *nameNode=get_ast_child(ast, 0);

    int arg;
    arg=rec_gen(argList);

    return gen_call_one(arg, nameNode);
}

#define trace(msg)
#ifndef trace(msg)
#define trace(msg) fprintf(outFile, "%s\n", msg)
#endif

int rec_gen(AST *ast){
    trace("zinfundeu");
    switch (get_ast_kind(ast)){
        case PROGRAM_START_NODE:    return gen_start(ast);
        case FUNCTION_CALL_NODE:    return gen_call(ast);
        case COMPOUND_STMT_NODE:    return gen_cpound(ast);
        case INT_VAL_NODE:          return gen_int_val(ast);
        case ASSIGN_NODE:           return gen_assign(ast);
        case VAR_DECL_NODE:         return gen_var_decl(ast);
        case VAR_DECL_INIT_NODE:    return gen_var_decl_init(ast);
        case FUNCTION_DEF_NODE:     return gen_func_def(ast);
        case INIT_DECL_LIST_NODE:   return gen_init_decl(ast);
        case RETURN_NODE:           return gen_ret(ast);
        case EXPRESSION_NODE:       return gen_expr(ast);
        case ARGUMENT_LIST_NODE:    return gen_arg_list(ast);
        case VAR_USE_NODE:          return gen_var_use(ast);
        case PLUS_NODE:             return gen_add(ast);
        /* code */
        break;
    
    default:
        break;
    }
        return -1;
}

