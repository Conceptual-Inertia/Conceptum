/*
 * main.c
 * The "Conceptum" Turing-Complete VM source code file
 * Copyright (c) 2016 Ruijie Fang <ruijief@acm.org>
 * All versions released under The GNU General Public License v3.0.
 * A LICENSE Copy can be found in the project repository.
 * ALL RIGHTS RESERVED.
 *
 * ACKNOWLEDGEMENTS
 * ----------------
 *
 *  - Jonathan Shug, who helped debug a memory error
 */

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <float.h>
#include <string.h>
#include <ctype.h>


#include "memman.h"

// Limits

#define CONCEPTIP_MAX_LENGTH 30000
#define CONCEPTFP_MAX_LENGTH 30000

/*
 * Comceptum Instruction set
 */

#define CONCEPT_IADD 100 // Integer Addition OUTPUT: Integer
#define CONCEPT_IDIV 101 // Integer Division OUTPUT: Integer
#define CONCEPT_IMUL 102 // Integer Multiplication OUTPUT: Integer

#define CONCEPT_FADD 103 // Float Addition OUTPUT: Float
#define CONCEPT_FDIV 104 // Float Division OUTPUT: Float
#define CONCEPT_FMUL 105 // Float Multiplication OUTPUT: Float

#define CONCEPT_ILT 106 // Integer Less Than OUTPUT: Boolean
#define CONCEPT_IEQ 107 // Integer Equal To OUTPUT: Boolean
#define CONCEPT_IGT 108 // Integer Greater Than OUTPUT: Boolean
#define CONCEPT_FLT 109 // Float Less Than OUTPUT: Boolean
#define CONCEPT_FEQ 110 // Float Equal To OUTPUT: Boolean
#define CONCEPT_FGT 111 // Float Greater than OUTPUT: Boolean
#define CONCEPT_AND 112 // Boolean AND OUTPUT: Boolean
#define CONCEPT_OR  113 // Boolean OR  OUTPUT: Boolean
#define CONCEPT_XOR 114 // Boolean XOR OUTPUT: Boolean
#define CONCEPT_NE  115 // Boolean NE  OUTPUT: Boolean
#define CONCEPT_IF  116 // Boolean IF  OUTPUT: Boolean

#define CONCEPT_CCONST 117 // Initialize Char Constant OUTPUT: Void
#define CONCEPT_ICONST 118 // Initialize Integer Constant OUTPUT: Void
#define CONCEPT_SCONST 119 // Initialize String Constant OUTPUT: Void
#define CONCEPT_FCONST 120 // Initialize Float Constant OUTPUT: Void
#define CONCEPT_BCONST 121 // Initialize Boolean Constant OUTPUT: Void
#define CONCEPT_VCONST 122 // Initialize Void Constant OUTPUT: Void

#define CONCEPT_PRINT 123 // Print to stdout OUTPUT: Void
#define CONCEPT_CALL 124 // Call a procedure(void *)
#define CONCEPT_GLOAD 127 // Load global value
#define CONCEPT_GSTORE 128 // Store global value
#define CONCEPT_POP 129 // Pop a value out of stack
#define CONCEPT_IF_ICMPLE 130 // if_icmple
#define CONCEPT_GOTO 131 // Goto Statement
#define CONCEPT_RETURN 132 // Return
#define CONCEPT_INC 133
#define CONCEPT_DEC 134
#define CONCEPT_DUP 135
#define CONCEPT_SWAP 136
#define CONCEPT_SHIFTL 137
#define CONCEPT_SHIFTR 138

// DEBUG prettifiers

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

/* ========================
 * Error handling functions
 * ========================
 */

// Error types
#define CONCEPT_COMPILER_ERROR 200
#define CONCEPT_STACK_OVERFLOW 201
#define CONCEPT_BUFFER_OVERFLOW 202
#define CONCEPT_INVALID_PARAMETER 203
#define CONCEPT_INVALID_TYPE 204
#define CONCEPT_GENERAL_ERROR 205
#define CONCEPT_FILE_EMPTY 206

#define CONCEPT_STATE_INFO 90
#define CONCEPT_STATE_WARNING 91
#define CONCEPT_STATE_ERROR 92
#define CONCEPT_STATE_CATASTROPHE 93
#define CONCEPT_WARN_NOEXIT 94
#define CONCEPT_WARN_EXITNOW 95
#define CONCEPT_NOWARNING_EXIT 96
#define CONCEPT_HALT 0
#define CONCEPT_ABORT 97

#define DEBUG 1 // debug params

static int32_t if_handles_exception(int32_t if_exception) {
    switch(if_exception) {
        case CONCEPT_WARN_NOEXIT:
            return 1;
        case CONCEPT_NOWARNING_EXIT:
            return 0;
        case CONCEPT_WARN_EXITNOW:
            return 0;
        case CONCEPT_HALT:
            return 0;
        case CONCEPT_ABORT:
            return 0;
        default:
            return 0;
    }
}

static void on_error(int32_t error, char *msg, int32_t action, int32_t if_exception) {  // TODO TODO Add Memory free!!
    switch(action) {
        case CONCEPT_STATE_INFO:
            if(if_handles_exception(if_exception))
                printf("[CONCEPTUM-Runtime] INFO: %s {%d}", msg, error);
            break;
        case CONCEPT_STATE_WARNING:
            if(if_handles_exception(if_exception))
                printf("[CONCEPTUM-Runtime] WARNING: %s {%d}", msg, error);
            break;
        case CONCEPT_STATE_ERROR:
            if(if_handles_exception(if_exception))
                printf("[CONCEPTUM-Runtime] NONEXIT ERROR: %s {%d}", msg, error);
            else
                printf("[CONCEPTUM-Runtime] EXIT ERROR: %s {%d}", msg, error);
                exit(if_exception);
            break;
        case CONCEPT_STATE_CATASTROPHE:
            printf("[CONCEPTUM-Runtime]");
            exit(if_exception);
    }
}

/* ========================
 * Utils
 * ========================
 */

// Conceptual Boolean
#define FALSE 0;
#define TRUE 1;
typedef int32_t BOOL;


// Conceptual String
typedef struct {
    char (*value);
    int32_t len;
} ConceptString_t;

// Conceptual Stack
typedef struct {
    int32_t top;
    int32_t size;
    void *(*operand_stack);
} ConceptStack_t;

struct {
    char **code;
    int32_t len;
} concept_program;

typedef struct {
    int32_t instr;
    void *payload;
} ConceptInstruction_t;


char **procedure_call_table;
int32_t procedure_call_table_length;

int32_t *procedure_length_table;
int32_t procedure_length_table_length;

ConceptInstruction_t **program;

/*
 * Utility Functions (Might not be used at all)
 * but for nominative references
 */


char* remove_spaces(char *src){
    char *dst = rmalloc(sizeof(src));
    int32_t s, d=0;
    for (s=0; src[s] != 0; s++)
        if (src[s] != ' ' && src[s] != '\t') {
            dst[d] = src[s];
            d++;
        }
    dst[d] = 0;
    return dst;
}

int is_void(char *s) {
    while (*s != '\0') {
        if (!isspace(*s))
            return 0;
        s++;
    }
    return 1;
}

