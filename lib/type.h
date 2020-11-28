
#ifndef TYPE_H
#define TYPE_H

// Não modificar a ordem dos tipos char, int, float e double!!!
typedef enum {
    CHAR_TYPE,
    INT_TYPE,
    FLOAT_TYPE,
    DOUBLE_TYPE,
    VOID_TYPE,
    NO_TYPE,
    ERR_TYPE
} Type;

// Get
char* get_type_str(Type type);
Type get_type_max(Type type_a, Type type_b);

#endif // TYPES_H