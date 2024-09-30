#include <stdio.h>
#include <stdlib.h>
//#include <windows.h>
#include <string.h>
#include <ctype.h>
#include "skrotoskript2.h"
#include "hashmap.h"

HashMap *func_map;

Variable stack[STACK_SIZE];
int sp = 0;
Variable heap[HEAP_SIZE];

Variable EMPTY;

// Variable getoperand(Code*, int*, char);
// Variable operation(Code*, int*, char);
Variable getstr_unchecked(Code*, int*);
Variable getvar_unchecked(Code*, int*);
Variable getoperation_unchecked(Code*, int*);
Variable getstr(Code*, int*);
Variable getvar(Code*, int*);
Variable getoperation(Code*, int*);
Variable getnext(Code*, int*);
Variable run(Code);
Code readfunction(char*);

void error(Code *cd, int errorFlag, int line) {
    printf("\nERROR %d: ", errorFlag);
    char* c;
    int spacing;
    if (cd != NULL)
    {
        if (cd->length <= 7) {
            c = cd->body;
            spacing = line;
        } else if (line > cd->length - 3) {
            c = &(cd->body[cd->length - 7]);
            spacing = 7 - cd->length + line;
        } else if (line > 3) {
            c = &(cd->body[line - 3]);
            spacing = 3;
        } else {
            c = cd->body;
            spacing = line;
        }  
    }
    
    switch (errorFlag) {
        case -1:
            printf("Missing command argument.");
            break;
        case -2:
            printf("Cannot open file.");
            break;
        case -3:
            printf("Stack overflow.");
            break;
        case -4:
            printf("Stack underflow. (Are you missing some parameters?)");
            break;
        case -5:
            printf("Heap index outside heap boundaries.         %.7s [line %d]", c, line);
            break;
        case 1:
            printf("Variable not initialized.                   %.7s [line %d]", c, line);
            break;
        case 2:
            printf("Syntax error.                               %.7s [line %d]", c, line);
            break;
        case 3:
            printf("Incompatible operand type.                  %.7s [line %d]", c, line);
            break;
        case 4:
            printf("Operation incompatible with operand type.   %.7s [line %d]", c, line);
            break;
        case 5:
            printf("Invalid jump.                               %.7s [line %d]", c, line);
            break;
        case 6:
            printf("String too long.                            %.7s [line %d]", c, line);
            break;
        case 7:
            printf("Type mismatch in declaration.               %.7s [line %d]", c, line);
            break;
    }
    printf("\n%*s                                                     ^", spacing, "");
    putchar('\a');
    exit(errorFlag);
}

void initvar(Variable *var) 
{
    memset(var, 0, sizeof(Variable) * 26);
    // 'n' is default newline char
    var[13].type = vstring;
    var[13].s = "\n";
    // 's' is default space char
    var[18].type = vstring;
    var[18].s = " ";
}
void initfunc(Code *func)
{
    memset(func, 0, sizeof(Code) * 26);
}
void initjumps(int *j)
{
    memset(j, -1, sizeof(int) * 26);
}
void initheap()
{
    memset(heap, 0, sizeof(Variable) * HEAP_SIZE);
}

/* read a string from the code */
char readbuffer[MAX_STR_LEN+1];
char* readstr(Code *cd, int *ip)
{
    char c;
    int rp = 0;
    while((c = cd->body[(*ip)++]) != '"')
    {
        readbuffer[rp++] = c;
        if (rp > MAX_STR_LEN) {
            error(cd, 6, (*ip)-1);
        }
    }
    readbuffer[rp] = '\0';
    return readbuffer;
}
/* push var to stack */
void push(Variable x)
{
    if (sp >= STACK_SIZE) {
        error(NULL, -3, 0);
    }
    stack[sp++] = x;
}
/* pop var from stack */
void pop(Variable *x)
{
    if (sp <= 0) {
        error(NULL, -4, 0);
    }
    *x = stack[--sp];
}
/* get from heap */
Variable hget(Code *cd, int *ip, Variable pos) 
{
    if (pos.type != vint) {
        error(cd, 4, (*ip)-1);
    }
    if (pos.i < 0 || pos.i >= HEAP_SIZE) {
        error(cd, -5, (*ip)-1);
    }
    return heap[pos.i];
}
/* write in heap */
void hset(Code *cd, int *ip, Variable pos, Variable value) 
{
    if (pos.type != vint) {
        error(cd, 4, (*ip)-1);
    }
    if (pos.i < 0 || pos.i >= HEAP_SIZE) {
        error(cd, -5, (*ip)-1);
    }
    heap[pos.i] = value;
}