int is_int(char *tbd) {
    char *rmvd_tbd = remove_spaces(tbd);
    return isdigit(atoi(rmvd_tbd));
}

int is_char(char *tbd) {
    char *rmvd_tbd = remove_spaces(tbd);
    return (strlen(rmvd_tbd) == 1 | strlen(rmvd_tbd) == 0);
}

int is_float(char *tbd) {
    double d = FLT_MAX;
    d = strtod(tbd, NULL);
    return ((d > 0 && (d > FLT_MAX || d < FLT_MIN))
            || (d < 0 && (d < -FLT_MAX || d > -FLT_MIN)));
}


/*
 * Stack Operations Functions
 */

// Allocate stack
static void stack_alloc(ConceptStack_t *stack, int32_t bt_size) {
    // size of a void pointer * maximum size
    void *stackContents = rmalloc(sizeof(void *) * bt_size);
    stack->operand_stack = stackContents;
    stack->size = bt_size;
    stack->top = (-1);

    if(DEBUG)
        printf("\nSTACK: ALLOC ConceptStack_t @ addr %p , operand_stack @ addr %p, size %d\n", stack, stack->operand_stack, stack->size);
}

// Deallocate (reset) stack
static void stack_dealloc(ConceptStack_t *stack) {
    // free objects stored in stack first
    for(int32_t i = 0; i <= stack->top; i++) {
        free(stack->operand_stack[i]);
    }
    // free the stack itself
    free(stack->operand_stack);
    // reset stack properties
    stack->top = -1;
    stack->size = 0;
}

// Free stack memory
inline static void stack_free(ConceptStack_t *stack) {
    // Deallocate the stack's contents and properties
    stack_dealloc(stack);
    // Free everything
    free(stack);
}

// Check if stack is empty TRUE: empty FALSE: not empty
inline static BOOL stack_is_empty(ConceptStack_t *stack) {
    return (stack->top == (-1));
}

// Check if stack is full TRUE: full FALSE: not full
inline static BOOL stack_is_full(ConceptStack_t *stack) {
    return (stack->top >= stack->size - 1);
}

// Push a content pointer into stack
static void stack_push(ConceptStack_t *stack, void *content_ptr) {
    void *local_ptr = content_ptr;

    // Exit when full
    if(stack_is_full(stack))
        on_error(CONCEPT_STACK_OVERFLOW, "Stack is full, operation abort.", CONCEPT_STATE_ERROR, CONCEPT_WARN_EXITNOW);

    if(DEBUG)
        printf("\nSTACK: PUSH, addr %p", local_ptr);

    // Push while incrementing top value
    stack->operand_stack[++(stack->top)] = local_ptr; // Increase by one BEFORE pushing

}

// Pop a content pointer out of the stack
static void* stack_pop(ConceptStack_t *stack) {
    if(stack_is_empty(stack)) {
        on_error(CONCEPT_GENERAL_ERROR, "Stack is empty. Returning a NULL.", CONCEPT_STATE_INFO, CONCEPT_WARN_NOEXIT);
        return NULL; // Nothing is stored yet!
    }

    void *ret =  stack->operand_stack[(stack->top)--]; // Decrease by one AFTER popping

    if(DEBUG) printf("\nSTACK: POP, addr %p, current top %d", ret, (stack->top));
    return ret;
}

// IADD Integer addition function
void concept_iadd(ConceptStack_t *stack) {
    int32_t a = *((int32_t *)stack_pop(stack));
    int32_t b = *((int32_t *)stack_pop(stack)); // pop again for another value

    if(DEBUG) { // print DEBUG info
        printf("\nIADD\n");
        printf("\t%d\tPLUS\t", a);
        printf("%d", b);
    }

    int32_t *c = rmalloc(sizeof(int32_t));

    if(a + b <= INT32_MAX && a + b >= INT32_MIN && !stack_is_full(stack)) {
        //int32_t c = a + b;
        *c = a + b;
        stack_push(stack, (void *)c);

        if(DEBUG) printf("\nIADD finished, RESULT %d\taddr %p", *c, c);

    } else {
        // Exceeds maximum limit, quit
        on_error(CONCEPT_BUFFER_OVERFLOW, "IADD Operation exceeds INT_MAX limit, Aborting...", CONCEPT_STATE_ERROR, CONCEPT_ABORT);
    }
}

// IDIV Integer division function
void concept_idiv(ConceptStack_t *stack) {
    int32_t a = *((int32_t *)stack_pop(stack));
    int32_t b = *((int32_t *)stack_pop(stack)); // pop again for another value

    if(DEBUG) { // print DEBUG info
        printf("\nIDIV\n");
        printf("\t%d\tDIVBY\t", a);
        printf("%d", b);
    }

    int32_t *c = rmalloc(sizeof(int32_t));

    if(a / b <= INT32_MAX && a / b >= INT32_MIN && !stack_is_full(stack)) {
        *c = a / b;
        stack_push(stack, (void *)c);

        if(DEBUG) printf("\nIDIV finished, RESULT %d\taddr %p", *c, c);

    } else {
        // Exceeds maximum limit, quit
        on_error(CONCEPT_BUFFER_OVERFLOW, "IDIV Operation exceeds INT_MAX limit, Aborting...", CONCEPT_STATE_ERROR, CONCEPT_ABORT);
    }
}

// IMUL Integer Multiplication function
void concept_imul(ConceptStack_t *stack) {
    int32_t a = *((int32_t *)stack_pop(stack));
    int32_t b = *((int32_t *)stack_pop(stack)); // pop again for another value

    if(DEBUG) { // print DEBUG info
        printf("\nIMUL\n");
        printf("\t%d\tTIMES\t", a);
        printf("%d", b);
    }

    int32_t *c = rmalloc(sizeof(int32_t));

    if(a * b <= INT32_MAX && a * b >= INT32_MIN && !stack_is_full(stack)) {
        *c = a * b;
        stack_push(stack, (void *)c);

        if(DEBUG) printf("\nIMUL finished, RESULT%d\taddr %p", *c, c);

    } else {
        // Exceeds maximum limit, quit
        on_error(CONCEPT_BUFFER_OVERFLOW, "IMUL Operation exceeds INT_MAX limit, Aborting...", CONCEPT_STATE_ERROR, CONCEPT_ABORT);
    }
}


// FADD Floating point addition function
void concept_fadd(ConceptStack_t *stack) {
    float a = *((float *)stack_pop(stack));
    float b = *((float *)stack_pop(stack));

    if(DEBUG) { // print DEBUG info
        printf("\nFADD\n");
        printf("\t%f\tPLUS\t", a);
        printf("%f", b);
    }

    float *c = rmalloc(sizeof(float));

    if(a + b <= FLT_MAX && a + b >= FLT_MIN && !stack_is_full(stack)) {
        *c = a + b;
        stack_push(stack, (void *)c);

        if(DEBUG) printf("\nFADD finished, RESULT%f\taddr %p", *c, c);

    } else {
        // Exceeds maximum limit, quit
        on_error(CONCEPT_BUFFER_OVERFLOW, "FADD Operation exceeds INT_MAX limit, Aborting...", CONCEPT_STATE_ERROR, CONCEPT_ABORT);
    }
}

