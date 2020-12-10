all: compile run

compile:
	@bison parser.y
	@flex scanner.l
	@gcc -Wall scanner.c parser.c lib/cg.c lib/type.c lib/ast.c lib/table.c -o compiler.bin

run: compile
	@./compiler.bin < inputs/main.c

pdf: run
	@dot -Tpdf table.dot -Nfontname=Roboto -Nfontsize=10 -o table.pdf
	@dot -Tpdf ast.dot -o ast.pdf

valgrind:
	@bison parser.y
	@flex scanner.l
	@gcc -Wall -g scanner.c parser.c lib/cg.c lib/type.c lib/ast.c lib/table.c -o compiler.bin
	@valgrind ./compiler.bin < inputs/main.c

main:
	@gcc -Wall inputs/main.c -o apagar
	@./apagar
	@rm -rf apagar

asm: run
	@cc -o out out.s lib/printint.c
	@./out

clean:
	@rm -rf compilador.bin parser.c parser.h scanner.c compiler.bin ast.dot ast.pdf table.dot table.pdf out.s out out.o

test: compile
	
	@./test.sh