/* sum of two variables */
Variable sum(Code *cd, int *ip, Variable a, Variable b) {
    if (a.type != b.type) error(cd, 3, (*ip)-1);
    Variable ret;
    ret.type = a.type;
    switch (a.type) {
        case vint:
            ret.i = a.i + b.i;
            break;
        case vfloat:
            ret.f = a.f + b.f;
            break;
        default:
            error(cd, 4, (*ip)-1);
    }
    return ret;
}
/* subtraction of two variables */
Variable sub(Code *cd, int *ip, Variable a, Variable b) {
    if (a.type != b.type) error(cd, 3, (*ip)-1);
    Variable ret;
    ret.type = a.type;
    switch (a.type) {
        case vint:
            ret.i = a.i - b.i;
            break;
        case vfloat:
            ret.f = a.f - b.f;
            break;
        default:
            error(cd, 4, (*ip)-1);
    }
    return ret;
}
/* multiplication of two variables */
Variable mul(Code *cd, int *ip, Variable a, Variable b) {
    if (a.type != b.type) error(cd, 3, (*ip)-1);
    Variable ret;
    ret.type = a.type;
    switch (a.type) {
        case vint:
            ret.i = a.i * b.i;
            break;
        case vfloat:
            ret.f = a.f * b.f;
            break;
        default:
            error(cd, 4, (*ip)-1);
    }
    return ret;
}
/* division of two variables */
Variable divv(Code *cd, int *ip, Variable a, Variable b) {
    if (a.type != b.type) error(cd, 3, (*ip)-1);
    Variable ret;
    ret.type = a.type;
    switch (a.type) {
        case vint:
            ret.i = a.i / b.i;
            break;
        case vfloat:
            ret.f = a.f / b.f;
            break;
        default:
            error(cd, 4, (*ip)-1);
    }
    return ret;
}
// /* module of two variables */
// Variable mod(Code *cd, int *ip, Variable a, Variable b) {
//     if (a.type != b.type) error(cd, 3, (*ip)-1);
//     Variable ret;
//     ret.type = a.type;
//     switch (a.type) {
//         case vint:
//             ret.i = a.i % b.i;
//             break;
//         default:
//             error(cd, 4, (*ip)-1);
//     }
//     return ret;
// }