// FDIV Floating point division function
void concept_fdiv(ConceptStack_t *stack) {
    float a = *((float *)stack_pop(stack));
    float b = *((float *)stack_pop(stack));

    if(DEBUG) { // print DEBUG info
        printf("\nFDIV\n");
        printf("\t%f\tDIVBY\t", a);
        printf("%f", b);
    }

    float *c = rmalloc(sizeof(float));

    if(a / b <= FLT_MAX && a / b >= FLT_MIN && !stack_is_full(stack)) {
        *c = a / b;
        stack_push(stack, (void *)c);

        if(DEBUG) printf("\nFDIV finished, RESULT %f\taddr %p", *c, c);

    } else {
        // Exceeds maximum limit, quit
        on_error(CONCEPT_BUFFER_OVERFLOW, "FADD Operation exceeds INT_MAX limit, Aborting...", CONCEPT_STATE_ERROR, CONCEPT_ABORT);
    }
}

// FMUL Floating point multiplication function
void concept_fmul(ConceptStack_t *stack) {
    float a = *((float *)stack_pop(stack));
    float b = *((float *)stack_pop(stack));

    if(DEBUG) { // print DEBUG info
        printf("\nFMUL\n");
        printf("\t%f\tTIMES\t", a);
        printf("%f", b);
    }

    float *c = rmalloc(sizeof(float));

    if(a * b <= FLT_MAX && a * b >= FLT_MIN && !stack_is_full(stack)) {
        *c = a * b;
        stack_push(stack, (void *)c);

        if(DEBUG) printf("\nFMUL finished, RESULT %f\taddr %p", *c, c);

    } else {
        // Exceeds maximum limit, quit
        on_error(CONCEPT_BUFFER_OVERFLOW, "FADD Operation exceeds INT_MAX limit, Aborting...", CONCEPT_STATE_ERROR, CONCEPT_ABORT);
    }
}


// ILT Integer Less Than comparison function
void concept_ilt(ConceptStack_t *stack) {
    int32_t a = *((int32_t *)stack_pop(stack));
    int32_t b = *((int32_t *)stack_pop(stack));

    if(DEBUG) { // print DEBUG info
        printf("\nILT\n");
        printf("\t%d\tLESSTHAN\t", a);
        printf("%d", b);
    }

    int32_t *c = rmalloc(sizeof(int32_t));

    if(a < b && !stack_is_full(stack)) {
        *c = TRUE;
        stack_push(stack, (void *)c);

    } else {
        *c = FALSE;
        stack_push(stack, (void *)c);
    }

    if(DEBUG) printf("\nILT finished, RESULT %d\taddr %p", *c, c);

}

// IEQ Integer Equality comparison function
void concept_ieq(ConceptStack_t *stack) {
    int32_t a = *((int32_t *)stack_pop(stack));
    int32_t b = *((int32_t *)stack_pop(stack));

    if(DEBUG) { // print DEBUG info
        printf("\nIEQ\n");
        printf("\t%d\tEQUALS\t", a);
        printf("%d", b);
    }

    int32_t *c = rmalloc(sizeof(int32_t));

    if(a == b && !stack_is_full(stack)) {
        *c = TRUE;
        stack_push(stack, (void *)c);
    } else {
        *c = FALSE;
        stack_push(stack, (void *)c);
    }

    if(DEBUG) printf("\nIEQ finished, RESULT %d\taddr %p", *c, c);
}

// IGT Integer Greater Than comparison function
void concept_igt(ConceptStack_t *stack) {
    int32_t a = *((int32_t *)stack_pop(stack));
    int32_t b = *((int32_t *)stack_pop(stack));

    if(DEBUG) { // print DEBUG info
        printf("\nIGT\n");
        printf("\t%d\tGTRTHAN\t", a);
        printf("%d", b);
    }

    int32_t *c = rmalloc(sizeof(int32_t));

    if(a > b && !stack_is_full(stack)) {
        *c = TRUE;
        stack_push(stack, (void *)c);
    } else {
        *c = FALSE;
        stack_push(stack, (void *)c);
    }

    if(DEBUG) printf("\nIGT finished, RESULT %d\taddr %p", *c, c);
}

// FLT Floating point Less Than comparison function
void concept_flt(ConceptStack_t *stack) {
    float a = *((float *)stack_pop(stack));
    float b = *((float *)stack_pop(stack));

    if(DEBUG) { // print DEBUG info
        printf("\nFLT\n");
        printf("\t%f\tLESSTHAN\t", a);
        printf("%f", b);
    }

    int32_t *c = rmalloc(sizeof(float)); // int32_t used for boolean value, NOT FLOAT!

    if(a < b) {
        *c = TRUE;
        stack_push(stack, (void *)c);
    } else {
        *c = FALSE;
        stack_push(stack, (void *)c);
    }

    if(DEBUG) printf("\nFLT finished, RESULT %d\taddr %p", *c, c);
}

// FEQ Floating point Equality comparison function
void concept_feq(ConceptStack_t *stack) {
    float a = *((float *)stack_pop(stack));
    float b = *((float *)stack_pop(stack));

    if(DEBUG) { // print DEBUG info
        printf("\nFEQ\n");
        printf("\t%f\tEQUALS\t", a);
        printf("%f", b);
    }

    int32_t *c = rmalloc(sizeof(int32_t));

    if(a == b) {
        *c = TRUE;
        stack_push(stack, (void *)c);
    } else {
        *c = FALSE;
        stack_push(stack, (void *)c);
    }

    if(DEBUG) printf("\nFEQ finished, RESULT %d\taddr %p", *c, c);
}

// FGT Floating point Greater Than comparison function
void concept_fgt(ConceptStack_t *stack) {
    float a = *((float *)stack_pop(stack));
    float b = *((float *)stack_pop(stack));

    if(DEBUG) { // print DEBUG info
        printf("\nFGT\n");
        printf("\t%f\tGRTHAN\t", a);
        printf("%f", b);
    }

    int32_t *c = rmalloc(sizeof(int32_t));

    if(a > b) {
        *c = TRUE;
        stack_push(stack, (void *)c);
    } else {
        *c = FALSE;
        stack_push(stack, (void *)c);
    }

    if(DEBUG) printf("\nFGT finished, RESULT %d\taddr %p", *c, c);
}

// AND
void concept_and(ConceptStack_t *stack) {

    if(DEBUG) printf("\nAND");

    BOOL *and = rmalloc(sizeof(BOOL));
    if(!stack_is_full(stack)) {
        *and = (*(int32_t *)stack_pop(stack) & *(int32_t *)stack_pop(stack));
        stack_push(stack, (void *)and);
    }

    if(DEBUG) printf("\nAND finished, RESULT %d\taddr %p", *and, and);
}

// OR
void concept_or(ConceptStack_t *stack) {

    if(DEBUG) printf("\nOR");

    BOOL *or = rmalloc(sizeof(BOOL));
    if(!stack_is_full(stack)) {
        *or = (*(int32_t *)stack_pop(stack) | *(int32_t *)stack_pop(stack));
        stack_push(stack, (void *)or);
    }

    if(DEBUG) printf("\nOR finished, RESULT %d\taddr %p", *or, or);
}

