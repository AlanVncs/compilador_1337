#if !defined(CG_H_)
#define CG_H_

#include "ast.h"

void genpreamble();
void gensyscalltest();
void genpostamble();
int rec_gen(AST *ast);
void freeall_registers(void);

#endif // CG_H_
