all: compile run

compile:
	@bison parser.y
	@flex scanner.l
	@gcc -Wall scanner.c parser.c lib/cg.c lib/type.c lib/ast.c lib/table.c -o compiler.bin

run: compile
	@./compiler.bin < inputs/main.c
	@dot -Tpdf table.dot -Nfontname=Roboto -Nfontsize=10 -o table.pdf
	@dot -Tpdf ast.dot -o ast.pdf
	@cc -o out out.s lib/printint.c
	@./out
	@rm -f parser.c parser.h scanner.c ast.dot table.dot

valgrind:
	@bison parser.y
	@flex scanner.l
	@gcc -Wall -g scanner.c parser.c lib/cg.c lib/type.c lib/ast.c lib/table.c -o compiler.bin
	@valgrind ./compiler.bin < inputs/main.c

test: compile
	@./test.sh

clean:
	@rm -rf compilador.bin parser.c parser.h scanner.c ast.dot ast.pdf table.dot table.pdf out.s out