// XOR
void concept_xor(ConceptStack_t *stack) {

    int32_t p = *(int32_t *) stack_pop(stack);
    int32_t q = *(int32_t *) stack_pop(stack);

    if(DEBUG) printf("\nXOR (%d XOR %d)", p, q);

    BOOL *xor = rmalloc(sizeof(BOOL));
    if(!stack_is_full(stack)) {
        *xor = (p & (!q)) | ((!p) & q);
        stack_push(stack, (void *)xor);
    }

    if(DEBUG) printf("\nXOR finished, RESULT %d\taddr %p", *xor, xor);
}

// NE
void concept_ne(ConceptStack_t *stack) {

    int32_t p = (*(int32_t *)stack_pop(stack));

    if(DEBUG) printf("\nNE (!%d)", p);

    BOOL *ne = rmalloc(sizeof(BOOL));
    if(!stack_is_full(stack)) {
        *ne = (!p);
        stack_push(stack, (void *)ne);
    }

    if(DEBUG) printf("\nNE finished, RESULT %d\taddr %p", *ne, ne);
}

// IF
void concept_if(ConceptStack_t *stack) {

    int32_t p = *(int32_t *) stack_pop(stack);
    int32_t q = *(int32_t *) stack_pop(stack);

    if(DEBUG) printf("\nIF(Boolean Algebra Operation), %d->%d", p, q);

    BOOL *cp_if = rmalloc(sizeof(BOOL));
    if(!stack_is_full(stack)) {
        *cp_if = ((!p) | q);
        stack_push(stack, (void *)cp_if);
    }

    if(DEBUG) printf("\nIF (Boolean Algebra Operation) finished, RESULT %d\taddr %p", *cp_if, cp_if);
}

void concept_cconst(ConceptStack_t *stack, char c) {

    if(DEBUG) printf("\nCCONST %c", c);

    char *c_ptr = rmalloc(sizeof(char)); // Prevent space from being collected

    *c_ptr = c;

    stack_push(stack, (void *)c_ptr);
}

void concept_iconst(ConceptStack_t *stack, int32_t i) {

    if(DEBUG) printf("\nICONST %d", i);

    int32_t *i_ptr = rmalloc(sizeof(int32_t));
    *i_ptr = i;

    stack_push(stack, (void *)i_ptr);
}

void concept_sconst(ConceptStack_t *stack, char *s) {

    if(DEBUG) {
        printf("\nSCONST\n");
        printf("Dumped Contents\n");
        printf("-=-=-=-=-=-=-=-=-\n");
        printf("%s", s);
        printf("\n\n");
    }

    char **s_ptr = rmalloc(sizeof(s)); // better than sizeof char*
    *s_ptr = s;

    stack_push(stack, (void *)s_ptr); // pointers, pointers, pointers dreaded POINTERS!!!!
}

void concept_fconst(ConceptStack_t *stack, float f) {

    if(DEBUG) printf("\nFCONST %f", f);

    float *f_ptr = rmalloc(sizeof(float));
    *f_ptr = f;

    stack_push(stack, (void *)f_ptr);
}


void concept_bconst(ConceptStack_t *stack, BOOL b) {

    if(DEBUG) printf("\nBCONST %d", b);

    BOOL *b_ptr = rmalloc(sizeof(BOOL));
    *b_ptr = b;
    if(!stack_is_full(stack))
        stack_push(stack, (void *)b_ptr);
}

void concept_vconst(ConceptStack_t *stack, void *v) {

    if(DEBUG) printf("\nVCONST bla bla bla... @ addr %p", v);

    // gonna be very ugly!

    void **v_ptr = rmalloc(sizeof(v));
    *v_ptr = v;

    if(!stack_is_full(stack))
        stack_push(stack, (void *)v_ptr);
}

void concept_print(ConceptStack_t *stack) {
    if(!stack_is_empty(stack))
        printf("%s", ((char *)stack->operand_stack[stack->top]));
}

void * concept_pop(ConceptStack_t *stack) {
    void *val = stack_pop(stack);
    return val;
}

void concept_incr(ConceptStack_t *stack) {
    int32_t *i = (int32_t*)rmalloc(sizeof(int32_t));
    *i = *((int32_t *)(stack_pop(stack))) + 1;
    stack_push(stack, i);
}

void concept_decr(ConceptStack_t *stack) {
    int32_t *i = (int32_t*)rmalloc(sizeof(int32_t));
    *i = *((int32_t *)(stack_pop(stack))) - 1;
    stack_push(stack, i);
}

void concept_swap(ConceptStack_t *stack) {
    int32_t i = *((int32_t *)(stack_pop(stack)));
    int32_t j = *((int32_t *)(stack_pop(stack)));
    int32_t *k = (int32_t *)rmalloc(sizeof(int32_t));
    *k = i + j;
    stack_push(stack, k);
}

void concept_dupl(ConceptStack_t *stack) {
    void *v = stack_pop(stack);
    // duplicate
    stack_push(stack, v);
    stack_push(stack, v);

}

int32_t* go_to(int32_t line_number) { // TODO TODO

    int32_t cumulative_line_count = 0;

    for(int32_t findex = 0; findex < procedure_length_table_length; findex++) {
        cumulative_line_count += procedure_length_table[findex];
        int32_t next_cum_len = cumulative_line_count + procedure_length_table[findex+1];
        if(line_number >= cumulative_line_count && line_number <= next_cum_len) {
            // in this procedure!
            int32_t pc_line = (line_number - cumulative_line_count);
            int32_t *rtn = (int32_t *)rmalloc(sizeof(int32_t)*3);
            rtn[0] = findex;
            rtn[1] = pc_line;
            rtn[2] = (-1);
            return rtn;
        }
    }
    // not a valid go to
    on_error(CONCEPT_COMPILER_ERROR, " goto statement not valid.", CONCEPT_STATE_ERROR, CONCEPT_WARN_EXITNOW);
    return 0; // to satisfy IDE
}

/*
 * Concept Debug Program
 */
int32_t concept_debug() {

    if(DEBUG) printf("\nConceptum Runtime DEBUG environment\n");

    ConceptStack_t stack_test;
    stack_alloc(&stack_test, 300);

    int32_t i = 28;
    int32_t j = 25;

    stack_push(&stack_test, (void *)&i);
    stack_push(&stack_test, (void *)&j);

    int32_t k = *((int32_t *)stack_pop(&stack_test));

    printf("\n%d\n", k);

    stack_push(&stack_test, (void *)&k);

    concept_iadd(&stack_test);

    int32_t *n = (int32_t *) (stack_pop(&stack_test));
    printf("\n%d\n", *n);

    int32_t a = 110;
    int32_t b = 20;
    stack_push(&stack_test, (void *)&a);
    stack_push(&stack_test, (void *)&b);

    concept_imul(&stack_test);
    int32_t *m = (int32_t *) (stack_pop(&stack_test));
    printf("\n%d\n", *m);

    stack_push(&stack_test, (void *)m);
    stack_push(&stack_test, (void *)n); // push back for div

    concept_idiv(&stack_test);
    int32_t *o = (int32_t *) (stack_pop(&stack_test));
    printf("\n%d\n", *o);
    return 0;
}


