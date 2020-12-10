#!/bin/bash

tests="inputs/*"

for file in ${tests}; do
    echo "Running ${file}"
    echo
    ./compiler.bin < ${file}
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
        exit 0
    esac

    clear
done