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

bool has_only_one_operand(char c) 
{
    //                           a,    b,    c,   d,    e,    f,    g,    h,    i,    j,   k,   l,    m,    n,   o,   p,   q,   r,   s,    t,    u,    v,    w,    x,    y,   z
    const bool lookup_table[] = {false,false,true,false,false,false,false,false,false,true,true,false,false,true,true,true,true,true,false,false,false,false,false,false,true,false};
    return lookup_table[c - 'A'];
}
int operation_type(char c)
{
    /*
        0 - 2-operand
        1 - 1-operand
        2 - 1-operand var output
    */
    //                           a,    b,    c,   d,    e,    f,    g,    h,    i,    j,   k,   l,    m,    n,   o,   p,   q,   r,   s,    t,    u,    v,    w,    x,    y,   z
    const int lookup_table[] = {0,false,true,false,false,false,false,false,false,true,true,false,false,true,true,true,true,true,false,false,false,false,false,false,true,false};
    return lookup_table[c - 'A'];
}

#endif