/*
 * File Reader Utilities and Lexer
 */

// Iterating event loop
// TODO implement iterator
void* eval(int32_t index, ConceptStack_t *stack, ConceptStack_t *global_stack, int32_t start_by) { // TODO


    if(DEBUG) {
        if (!index)
            printf(ANSI_COLOR_RESET ANSI_COLOR_MAGENTA "\n\nConceptum: Welcome to the eval() Loop. FYI: Curr index %d, starting by line %d \n", index,
                   start_by);
        else
            printf(ANSI_COLOR_RESET ANSI_COLOR_YELLOW "\n\nConceptum: eval() being called RECURSIVELY again. FYI: Curr index %d, starting by line %d \n", index,
                   start_by);
    }

    if(DEBUG) printf("\n eval: Defining a call stack... for your mental healthcare!");
    ConceptStack_t call_stack; // if any

    if(program[0] == NULL) on_error(CONCEPT_COMPILER_ERROR, "struct ConceptInstruction_t blank.", CONCEPT_ABORT, CONCEPT_STATE_CATASTROPHE);

    int32_t goto_dispatch_line = 0;


    for(int32_t i = start_by; i < procedure_length_table[index]; i++) {

        if(DEBUG) printf("\n eval: Dispatching instruction %d @ index %d: %d", i, index, program[index][i].instr);


        switch (program[index][i].instr) {
            case CONCEPT_IADD:
                concept_iadd(stack);
                break;
            case CONCEPT_IDIV:
                concept_idiv(stack);
                break;
            case CONCEPT_IMUL:
                concept_imul(stack);
                break;
            case CONCEPT_FADD:
                concept_fadd(stack);
                break;
            case CONCEPT_FDIV:
                concept_fdiv(stack);
                break;
            case CONCEPT_FMUL:
                concept_fmul(stack);
                break;
            case CONCEPT_ILT:
                concept_ilt(stack);
                break;
            case CONCEPT_IEQ:
                concept_ieq(stack);
                break;
            case CONCEPT_IGT:
                concept_igt(stack);
                break;
            case CONCEPT_FLT:
                concept_flt(stack);
                break;
            case CONCEPT_FEQ:
                concept_feq(stack);
                break;
            case CONCEPT_FGT:
                concept_fgt(stack);
                break;
            case CONCEPT_AND:
                concept_and(stack);
                break;
            case CONCEPT_OR:
                concept_or(stack);
                break;
            case CONCEPT_XOR:
                concept_xor(stack);
                break;
            case CONCEPT_NE:
                concept_ne(stack);
                break;
            case CONCEPT_IF:
                concept_if(stack);
                break;
            case CONCEPT_CCONST:
                concept_cconst(stack,  (*(char *) (program[index][i].payload)));
                break;
            case CONCEPT_ICONST:
                concept_iconst(stack, (*(int32_t *)(program[index][i].payload)));
                break;
            case CONCEPT_SCONST:
                concept_sconst(stack, (char *)(program[index][i].payload));
                break;
            case CONCEPT_FCONST:
                concept_fconst(stack, (*(float *)(program[index][i].payload)));
                break;
            case CONCEPT_BCONST:
                concept_bconst(stack, (*(BOOL *)(program[index][i].payload)));
                break;
            case CONCEPT_VCONST:
                //concept_vconst(stack, program[index][i].payload);
                break;
            case CONCEPT_PRINT:
                concept_print(stack);
                break;
            case CONCEPT_POP:
                concept_pop(stack);
                break;
            case CONCEPT_GLOAD:
                stack_push(stack, stack_pop(global_stack));
                break;
            case CONCEPT_GSTORE:
                stack_push(global_stack, stack_pop(stack));
                break;
            case CONCEPT_CALL:
                stack_alloc(&call_stack, CONCEPTFP_MAX_LENGTH);
                if(DEBUG) printf("\nFCALL\t:%d (Name: %s)", (*(int32_t *)(program[index][i].payload)), procedure_call_table[*(int32_t *)(program[index][i].payload)]);
                stack_push(stack, eval((*(int32_t *)(program[index][i].payload)), &call_stack, global_stack, 0));
                // stack_push(stack, ret_val);
                break;
            case CONCEPT_INC:
                concept_incr(stack);
                break;
            case CONCEPT_DEC:
                concept_decr(stack);
                break;
            case CONCEPT_SWAP:
                concept_swap(stack);
                break;
            case CONCEPT_DUP:
                concept_dupl(stack);
                break;
            case CONCEPT_IF_ICMPLE:
                if(*((BOOL *)(stack_pop(stack))))
                    return eval(((int32_t*)(program[index][i].payload))[0], stack, global_stack, ((int32_t*)(program[index][i].payload))[1]);
                break;
            case CONCEPT_GOTO:
                return eval(((int32_t*)(program[index][i].payload))[0], stack, global_stack, ((int32_t*)(program[index][i].payload))[1]);
                break;
            case CONCEPT_HALT:
                on_error(CONCEPT_GENERAL_ERROR, " Exit by HALT.", CONCEPT_STATE_ERROR, CONCEPT_WARN_EXITNOW);
                break;
            case CONCEPT_RETURN:
                if(DEBUG) printf("\neval: RETURNing to parent function call...\n" ANSI_COLOR_RESET ANSI_COLOR_MAGENTA);
                return stack_pop(stack);
                break; // DO NOTHING
            default:
                on_error(CONCEPT_COMPILER_ERROR, "Error: Unknown instruction", CONCEPT_STATE_CATASTROPHE,
                         CONCEPT_ABORT);
                break; // do nothing
        }
    }
     if(DEBUG) printf("\neval: Naturally RETURNing to parent function call...\n");
     return stack_pop(stack); // TODO TODO redesign this function.
}

void cleanup(ConceptStack_t *global_stack) {
    memfree();
    stack_free(global_stack);
}


char* substring(char *string, int32_t start, int32_t end) {
    char* subbuff = rmalloc(sizeof(char) * (end - start));
    memcpy(subbuff, &string[start], (end - start));
    subbuff[end - start] = '\0';
    return subbuff;
}

