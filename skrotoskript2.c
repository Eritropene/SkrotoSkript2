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

const char NEWLINE[2] = "\n";
const char SPACE[2] = " ";

Variable getoperand(Code*, int*, char);
Variable operation(Code*, int*, char);
Variable run(Code);
Code readfunction(const char*);

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
    printf("\n% *c                                                     ^", spacing, ' ');
    putchar('\a');
    exit(errorFlag);
}

void initvar(Variable *var) 
{
    memset(var, 0, sizeof(Variable) * 26);
    // 'n' is default newline char
    var[13].type = vstring;
    var[13].s = NEWLINE;
    // 's' is default space char
    var[18].type = vstring;
    var[18].s = SPACE;
}
void initfunc(Code *func)
{
    memset(func, 0, sizeof(Code) * 26);
}
void initjumps(int *j)
{
    memset(j, -1, sizeof(int) * 26);
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

void push(Variable x)
{
    if (sp >= STACK_SIZE) {
        error(NULL, -3, 0);
    }
    stack[sp++] = x;
}
void pop(Variable *x)
{
    if (sp <= 0) {
        error(NULL, -4, 0);
    }
    *x = stack[--sp];
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
/* module of two variables */
Variable mod(Code *cd, int *ip, Variable a, Variable b) {
    if (a.type != b.type) error(cd, 3, (*ip)-1);
    Variable ret;
    ret.type = a.type;
    switch (a.type) {
        case vint:
            ret.i = a.i % b.i;
            break;
        default:
            error(cd, 4, (*ip)-1);
    }
    return ret;
}

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
            ret.b == (a.b == b.b) ? true : false;
            break;
        default:
            error(cd, 4, (*ip)-1);
    }
    return ret;
}

/* return a variable or the result of an operation */
Variable getoperand(Code *cd, int *ip, char x) {
    if (islower(x)) 
    {
        return cd->vars[x - 'a'];
    } 
    else if (isupper(x)) 
    {
        return operation(cd, ip, x);
    } 
    else 
    {
        error(cd, 2, (*ip)-1);
        Variable a;
        return a;
    }
}

/* return the result of an operation */
Variable operation(Code *cd, int *ip, char c) {
    int p = *ip;
    char next = cd->body[(*ip)++];

    Variable ret;
    ret.type = vnull;
    /* call */
    if (c == 'C') {
        if (islower(next) == 0) {
            error(cd, 2, p+1);
        }
        Code f = cd->funcs[next - 'a'];
        return (f.length == 0) ? ret : (Variable)run(f);
    }

    Variable a = getoperand(cd, ip, next);
    if (c == 'K') {
        return not(cd, ip, a);
    }

    next = cd->body[(*ip)++];
    Variable b = getoperand(cd, ip, next);
    switch (c) {
        case 'A':
            return sum(cd, ip, a, b);
            break;
        case 'Z':
            return sub(cd, ip, a, b);
            break;
        case 'M':
            return mul(cd, ip, a, b);
            break;
        case 'D':
            return divv(cd, ip, a, b);
            break;
        case 'H':
            return mod(cd, ip, a, b);
            break;
        case 'T':
            return and(cd, ip, a, b);
            break;
        case 'V':
            return or(cd, ip, a, b);
            break;
        case 'G':
            return grt(cd, ip, a, b);
            break;
        case 'L':
            return lss(cd, ip, a, b);
            break;
        case 'E':
            return eql(cd, ip, a, b);
            break;

        default:
            error(cd, 2, p);
    }
    return ret;
}

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

