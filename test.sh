#!/bin/bash

tests="inputs/*"

for file in ${tests}; do
    echo "Running ${file}"
    echo
    ./compiler.bin < ${file}
    dot -Tpdf table.dot -Nfontname=Roboto -Nfontsize=10 -o table.pdf
	dot -Tpdf ast.dot -o ast.pdf
    rm -f parser.c parser.h scanner.c ast.dot table.dot
    # gcc -S ${file}
	if [[ -e out.s ]]; then
        cc -o out out.s lib/printint.c
        ./out
    fi
    rm -f out out.s
    echo
    echo -n "Press any key to run the next test or ESC exit "
    
    read -s -n1  key

    case $key in $'\e')
        make clean
        exit 0
    esac

    clear
done

make clean