void read_prog(char *file_path) {
    int32_t lines_allocated = 128;
    int32_t max_line_len = 100;

    /* Allocate lines of text */
    char **words = (char **)rmalloc(sizeof(char*)*lines_allocated);
    if (words==NULL) {
        fprintf(stderr,"Out of memory (1).\n");
        exit(1);
    }

    FILE *fp = fopen(file_path, "r");
    if (fp == NULL) {
        fprintf(stderr,"Error opening file.\n");
        exit(2);
    }

    int32_t i;
    for (i=0;1;i++) {
        int32_t j;

        /* Have we gone over our line allocation? */
        if (i >= lines_allocated) {
            int32_t new_size;

            /* Double our allocation and re-allocate */
            new_size = lines_allocated*2;
            words = (char **)rrealloc(words,sizeof(char*)*new_size);
            if (words==NULL) {
                fprintf(stderr,"err read_file(): Out of memory.\n");
                exit(3);
            }
            lines_allocated = new_size;
        }
        /* Allocate space for the next line */
        words[i] = rmalloc(max_line_len);
        if (words[i]==NULL) {
            fprintf(stderr,"err read_file(): Out of memory (3).\n");
            exit(4);
        }
        if (fgets(words[i],max_line_len-1,fp)==NULL)
            break;

        /* Get rid of CR or LF at end of line */
        for (j=strlen(words[i])-1;j>=0 && (words[i][j]=='\n' || words[i][j]=='\r');j--);
        ; ; ;
        words[i][j+1]='\0';
    }
    /* Close file */
    fclose(fp);

    // words: string array
    // Every string in the array represents a line in the actual file.
    // The array itself represents the text file composed of multiple line(s).
    // The length here, (i) means the length of the string array (since a pointer of pointer can not be measured)

    concept_program.code = words;
    concept_program.len = i;

    //int32_t j;
    //for(j = 0; j < i; j++)
    //    printf("%s\n", words[j]);

    /* Good practice to free memory */
    //for (;i>=0;i--)
    //    free(words[i]);
    //free(words);
    //return 0;
}
// 1

