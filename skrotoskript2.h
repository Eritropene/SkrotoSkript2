#ifndef SKROTOSKRIPT2_H
#define SKROTOSKRIPT2_H

#define STACK_SIZE 256
#define HEAP_SIZE 256
#define MAX_STR_LEN 256

typedef enum { false, true } bool;

typedef enum VT { vnull, vstring, vint, vfloat, vbool } VarType;

typedef struct V {
    VarType type;
    union {
        char* s;
        int i;
        float f;
        bool b;
    };
} Variable;

typedef struct C {
    int length;
    char *body;
    Variable *vars;
    struct C *funcs;
    int *jumps;
} Code;

#endif