/* logic and between two boolean variables */
Variable and(Code *cd, int *ip, Variable a, Variable b) {
    if (a.type != vbool || b.type != vbool) error(cd, 3, (*ip)-1);
    Variable ret;
    ret.type = vbool;
    ret.b = (a.b == true && b.b == true) ? true : false;
    return ret;
}
/* logic or between two boolean variables */
Variable or(Code *cd, int *ip, Variable a, Variable b) {
    if (a.type != vbool || b.type != vbool) error(cd, 3, (*ip)-1);
    Variable ret;
    ret.type = vbool;
    ret.b = (a.b == true || b.b == true) ? true : false;
    return ret;
}
/* logic not of a boolean variable */
Variable not(Code *cd, int *ip, Variable a) {
    if (a.type != vbool) error(cd, 3, (*ip)-1);
    Variable ret;
    ret.type = vbool;
    ret.b = (a.b == true) ? false : true;
    return ret;
}
/* a > b comparison */
Variable grt(Code *cd, int *ip, Variable a, Variable b) {
    if (a.type == vbool || b.type == vbool || a.type != b.type) error(cd, 3, (*ip)-1);
    Variable ret;
    ret.type = vbool;
    switch (a.type) {
        case vint:
            ret.b = (a.i > b.i) ? true : false;
            break;
        case vfloat:
            ret.b = (a.f > b.f) ? true : false;
            break;
        default:
            error(cd, 4, (*ip)-1);
    }
    return ret;
}
/* a < b comparison */
Variable lss(Code *cd, int *ip, Variable a, Variable b) {
    if (a.type == vbool || b.type == vbool || a.type != b.type) error(cd, 3, (*ip)-1);
    Variable ret;
    ret.type = vbool;
    switch (a.type) {
        case vint:
            ret.b = (a.i < b.i) ? true : false;
            break;
        case vfloat:
            ret.b = (a.f < b.f) ? true : false;
            break;
        default:
            error(cd, 4, (*ip)-1);
    }
    return ret;
}
/* a == b comparison */
Variable eql(Code *cd, int *ip, Variable a, Variable b) {
    if (a.type != b.type) error(cd, 3, (*ip)-1);
    Variable ret;
    ret.type = vbool;
    switch (a.type) {
        case vint:
            ret.b = (a.i == b.i) ? true : false;
            break;
        case vfloat:
            ret.b = (a.f == b.f) ? true : false;
            break;
        case vbool:
            ret.b = (a.b == b.b) ? true : false;
            break;
        default:
            error(cd, 4, (*ip)-1);
    }
    return ret;
}
/* print variable to console */
void print(Variable v)
{
    switch (v.type) {
        case vint:
            printf("%d", v.i);
            break;
        case vfloat:
            printf("%g", v.f);
            break;
        case vstring:
            printf("%s", v.s);
            break;
        case vbool:
            printf(v.b == false ? "False" : "True");
            break;
        case vnull:
            printf("%s", "Null");
            break;
    }
}
/* input */
void input(Code *cd, int *ip, Variable* x) {
    switch (x->type) {
        case vint:
            scanf("%d", &(x->i));
            break;
        case vfloat:
            scanf("%g", &(x->f));
            break;
        case vstring:
            scanf("%s", x->s);
            break;
        default:
            error(cd, 3, (*ip)-1);
    }
    fflush(stdin);
}
/* read from code a string variable (" ") */
Variable getstr_unchecked(Code *cd, int *ip)
{
    Variable v;
    v.type = vstring;
    *ip = *ip + 1;
    v.s = readstr(cd, ip);
    char *str = malloc((strlen(v.s) + 1)*sizeof(char));
    strcpy(str, v.s);
    v.s = str;
    return v;
}
Variable getstr(Code *cd, int *ip)
{
    char c = cd->body[*ip];
    if (c != '"') {
        error(cd, 3, *ip);
    }
    return getstr_unchecked(cd, ip);
}

