
#ifndef TYPE_H
#define TYPE_H

typedef enum {
    CHAR_TYPE,
    INT_TYPE,
    FLOAT_TYPE,
    DOUBLE_TYPE,
    VOID_TYPE,
    STRUCT_TYPE,
    ENUM_TYPE,
    TYPEDEF_TYPE,
    NO_TYPE,
    ERR_TYPE
} Type;

const char* get_type_str(Type type);

#endif // TYPES_H