Variable run(Code cd)
{
    int ip = 0;
    // local variables
    Variable var[26];
    // functions
    Code func[26];

    Variable returnValue;
    returnValue.type = vnull;

    initvar(var);
    initfunc(func);

    cd.vars = var;
    cd.funcs = func;

    char c;
    char x;
    while ((c = cd.body[ip++]) != '\0') {
        switch (c) {
            /* variable declaration */
            case 'I':
                // get the Variable
                x = cd.body[ip++];
                if (islower(x) == 0) {
                    error(&cd, 2, ip-1);
                }

                // look ahead
                c = cd.body[ip++];
                if (c == '"') {
                    var[x - 'a'].i = atoi(readstr(&cd, &ip));
                    var[x - 'a'].type = vint;
                } else {
                    Variable v = getoperand(&cd, &ip, c);
                    var[x - 'a'].type = vint;
                    switch (v.type) {
                        case vint:
                            var[x - 'a'].i = v.i;
                            break;
                        case vfloat:
                            var[x - 'a'].i = (int)v.f;
                            break;
                        case vbool:
                            var[x - 'a'].i = (int)v.b;
                            break;
                        case vstring:
                            var[x - 'a'].i = atoi(v.s);
                            break;
                        default:
                            error(&cd, 7, ip-1);
                    }
                }
                break;

            case 'F':
                // get the Variable
                x = cd.body[ip++];
                if (islower(x) == 0) {
                    error(&cd, 2, ip-1);
                }

                // look ahead
                c = cd.body[ip++];
                if (c == '"') {
                    var[x - 'a'].f = atof(readstr(&cd, &ip));
                    var[x - 'a'].type = vfloat;
                } else {
                    Variable v = getoperand(&cd, &ip, c);
                    if (vfloat != v.type) {
                        error(&cd, 7, ip-1);
                    }
                    var[x - 'a'] = v;
                }
                break;

            case 'S':
                // get the Variable
                x = cd.body[ip++];
                if (islower(x) == 0) {
                    error(&cd, 2, ip-1);
                }

                // look ahead
                c = cd.body[ip++];
                if (c == '"') {
                    var[x - 'a'].s = readstr(&cd, &ip);
                    int len = strlen(var[x - 'a'].s);
                    char *str = malloc((len + 1)*sizeof(char));
                    strcpy(str, var[x - 'a'].s);
                    var[x - 'a'].s = str;
                    var[x - 'a'].type = vstring;
                } else {
                    Variable v = getoperand(&cd, &ip, c);
                    if (vstring != v.type) {
                        error(&cd, 7, ip-1);
                    }
                    var[x - 'a'] = v;
                }
                break;

            case 'B':
                // get the Variable
                x = cd.body[ip++];
                if (islower(x) == 0) {
                    error(&cd, 2, ip-1);
                }

                // look ahead
                c = cd.body[ip++];
                if (c == '"') {
                    var[x - 'a'].b = atoi(readstr(&cd, &ip)) ? true : false;
                    var[x - 'a'].type = vbool;
                } else {
                    Variable v = getoperand(&cd, &ip, c);
                    if (vint != v.type) {
                        error(&cd, 7, ip-1);
                    }
                    var[x - 'a'] = v;
                }
                break;
            
            case 'Q':
                ip++;
                break;

            /*jump*/
            case 'J':
                // look ahead
                c = cd.body[ip++];
                int jumpPosition;
                if (c == '"') {
                    jumpPosition = atoi(readstr(&cd, &ip));
                } else if (islower(c)) {
                    jumpPosition = cd.jumps[c - 'a'];
                } else {
                    error(&cd, 2, ip-1);
                }
                if (jumpPosition >= cd.length) {
                    error(&cd, 5, jumpPosition);
                }
                ip = jumpPosition;
                break;

            /*conditional-jump*/
            case 'X':
                // get the Variable
                x = cd.body[ip++];
                Variable cond = getoperand(&cd, &ip, x); 
                if (cond.type != vbool) {
                    error(&cd, 3, ip-1);
                }
                // look ahead
                c = cd.body[ip++];
                if (c == '"') {
                    jumpPosition = atoi(readstr(&cd, &ip));
                } else if (islower(c)) {
                    jumpPosition = cd.jumps[c - 'a'];
                } else {
                    error(&cd, 2, ip-1);
                }

                if (jumpPosition >= cd.length) {
                    error(&cd, 5, jumpPosition);
                }
                // condition
                if (cond.b == true) {
                    ip = jumpPosition;
                }
                break;

            /* print */
            case 'O':
                // look ahead
                x = cd.body[ip++];

                if (x == '"') {
                    printf("%s", readstr(&cd, &ip));
                } else {
                    print(getoperand(&cd, &ip, x));
                }
                break;

            /* input */
            case 'N':
                // get the Variable
                x = cd.body[ip++];
                if (islower(x) == 0) {
                    error(&cd, 2, ip-1);
                }
                input(&cd, &ip, &var[x - 'a']);
                break;

            /* push */
            case 'P':
                x = cd.body[ip++];
                push(getoperand(&cd, &ip, x));
                break;
            /* pop */
            case 'Y':
                x = cd.body[ip++];
                if (islower(x) == 0) {
                    error(&cd, 2, ip-1);
                }
                pop(&var[x - 'a']);
                break;
            /* import */
            case 'W':
                // get the Func
                x = cd.body[ip++];
                if (islower(x) == 0) {
                    error(&cd, 2, ip-1);
                }
                // look ahead
                c = cd.body[ip++];
                if (c == '"') {
                    sprintf(readbuffer, "%s.skr", readstr(&cd, &ip));
                    func[x - 'a'] = readfunction(readbuffer);
                } else {
                    error(&cd, 2, ip-1);
                }
                break;

            /* return */
            case 'R':
                c = cd.body[ip++];
                return getoperand(&cd, &ip, c);
                break;

            default:
                returnValue = operation(&cd, &ip, c);
        }
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
Code readfunction(const char* skr_file)
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