/* read from code a variable */
Variable getvar_unchecked(Code *cd, int *ip)
{
    char c = cd->body[(*ip)++];
    return cd->vars[c - 'a'];
}
Variable getvar(Code *cd, int *ip)
{
    char c = cd->body[*ip];
    if (islower(c) == 0) {
        error(cd, 3, *ip);
    }
    return getvar_unchecked(cd, ip);
}
/* rewrite a variable -> get the var, get the new value */
void writevar(Code *cd, int *ip, VarType type)
{
    int p = *ip;
    char var = cd->body[(*ip)++];
    if (islower(var) == 0) {
        error(cd, 2, *ip);
        return;
    }

    Variable value = getoperation(cd, ip);
    switch (type + value.type * 5) {
        case (vnull + vnull * 5):
        case (vnull + vint * 5):
        case (vnull + vfloat * 5):
        case (vnull + vbool * 5):
        case (vnull + vstring * 5):

        case (vint + vnull * 5):
        case (vfloat + vnull * 5):
        case (vbool + vnull * 5):
        case (vstring + vnull * 5):
            error(cd, 4, p);
            return;

        case (vint + vint * 5):
            cd->vars[var - 'a'].i = value.i;
            break;
        case (vint + vfloat * 5):
            cd->vars[var - 'a'].i = (int)value.f;
            break;
        case (vint + vbool * 5):
            cd->vars[var - 'a'].i = (int)value.b;
            break;
        case (vint + vstring * 5):
            cd->vars[var - 'a'].i = atoi(value.s);
            break;

        case (vfloat + vint * 5):
            cd->vars[var - 'a'].f = (float)value.i;
            break;
        case (vfloat + vfloat * 5):
            cd->vars[var - 'a'].f = value.f;
            break;
        case (vfloat + vbool * 5):
            cd->vars[var - 'a'].f = (float)value.b;
            break;
        case (vfloat + vstring * 5):
            cd->vars[var - 'a'].f = atof(value.s);
            break;

        case (vbool + vint * 5):
            cd->vars[var - 'a'].b = value.i == 0 ? false : true;
            break;
        case (vbool + vbool * 5):
            cd->vars[var - 'a'].b = value.b;
            break;
        case (vbool + vfloat * 5):
        case (vbool + vstring * 5):
            error(cd, 4, p);
            return;

        case (vstring + vint * 5):
            sprintf(readbuffer, "%d", value.i);
            cd->vars[var - 'a'].s = (char*)malloc((strlen(readbuffer) + 1) * sizeof(char));
            strcpy(cd->vars[var - 'a'].s, readbuffer);
            break;
        case (vstring + vfloat * 5):
            sprintf(readbuffer, "%g", value.f);
            cd->vars[var - 'a'].s = (char*)malloc((strlen(readbuffer) + 1) * sizeof(char));
            strcpy(cd->vars[var - 'a'].s, readbuffer);
            break;
        case (vstring + vbool * 5):
            sprintf(readbuffer, "%d", value.i);
            cd->vars[var - 'a'].s = (char*)malloc((strlen(readbuffer) + 1) * sizeof(char));
            strcpy(cd->vars[var - 'a'].s, readbuffer);
            break;
        case (vstring + vstring * 5):
            cd->vars[var - 'a'].s = (char*)malloc((strlen(value.s) + 1) * sizeof(char));
            strcpy(cd->vars[var - 'a'].s, value.s);
            break;
    }
    cd->vars[var - 'a'].type = type;
}
int getjump(Code *cd, int *ip)
{
    char c = cd->body[(*ip)++];
    int jumpPosition;
    if (c == '"') {
        jumpPosition = atoi(readstr(cd, ip));
    } else if (islower(c)) {
        jumpPosition = cd->jumps[c - 'a'];
    } else {
        error(cd, 2, (*ip)-1);
    }
    if (jumpPosition > cd->length) {
        error(cd, 5, jumpPosition);
    }
    return jumpPosition;
}

/* read from code a function call */
Code getfunc_unchecked(Code *cd, int *ip)
{
    char c = cd->body[(*ip)++];
    return cd->funcs[c - 'a'];
}
Code getfunc(Code *cd, int *ip)
{
    char c = cd->body[*ip];
    if (islower(c) == 0) {
        error(cd, 2, *ip);
    }
    return getfunc_unchecked(cd, ip);
}

/* read either a variable, an operation or a literal */
Variable getnext(Code *cd, int *ip)
{
    char c = cd->body[*ip];
    if (islower(c)) {
        return getvar_unchecked(cd, ip);
    } else if (isupper(c)) {
        return getoperation_unchecked(cd, ip);
    } else if (c == '"') {
        return getstr_unchecked(cd, ip);
    } else {
        error(cd, 2, *ip);
        return EMPTY;
    }
}

