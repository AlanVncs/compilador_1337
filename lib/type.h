
#ifndef TYPE_H
#define TYPE_H

typedef enum {
    CHAR_TYPE,
    INT_TYPE,
    FLOAT_TYPE,
    DOUBLE_TYPE,
    LABEL_TYPE,
    VOID_TYPE,
    NO_TYPE,
    ERR_TYPE
    // STRUCT_TYPE,
    // EXPR_TYPE,
} Type;

const char* get_type_str(Type type);

#endif // TYPES_H