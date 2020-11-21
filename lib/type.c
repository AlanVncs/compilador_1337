#include <stdio.h>
#include <stdlib.h>
#include "type.h"


const char* get_type_str(Type type){
    switch (type){
        case CHAR_TYPE:   return "char";
        case INT_TYPE:    return "int";
        case FLOAT_TYPE:  return "float";
        case DOUBLE_TYPE: return "double";
        case LABEL_TYPE:  return "label";
        case VOID_TYPE:   return "void";
        case NO_TYPE:     return "noType";
        case ERR_TYPE:    return "errType";
        
        default:
            printf("Type not found: %d\n", type);
            exit(EXIT_FAILURE);
    }
}