/* read from code and compute an operation */
Variable getoperation_unchecked(Code *cd, int *ip)
{
    char c = cd->body[(*ip)++];
    switch (c) {
        case 'Q': // label definition (already done)
            ip++;
            return EMPTY;
        case 'C': // function call
            Code f = getfunc(cd, ip);
            return (f.length == 0) ? EMPTY : run(f);
        case 'J': // jump
            *ip = getjump(cd, ip);
            return EMPTY;
        case 'H': // get from heap in position given
            Variable a = getnext(cd, ip);
            return hget(cd, ip, a);
        case 'K': // logic not
            a = getnext(cd, ip);
            return not(cd, ip, a);
        case 'X': // conditional jump
            a = getnext(cd, ip);
            if (a.type != vbool) error(cd, 3, (*ip)-1);
            int jmp_ip = getjump(cd, ip);
            if (a.b == true) *ip = jmp_ip;
            return EMPTY;
        case 'P': // push to stack
            push(getnext(cd, ip));
            return EMPTY;
        case 'Y': // pop from stack
            c = cd->body[(*ip)++];
            if (islower(c) == 0) error(cd, 2, (*ip)-1);
            pop(&(cd->vars[c - 'a']));
            return EMPTY;
        case 'O': // print
            print(getnext(cd, ip));
            return EMPTY;
        case 'N': // scanf
            c = cd->body[(*ip)++];
            if (islower(c) == 0) error(cd, 2, (*ip)-1);
            input(cd, ip, &(cd->vars[c - 'a']));
            return EMPTY;
        case 'I': // int declaration
            writevar(cd, ip, vint);
            return EMPTY;
        case 'F': // float declaration
            writevar(cd, ip, vfloat);
            return EMPTY;
        case 'B': // boolean declaration
            writevar(cd, ip, vbool);
            return EMPTY;
        case 'S': // string declaration
            writevar(cd, ip, vstring);
            return EMPTY;
        case 'A': // add
            a = getnext(cd, ip);
            Variable b = getnext(cd, ip);
            return sum(cd, ip, a, b);
        case 'Z': // subtract
            a = getnext(cd, ip);
            b = getnext(cd, ip);
            return sub(cd, ip, a, b);
        case 'M': // multiply
            a = getnext(cd, ip);
            b = getnext(cd, ip);
            return mul(cd, ip, a, b);
        case 'D': // divide
            a = getnext(cd, ip);
            b = getnext(cd, ip);
            return divv(cd, ip, a, b);
        case 'T': // logic and
            a = getnext(cd, ip);
            b = getnext(cd, ip);
            return and(cd, ip, a, b);
        case 'V': // logic or
            a = getnext(cd, ip);
            b = getnext(cd, ip);
            return or(cd, ip, a, b);
        case 'G': // greater than
            a = getnext(cd, ip);
            b = getnext(cd, ip);
            return grt(cd, ip, a, b);
        case 'L': // less than
            a = getnext(cd, ip);
            b = getnext(cd, ip);
            return lss(cd, ip, a, b);
        case 'E': // equal to
            a = getnext(cd, ip);
            b = getnext(cd, ip);
            return eql(cd, ip, a, b);
        case 'U':
            a = getnext(cd, ip);
            b = getnext(cd, ip);
            hset(cd, ip, a, b);
            return EMPTY;
        case 'W': // import
            c = cd->body[(*ip)++];
            if (islower(c) == 0) error(cd, 2, (*ip)-1);
            c = cd->body[(*ip)++];
            if (cd->body[(*ip)++] == '"') {
                sprintf(readbuffer, "%s.skr", readstr(cd, ip));
                cd->funcs[c - 'a'] = readfunction(readbuffer);
            } else {
                error(cd, 2, (*ip)-1);
            }
            return EMPTY;
        default:
            error(cd, 2, *ip);
            return EMPTY;
    }
}
Variable getoperation(Code *cd, int *ip)
{
    char c = cd->body[*ip];
    if (isupper(c) == 0) {
        error(cd, 3, *ip);
    }
    return getoperation_unchecked(cd, ip);
}

// /* return a variable or the result of an operation */
// Variable getoperand(Code *cd, int *ip, char x) {
//     if (islower(x)) 
//     {
//         return cd->vars[x - 'a'];
//     } 
//     else if (isupper(x)) 
//     {
//         return operation(cd, ip, x);
//     } 
//     else 
//     {
//         error(cd, 2, (*ip)-1);
//         return EMPTY;
//     }
// }

// /* return the result of an operation */
// Variable operation(Code *cd, int *ip, char c) {
//     int p = *ip;
//     char next = cd->body[(*ip)++];