// parse_procedures() reads in line by line, and finds the line declaring a procedure.
// After that the procedure is being parsed in to an array of linear bytecodes
// After that a bytecode array is constructed
// A similar array of strings ^^ procedure_call_table is also constructed in order to map the value of a function to its address in the bytecode
// array
// finally the interpreter will perform inline expansion on all calls to make the destination procedure's name NOT
// the String name, but the ACTUAL address of the bytecode procedure, which in turn makes an O(n) + O(1) complexity an O(1) complexity
void parse_procedures() {

    if(DEBUG) printf(ANSI_COLOR_CYAN "\nConceptual-FANNGGOVITCH Bytecode Parser. Parsing input...\n");

    int32_t how_many_procedures = 0;
    for(int32_t d = 0; d < concept_program.len; d++) {
        if(strstr(concept_program.code[d], "procedure")) {
            how_many_procedures++;
        }
    }

    if(DEBUG) printf("\nParsing procedures... Procedures count: %d", how_many_procedures);

    procedure_call_table = (char **)rmalloc(sizeof(char *)*how_many_procedures);

    if(DEBUG) printf("\nAllocated procedure call table... Call table size: %lu \t Call items: %lu",
                     sizeof(procedure_call_table), sizeof(procedure_call_table)/sizeof(char *));

    if(DEBUG) printf("\n\nParsing input into procedure call table...");

    int32_t prog_counter = 0;
    XXX_get_procedure_stats: for(int32_t d = 0; d < concept_program.len; d++) {
        if(strstr(concept_program.code[d], "procedure")) {
            char *proc = concept_program.code[d];

            if(DEBUG) printf("\n Parse: Found 1 procedure. %d th @ line %d listing:  >> %s", prog_counter, d, proc);

            char *proc_w_s = remove_spaces(proc);

            if(DEBUG) printf("\n Parse: Removed procedure declaration line spaces. Printout: >> %s", proc_w_s);

            char *proc_name = substring(proc, 10, ((int32_t)strlen(proc_w_s)+1));

            if(DEBUG) printf("\n Parse: Extracted procedure name using substring. Pushing into the call table... Result: >> %s", proc_name);
            procedure_call_table[prog_counter] = proc_name;

            if(DEBUG) printf("\n Parse: %d:%d:%s pushed into function call table. Congrats!", d, prog_counter, proc_name);

            prog_counter++;
        }
    }

    procedure_call_table_length = prog_counter;
    if(DEBUG) printf("\n Parse: Parsed procedure names. Call table length: %d. Now allocating bytecode array...", procedure_call_table_length);

    ConceptInstruction_t **compiled_bytecode_collection = (ConceptInstruction_t **)rmalloc(sizeof(ConceptInstruction_t *)* prog_counter);
    procedure_length_table = (int32_t *)rmalloc(sizeof(int32_t)*prog_counter);
    procedure_length_table_length = prog_counter;
    int32_t procedure_counter = 0;

    if(DEBUG) {
        printf("\n Parse: Bytecode array allocated. Proceeding to parse source code into bytecode...");
        printf("\nFANNGGOVITCH Bytecode Lexer: START\n");
    }

    XXX_parse_each_procedures: for(int32_t j = 0; j < concept_program.len; j++) { // read in the procedure(s)
        if(strstr(concept_program.code[j], "procedure")) {
            ConceptInstruction_t *procedure; // ConceptInstruction_t
            int32_t counter = 0; // fur PSA
            int32_t i = j;
            for(j; !strstr(concept_program.code[j], "ret"); j++);
            int32_t procedure_len = j - i;
            procedure_length_table[procedure_counter] = procedure_len;
            char *prog_name_line = concept_program.code[i];
            if(DEBUG) {
                printf("\n lexer: %dth Procedure discovered @ %d, procedure return discovered @ %d, len %d \n\t| procedure name >> %s",
                       procedure_counter, i, j, procedure_len, prog_name_line);
            }

            procedure = (ConceptInstruction_t *)rmalloc(procedure_len * sizeof(ConceptInstruction_t)); // including the return statement

            if(DEBUG) {
                printf("\n lexer: Allocated procedure bytecode array space. Total size: %lu; Len: %d. Parsing every single line of program..." ANSI_COLOR_RESET,
                       sizeof(procedure), procedure_len);
                printf(ANSI_COLOR_GREEN "\n lexer: ProgramSyntaxAnalyser: START\n");
            }

            XXX_parse_each_line_in_procedure: for(i = i + 1; i <= j; i++) { // from the first line of program to the ret statement, read every line and parse
                // parse, parse, parse!
                char *s_line = concept_program.code[i];
                int32_t p;
                for(p = 0; p < strlen(s_line) && s_line[p] != ' ' && s_line[p] != '\t'; p++);
                char *instr = (char *)rmalloc(sizeof(char) * p);
                for(int32_t q = 0; q < p; q++) instr[q] = s_line[q];
                int32_t param_flag = 0;
                char *param;
                if(strlen(s_line) - p > 0) {
                    param_flag = 1;
                    param = (char *)rmalloc(sizeof(char)*(strlen(s_line)-p));
                    int32_t r = 0;
                    for(p = p + 1; p < strlen(s_line); p++) {
                        param[r] = s_line[p];
                        r++;
                    }
                }

                if(DEBUG) {
                    printf(" \nlexer: PSA: Resolved 1 line. Instr: ||%s||.", instr);
                    if(param_flag)
                        printf(" \n\tParam has flag. Flag: %s.", param);
                }

                // The advent of a gigantic if... C switches doesn't support char*
                if(!strcmp(instr, "iadd")) {
                    procedure[counter].instr = CONCEPT_IADD;
                    if(DEBUG) printf("\nlexer: PSA: Instr is IADD. Currently assigning @ line [%d]. Program [%d].", (counter), procedure_counter);
                } else if(!strcmp(instr, "idiv")) {
                    procedure[counter].instr = CONCEPT_IDIV;
                    if(DEBUG) printf("\nlexer: PSA: Instr is IDIV. Currently assigning @ line [%d]. Program [%d].", (counter), procedure_counter);
                } else if(!strcmp(instr, "imul")) {
                    procedure[counter].instr = CONCEPT_IMUL;
                    if(DEBUG) printf("\nlexer: PSA: Instr is IMUL. Currently assigning @ line [%d]. Program [%d].", (counter), procedure_counter);
                } else if(!strcmp(instr, "fadd")) {
                    procedure[counter].instr = CONCEPT_FADD;
                    if(DEBUG) printf("\nlexer: PSA: Instr is FADD. Currently assigning @ line [%d]. Program [%d].", (counter), procedure_counter);
                } else if(!strcmp(instr, "fdiv")) {
                    procedure[counter].instr = CONCEPT_FDIV;
                    if(DEBUG) printf("\nlexer: PSA: Instr is FDIV. Currently assigning @ line [%d]. Program [%d].", (counter), procedure_counter);
                } else if(!strcmp(instr, "fmul")) {
                    procedure[counter].instr = CONCEPT_FMUL;
                    if(DEBUG) printf("\nlexer: PSA: Instr is FMUL. Currently assigning @ line [%d]. Program [%d].", (counter), procedure_counter);
                } else if(!strcmp(instr, "ilt")) {
                    procedure[counter].instr = CONCEPT_ILT;
                    if(DEBUG) printf("\nlexer: PSA: Instr is ILT. Currently assigning @ line [%d]. Program [%d].", (counter), procedure_counter);
                } else if(!strcmp(instr, "ieq")) {
                    procedure[counter].instr = CONCEPT_IEQ;
                    if(DEBUG) printf("\nlexer: PSA: Instr is IEQ. Currently assigning @ line [%d]. Program [%d].", (counter), procedure_counter);
                } else if(!strcmp(instr, "igt")) {
                    procedure[counter].instr = CONCEPT_IGT;
                    if(DEBUG) printf("\nlexer: PSA: Instr is IGT. Currently assigning @ line [%d]. Program [%d].", (counter), procedure_counter);
                } else if(!strcmp(instr, "flt")) {
                    procedure[counter].instr = CONCEPT_FLT;
                    if(DEBUG) printf("\nlexer: PSA: Instr is FLT. Currently assigning @ line [%d]. Program [%d].", (counter), procedure_counter);
                } else if(!strcmp(instr, "feq")) {
                    procedure[counter].instr = CONCEPT_FEQ;
                    if(DEBUG) printf("\nlexer: PSA: Instr is FEQ. Currently assigning @ line [%d]. Program [%d].", (counter), procedure_counter);
                } else if(!strcmp(instr, "fgt")) {
                    procedure[counter].instr = CONCEPT_FGT;
                    if(DEBUG) printf("\nlexer: PSA: Instr is FGT. Currently assigning @ line [%d]. Program [%d].", (counter), procedure_counter);
                } else if(!strcmp(instr, "and")) {
                    procedure[counter].instr = CONCEPT_AND;
                    if(DEBUG) printf("\nlexer: PSA: Instr is AND. Currently assigning @ line [%d]. Program [%d].", (counter), procedure_counter);
                } else if(!strcmp(instr, "or")) {
                    procedure[counter].instr = CONCEPT_OR;
                    if(DEBUG) printf("\nlexer: PSA: Instr is OR. Currently assigning @ line [%d]. Program [%d].", (counter), procedure_counter);
                } else if(!strcmp(instr, "xor")) {
                    procedure[counter].instr = CONCEPT_XOR;
                    if(DEBUG) printf("\nlexer: PSA: Instr is XOR. Currently assigning @ line [%d]. Program [%d].", (counter), procedure_counter);
                } else if(!strcmp(instr, "ne")) {
                    procedure[counter].instr = CONCEPT_NE;
                    if(DEBUG) printf("\nlexer: PSA: Instr is NE. Currently assigning @ line [%d]. Program [%d].", (counter), procedure_counter);
                } else if(!strcmp(instr, "if")) {
                    procedure[counter].instr = CONCEPT_IF;
                    if(DEBUG) printf("\nlexer: PSA: Instr is IF. Currently assigning @ line [%d]. Program [%d].", (counter), procedure_counter);
                } else if(!strcmp(instr, "cconst")) {
                    procedure[counter].instr = CONCEPT_CCONST;
                    if(!param_flag) exit(130);
                    char *c = rmalloc(sizeof(char));
                    *c = param[0];
                    procedure[counter].payload = (void *)c;
                    if(DEBUG) printf("\nlexer: PSA: Instr is CCONST. Currently assigning @ line [%d]. Program [%d].", (counter), procedure_counter);
                } else if(!strcmp(instr, "iconst")) {
                    procedure[counter].instr = CONCEPT_ICONST;
                    if(!param_flag) exit(130);
                    int32_t *a = rmalloc(sizeof(int32_t));
                    *a = atoi(param);
                    procedure[counter].payload = (void *)a;
                    if(DEBUG) printf("\nlexer: PSA: Instr is ICONST. Currently assigning @ line [%d]. Program [%d].", (counter), procedure_counter);
                } else if(!strcmp(instr, "sconst")) {
                    procedure[counter].instr = CONCEPT_SCONST;
                    if(!param_flag) exit(130);
                    procedure[counter].payload = (void *)param;
                    if(DEBUG) printf("\nlexer: PSA: Instr is SCONST. Currently assigning @ line [%d]. Program [%d].", (counter), procedure_counter);
                } else if(!strcmp(instr, "fconst")) {
                    procedure[counter].instr = CONCEPT_FCONST;
                    if(!param_flag) exit(130);
                    float *f = rmalloc(sizeof(float));
                    *f = (float)atof(param);
                    procedure[counter].payload = (void *)f;
                    if(DEBUG) printf("\nlexer: PSA: Instr is FCONST. Currently assigning @ line [%d]. Program [%d].", (counter), procedure_counter);
                } else if(!strcmp(instr, "bconst")) {
                    procedure[counter].instr = CONCEPT_BCONST;
                    if(!param_flag) exit(130);
                    int32_t *b = rmalloc(sizeof(int32_t));
                    *b = atoi(param);
                    procedure[counter].payload = (void *)b;
                    if(DEBUG) printf("\nlexer: PSA: Instr is BCONST. Currently assigning @ line [%d]. Program [%d].", (counter), procedure_counter);
                } else if(!strcmp(instr, "vconst")) {
                    procedure[counter].instr = CONCEPT_VCONST;
                    // if(!param_flag) exit(130);
                    // procedure[counter].payload = (void *)void;
                    if(DEBUG) printf("\nlexer: PSA: Instr is VCONST. Currently assigning @ line [%d]. Program [%d].", (counter), procedure_counter);
                } else if(!strcmp(instr, "print")) {
                    procedure[counter].instr = CONCEPT_PRINT;
                    if(DEBUG) printf("\nlexer: PSA: Instr is PRINT. Currently assigning @ line [%d]. Program [%d].", (counter), procedure_counter);
                } else if(!strcmp(instr, "pop")) {
                    procedure[counter].instr = CONCEPT_POP;
                    if(DEBUG) printf("\nlexer: PSA: Instr is POP. Currently assigning @ line [%d]. Program [%d].", (counter), procedure_counter);
                } else if(!strcmp(instr, "goto")) {
                    procedure[counter].instr = CONCEPT_GOTO;
                    if(!param_flag) exit(130);
                    int32_t goto_line_num = atoi(param);
                    int32_t *gif = go_to(goto_line_num);
                    procedure[counter].payload = (void *)gif;
                    if(DEBUG) printf("\nlexer: PSA: Instr is GOTO. Currently assigning @ line [%d]. Program [%d].", (counter), procedure_counter);
                } else if(!strcmp(instr, "if_icmple")) {
                    procedure[counter].instr = CONCEPT_IF_ICMPLE;
                    if(!param_flag) exit(130);
                    int32_t goto_line_num = atoi(param);
                    int32_t *gif = go_to(goto_line_num);
                    procedure[counter].payload = (void *)gif;
                    if(DEBUG) printf("\nlexer: PSA: Instr is IF_ICMPLE. Currently assigning @ line [%d]. Program [%d].", (counter), procedure_counter);
                } else if(!strcmp(instr, "call")) {
                    procedure[counter].instr = CONCEPT_CALL;
                    if(DEBUG) printf("\nlexer: PSA: Instr is CALL. Currently assigning @ line [%d]. Program [%d].", (counter), procedure_counter);
                    if(!param_flag) exit(130);
                    // perform an O(n) search to substitute in the actual position
                    int32_t *call_addr;
                    int32_t flag = 0;
                    for(int32_t m = 0; m < procedure_call_table_length; m++) {
                        if(!strcmp(param, procedure_call_table[m])) {
                            // That's the procedure we want!
                            call_addr = (int32_t *)rmalloc(sizeof(int32_t));
                            *call_addr = m;
                            if(DEBUG) printf("\n CALL: Procedure found, located @ %d.", m);
                            procedure[counter].payload = call_addr;
                            flag = 1;
                        }
                    }
                    if(!flag) { printf("Illegal call.\n"); exit(130); }
                } else if(!strcmp(instr, "gstore")) {
                    procedure[counter].instr = CONCEPT_GSTORE;
                    if(DEBUG) printf("\nlexer: PSA: Instr is GSTORE. Currently assigning @ line [%d]. Program [%d].", (counter), procedure_counter);
                } else if(!strcmp(instr, "gload")) {
                    procedure[counter].instr = CONCEPT_GLOAD;
                    if(DEBUG) printf("\nlexer: PSA: Instr is GLOAD. Currently assigning @ line [%d]. Program [%d].", (counter), procedure_counter);
                } else if(!strcmp(instr, "ret")) {
                    procedure[counter].instr = CONCEPT_RETURN;
                    if(DEBUG) printf("\nlexer: PSA: Instr is RET. Currently assigning @ line [%d]. Program [%d].", (counter), procedure_counter);
                } else if(!strcmp(instr, "inc")) {
                    procedure[counter].instr = CONCEPT_INC;
                    if(DEBUG) printf("\nlexer: PSA: Instr is INC. Currently assigning @ line [%d]. Program [%d].", (counter), procedure_counter);
                } else if(!strcmp(instr, "dec")) {
                    procedure[counter].instr = CONCEPT_DEC;
                    if(DEBUG) printf("\nlexer: PSA: Instr is DEC. Currently assigning @ line [%d]. Program [%d].", (counter), procedure_counter);
                } else if(!strcmp(instr, "dup")) {
                    procedure[counter].instr = CONCEPT_DUP;
                    if(DEBUG) printf("\nlexer: PSA: Instr is DUP. Currently assigning @ line [%d]. Program [%d].", (counter), procedure_counter);
                } else if(!strcmp(instr, "swap")) {
                    procedure[counter].instr = CONCEPT_SWAP;
                    if(DEBUG) printf("\nlexer: PSA: Instr is SWAP. Currently assigning @ line [%d]. Program [%d].", (counter), procedure_counter);
                } else if(!strcmp(instr, "halt")) {
                    procedure[counter].instr = CONCEPT_HALT;
                    if(DEBUG) printf("\nlexer: PSA: Instr is HALT. Currently assigning @ line [%d]. Program [%d].", (counter), procedure_counter);
                } else  {
                    printf("\n lexer:PSA: ERR: INVALID INSTR DETECTED > ABRT. Currently assigning @ line [%d]. Program [%d].", (counter), procedure_counter);
                    exit(130);
                } // ABRT

                counter ++;
            }

            // if
            counter = 0;
            compiled_bytecode_collection[procedure_counter] = procedure;
            procedure_counter++;
        }

        if(DEBUG) printf(ANSI_COLOR_RESET ANSI_COLOR_RED"\n\n CONGRADULATIONS! Successfully parsed everything into Bytecode. Starting the bytecode interpreter...\n"ANSI_COLOR_RESET);

        program = compiled_bytecode_collection;

        // return compiled_bytecode_collection;
    }
}


