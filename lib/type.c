#include <stdio.h>
#include <stdlib.h>
#include "type.h"

// Get
char* get_type_str(Type type){
    switch (type){
        case CHAR_TYPE:    return "char";
        case INT_TYPE:     return "int";
        case FLOAT_TYPE:   return "float";
        case DOUBLE_TYPE:  return "double";
        case VOID_TYPE:    return "void";
        case NO_TYPE:      return "noType";
        case ERR_TYPE:     return "errType";
        
        default:
            printf("get_type_str: Type not found: %d\n", type);
            exit(EXIT_FAILURE);
    }
}

Type get_type_max(Type type_a, Type type_b){
    if      (type_a > type_b) return type_a;
    else if (type_a < type_b) return type_b;
    else                      return type_a;
}