#include <stdio.h>
#include <stdlib.h>
#include "type.h"


const char* get_type_str(Type type){
    switch (type){
        case CHAR_TYPE:    return "char";
        case INT_TYPE:     return "int";
        case FLOAT_TYPE:   return "float";
        case DOUBLE_TYPE:  return "double";
        case VOID_TYPE:    return "void";
        case STRUCT_TYPE:  return "struct";
        // case ENUM_TYPE:    return "enum";
        // case TYPEDEF_TYPE: return "typedef";
        case NO_TYPE:      return "noType";
        case ERR_TYPE:     return "errType";
        
        default:
            printf("get_type_str: Type not found: %d\n", type);
            exit(EXIT_FAILURE);
    }
}