void run(char *arg) {
    // read in the program
    read_prog(arg);
    if(concept_program.code == NULL || concept_program.len == 0 || concept_program.len == -1)
        on_error(CONCEPT_COMPILER_ERROR, "Input program not found.",  CONCEPT_STATE_CATASTROPHE, CONCEPT_ABORT);

    if(DEBUG) {
        printf("\n-=-=-=-=-=-=-=-=Your Program Listings=-=-=-=-=-=-=-=-=-\n");
        for(int i = 0; i < concept_program.len; i++)
            printf("%s\n", concept_program.code[i]);
        printf("\n-=-=-=-=-=-=-=-=End  Program Listings=-=-=-=-=-=-=-=-=-\n");
    }

    // Allocate the two stacks
    // -=-=-=-=-=-=-=-=-=-=-=-
    // Two stacks are needed in order to simulate a Turing-complete machine in theoretical Computer Science.
    // The Turing machine defines a tape running through a conceptual machine with  two sides
    // which the machine can have RANDOM, COMPLETE/INFINITE memory access
    // One stack only simulates one side of the Turing machine.
    // We'll need two stacks on both sides in theory to gain the full potential of a 2xPDA which is Turing-Equivalent.
    // Here we allocate two stacks, one global stack and one instruction stack for future use.

    ConceptStack_t i_stack;
    ConceptStack_t f_stack;
    stack_alloc(&i_stack, (size_t)CONCEPTIP_MAX_LENGTH);
    stack_alloc(&f_stack, (size_t)CONCEPTFP_MAX_LENGTH); // TODO TODO TODO

    //ConceptBytecode_t **bytecodes = parse();
    //event_loop(bytecodes[0], &f_stack, &i_stack);

    parse_procedures();

    for(int i = 0; i < procedure_length_table_length; i++) {
        for(int j = 0; j < procedure_length_table[i]; j++) {
            int instr = program[i][j].instr;
            printf("\n%d\n", instr);
        }
    }

    eval(0, &f_stack, &i_stack, 0);

    cleanup(&i_stack);
    cleanup(&f_stack);
    memfree();
}

int32_t main(int32_t argc, char **argv) { // test codes here!
    if(argc == 2) run(argv[1]);
    else {
        printf("\n Conceptum \n");
        printf("Usage: ./cvm <code_file_path>\n");
        printf("Err: No input file specified. Exiting...");
    }
    return 0;
}