//     Variable ret = EMPTY;

//     /* call */
//     if (c == 'C') {
//         if (islower(next) == 0) {
//             error(cd, 2, p+1);
//         }
//         Code f = cd->funcs[next - 'a'];
//         return (f.length == 0) ? ret : run(f);
//     }

//     Variable a = getoperand(cd, ip, next);
//     if (c == 'K') {
//         return not(cd, ip, a);
//     } else if (c == 'H') {
//         return hget(cd, ip, a);
//     }

//     next = cd->body[(*ip)++];
//     Variable b = getoperand(cd, ip, next);
//     switch (c) {
//         case 'A':
//             return sum(cd, ip, a, b);
//             break;
//         case 'Z':
//             return sub(cd, ip, a, b);
//             break;
//         case 'M':
//             return mul(cd, ip, a, b);
//             break;
//         case 'D':
//             return divv(cd, ip, a, b);
//             break;
//         // case 'H':
//         //     return mod(cd, ip, a, b);
//         //     break;
//         case 'T':
//             return and(cd, ip, a, b);
//             break;
//         case 'V':
//             return or(cd, ip, a, b);
//             break;
//         case 'G':
//             return grt(cd, ip, a, b);
//             break;
//         case 'L':
//             return lss(cd, ip, a, b);
//             break;
//         case 'E':
//             return eql(cd, ip, a, b);
//             break;

//         default:
//             error(cd, 2, p);
//     }
//     return ret;
// }

/* execute a code ,, retur nthe result */
Variable run(Code cd)
{
    int ip = 0;
    // local variables
    Variable var[26];
    // functions
    Code func[26];

    Variable returnValue = EMPTY;

    initvar(var);
    initfunc(func);

    cd.vars = var;
    cd.funcs = func;

    char c = cd.body[ip];
    // main loop
    while (c != '\0' && c != 'R')
    {
        returnValue = getoperation(&cd, &ip);
    }
    // program end
    if (c == 'R') 
    {
        ip++;
        return getnext(&cd, &ip);
    }
    return returnValue;
}

void readfile(FILE* file, int count, char* buffer) {
    char c;
    int n = 0;
    while ((c = fgetc(file)) != EOF && n < count)
    {
        buffer[n++] = (char) c;
    }
    buffer[n] = '\0';
}
Code readfunction(char* skr_file)
{
    Code* code = search(func_map, skr_file);
    if (code == NULL) {
        FILE* f = fopen(skr_file, "r");
        if (f == NULL) {
            error(NULL, -2, 0);
        }
        code = (Code*)malloc(sizeof(Code));

        /*read file*/
        fseek(f, 0, SEEK_END);
        code->length = ftell(f); // length
        rewind(f);

        code->body = (char*)malloc((code->length+1) * sizeof(char)); // body
        readfile(f, code->length, code->body);
        fclose(f);

        int* jumps = (int*)malloc(26 * sizeof(int));
        for (int i = 0; i<code->length; i++) { // jump labels
            if (code->body[i] == 'Q') {
                char j = code->body[++i];
                if (islower(j)) {
                    jumps[j - 'a'] = i + 1;
                } else {
                    error(code, 2, i);
                }
            }
        }
        code->jumps = jumps;

        insert(func_map, skr_file, code);
    }
    return *code;
}
int main(int argc, char** args) {
    if (argc < 2) {
        error(NULL, -1, 0);
    }

    EMPTY.type = vnull;
    initheap();

    func_map = create_hashmap();

    Code mainCode = readfunction(args[1]);

    //disable buffer
    setbuf(stdout, NULL);

    // push args to stack
    for (int i = 2; i < argc; i++) {
        Variable ar;
        ar.type = vstring;
        ar.s = args[i];
        push(ar);
    }

    /*run code*/
    Variable return_value = run(mainCode);

    printf("\nSkrotoprocess done. Return value: ");
    print(return_value);
    putchar('\n');
    putchar('\n');

    free_hashmap(func_map);

    return EXIT_SUCCESS;
}