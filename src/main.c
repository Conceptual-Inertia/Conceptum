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
#include <unistd.h>
#include <limits.h>
#include <float.h>
#include <string.h>
#include <ctype.h>

#include "memman.h"
#include "conceptlint.h"

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
#define CONCEPT_IF  116 // Boolean IF  OUTPUT: Boolean // TODO

#define CONCEPT_CCONST 117 // Initialize Char Constant OUTPUT: Void
#define CONCEPT_ICONST 118 // Initialize Integer Constant OUTPUT: Void
#define CONCEPT_SCONST 119 // Initialize String Constant OUTPUT: Void
#define CONCEPT_FCONST 120 // Initialize Float Constant OUTPUT: Void
#define CONCEPT_BCONST 121 // Initialize Boolean Constant OUTPUT: Void
#define CONCEPT_VCONST 122 // Initialize Void Constant OUTPUT: Void

#define CONCEPT_PROCEDURE 123 // Initialize Concept Function OUTPUT: Void
#define CONCEPT_PRINT 124 // Print to stdout OUTPUT: Void
#define CONCEPT_CALL 125 // Call a procedure(void *)
#define CONCEPT_LOAD 126 // Load value
#define CONCEPT_STORE 127 // Store value
#define CONCEPT_FLOAD 128 // Load float value
#define CONCEPT_FSTORE 129 // Store float value
#define CONCEPT_GLOAD 130 // Load global value
#define CONCEPT_GSTORE 131 // Store global value
#define CONCEPT_POP 132 // Pop a value out of stack

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

// Conceptual bytecode
typedef struct {
    int32_t instruction;
    void *value; // if any
} ConceptBytecode_t;

struct {
    char **code;
    int len;
} concept_program;

BOOL is_running;

/*
 * Stack Operations Functions
 */

// Allocate stack
static void stack_alloc(ConceptStack_t *stack, int32_t bt_size) {
    // size of a void pointer * maximum size
    void *stackContents = malloc(sizeof(void *) * bt_size);
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
    int32_t type = CONCEPT_IADD; // for debugging purposes
    int32_t a = *((int32_t *)stack_pop(stack));
    int32_t b = *((int32_t *)stack_pop(stack)); // pop again for another value

    if(DEBUG) { // print DEBUG info
        printf("\nIADD\n");
        printf("\t%d\tPLUS\t", a);
        printf("%d", b);
    }

    int32_t *c = malloc(sizeof(int32_t));

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
    int32_t type = CONCEPT_IDIV;
    int32_t a = *((int32_t *)stack_pop(stack));
    int32_t b = *((int32_t *)stack_pop(stack)); // pop again for another value

    if(DEBUG) { // print DEBUG info
        printf("\nIDIV\n");
        printf("\t%d\tDIVBY\t", a);
        printf("%d", b);
    }

    int32_t *c = malloc(sizeof(int32_t));

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
    int32_t type = CONCEPT_IMUL;
    int32_t a = *((int32_t *)stack_pop(stack));
    int32_t b = *((int32_t *)stack_pop(stack)); // pop again for another value

    if(DEBUG) { // print DEBUG info
        printf("\nIMUL\n");
        printf("\t%d\tTIMES\t", a);
        printf("%d", b);
    }

    int32_t *c = malloc(sizeof(int32_t));

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
    int32_t type = CONCEPT_FADD;
    float a = *((float *)stack_pop(stack));
    float b = *((float *)stack_pop(stack));

    if(DEBUG) { // print DEBUG info
        printf("\nFADD\n");
        printf("\t%f\tPLUS\t", a);
        printf("%f", b);
    }

    float *c = malloc(sizeof(float));

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
    int32_t type = CONCEPT_FDIV;
    float a = *((float *)stack_pop(stack));
    float b = *((float *)stack_pop(stack));

    if(DEBUG) { // print DEBUG info
        printf("\nFDIV\n");
        printf("\t%f\tDIVBY\t", a);
        printf("%f", b);
    }

    float *c = malloc(sizeof(float));

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
    int32_t type = CONCEPT_FMUL;
    float a = *((float *)stack_pop(stack));
    float b = *((float *)stack_pop(stack));

    if(DEBUG) { // print DEBUG info
        printf("\nFMUL\n");
        printf("\t%f\tTIMES\t", a);
        printf("%f", b);
    }

    float *c = malloc(sizeof(float));

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
    int32_t type = CONCEPT_ILT;
    int32_t a = *((int32_t *)stack_pop(stack));
    int32_t b = *((int32_t *)stack_pop(stack));

    if(DEBUG) { // print DEBUG info
        printf("\nILT\n");
        printf("\t%d\tLESSTHAN\t", a);
        printf("%d", b);
    }

    int32_t *c = malloc(sizeof(int32_t));

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
    int32_t type = CONCEPT_IEQ;
    int32_t a = *((int32_t *)stack_pop(stack));
    int32_t b = *((int32_t *)stack_pop(stack));

    if(DEBUG) { // print DEBUG info
        printf("\nIEQ\n");
        printf("\t%d\tEQUALS\t", a);
        printf("%d", b);
    }

    int32_t *c = malloc(sizeof(int32_t));

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
    int32_t type = CONCEPT_IGT;
    int32_t a = *((int32_t *)stack_pop(stack));
    int32_t b = *((int32_t *)stack_pop(stack));

    if(DEBUG) { // print DEBUG info
        printf("\nIGT\n");
        printf("\t%d\tGTRTHAN\t", a);
        printf("%d", b);
    }

    int32_t *c = malloc(sizeof(int32_t));

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
    int32_t type = CONCEPT_FLT;
    float a = *((float *)stack_pop(stack));
    float b = *((float *)stack_pop(stack));

    if(DEBUG) { // print DEBUG info
        printf("\nFLT\n");
        printf("\t%f\tLESSTHAN\t", a);
        printf("%f", b);
    }

    int32_t *c = malloc(sizeof(float)); // int32_t used for boolean value, NOT FLOAT!

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
    int32_t type = CONCEPT_FEQ;
    float a = *((float *)stack_pop(stack));
    float b = *((float *)stack_pop(stack));

    if(DEBUG) { // print DEBUG info
        printf("\nFEQ\n");
        printf("\t%f\tEQUALS\t", a);
        printf("%f", b);
    }

    int32_t *c = malloc(sizeof(int32_t));

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
    int32_t type = CONCEPT_FGT;
    float a = *((float *)stack_pop(stack));
    float b = *((float *)stack_pop(stack));

    if(DEBUG) { // print DEBUG info
        printf("\nFGT\n");
        printf("\t%f\tGRTHAN\t", a);
        printf("%f", b);
    }

    int32_t *c = malloc(sizeof(int32_t));

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
    int32_t type = CONCEPT_AND;

    if(DEBUG) printf("\nAND");

    BOOL *and = malloc(sizeof(BOOL));
    if(!stack_is_full(stack)) {
        *and = (*(int32_t *)stack_pop(stack) & *(int32_t *)stack_pop(stack));
        stack_push(stack, (void *)and);
    }

    if(DEBUG) printf("\nAND finished, RESULT %d\taddr %p", *and, and);
}

// OR
void concept_or(ConceptStack_t *stack) {
    int32_t type = CONCEPT_OR;

    if(DEBUG) printf("\nOR");

    BOOL *or = malloc(sizeof(BOOL));
    if(!stack_is_full(stack)) {
        *or = (*(int32_t *)stack_pop(stack) | *(int32_t *)stack_pop(stack));
        stack_push(stack, (void *)or);
    }

    if(DEBUG) printf("\nOR finished, RESULT %d\taddr %p", *or, or);
}

// XOR
void concept_xor(ConceptStack_t *stack) {
    int32_t type = CONCEPT_XOR;

    int32_t p = *(int32_t *) stack_pop(stack);
    int32_t q = *(int32_t *) stack_pop(stack);

    if(DEBUG) printf("\nXOR (%d XOR %d)", p, q);

    BOOL *xor = malloc(sizeof(BOOL));
    if(!stack_is_full(stack)) {
        *xor = (p & (!q)) | ((!p) & q);
        stack_push(stack, (void *)xor);
    }

    if(DEBUG) printf("\nXOR finished, RESULT %d\taddr %p", *xor, xor);
}

// NE
void concept_ne(ConceptStack_t *stack) {
    int32_t type = CONCEPT_NE;

    int32_t p = (*(int32_t *)stack_pop(stack));

    if(DEBUG) printf("\nNE (!%d)", p);

    BOOL *ne = malloc(sizeof(BOOL));
    if(!stack_is_full(stack)) {
        *ne = (!p);
        stack_push(stack, (void *)ne);
    }

    if(DEBUG) printf("\nNE finished, RESULT %d\taddr %p", *ne, ne);
}

// IF
void concept_if(ConceptStack_t *stack) {
    int32_t type = CONCEPT_IF;

    int32_t p = *(int32_t *) stack_pop(stack);
    int32_t q = *(int32_t *) stack_pop(stack);

    if(DEBUG) printf("\nIF(Boolean Algebra Operation), %d->%d", p, q);

    BOOL *cp_if = malloc(sizeof(BOOL));
    if(!stack_is_full(stack)) {
        *cp_if = ((!p) | q);
        stack_push(stack, (void *)cp_if);
    }

    if(DEBUG) printf("\nIF (Boolean Algebra Operation) finished, RESULT %d\taddr %p", *cp_if, cp_if);
}

void concept_cconst(ConceptStack_t *stack, char c) {
    int32_t type = CONCEPT_CCONST;

    if(DEBUG) printf("\nCCONST %c", c);

    char *c_ptr = malloc(sizeof(char)); // Prevent space from being collected

    *c_ptr = c;

    stack_push(stack, (void *)c_ptr);
}

void concept_iconst(ConceptStack_t *stack, int32_t i) {
    int32_t type = CONCEPT_ICONST;

    if(DEBUG) printf("\nICONST %d", i);

    int32_t *i_ptr = malloc(sizeof(int32_t));
    *i_ptr = i;

    stack_push(stack, (void *)i_ptr);
}

void concept_sconst(ConceptStack_t *stack, char *s) {
    int32_t type = CONCEPT_SCONST;

    if(DEBUG) {
        printf("\nSCONST\n");
        printf("Dumped Contents\n");
        printf("-=-=-=-=-=-=-=-=-\n");
        printf("%s", s);
        printf("\n\n");
    }

    char **s_ptr = malloc(sizeof(s)); // better than sizeof char*
    *s_ptr = s;

    stack_push(stack, (void *)s_ptr); // pointers, pointers, pointers dreaded POINTERS!!!!
}

void concept_fconst(ConceptStack_t *stack, float f) {
    int32_t type = CONCEPT_FCONST;

    if(DEBUG) printf("\nFCONST %f", f);

    float *f_ptr = malloc(sizeof(float));
    *f_ptr = f;

    stack_push(stack, (void *)f_ptr);
}


void concept_bconst(ConceptStack_t *stack, BOOL b) {
    int32_t type = CONCEPT_BCONST;

    if(DEBUG) printf("\nBCONST %d", b);

    BOOL *b_ptr = malloc(sizeof(BOOL));
    *b_ptr = b;
    if(!stack_is_full(stack))
        stack_push(stack, (void *)b_ptr);
}

void concept_vconst(ConceptStack_t *stack, void *v) {
    int32_t type = CONCEPT_VCONST;

    if(DEBUG) printf("\nVCONST bla bla bla... @ addr %p", v);

    // gonna be very ugly!

    void **v_ptr = malloc(sizeof(v));
    *v_ptr = v;

    if(!stack_is_full(stack))
        stack_push(stack, (void *)v_ptr);
}

void concept_print(ConceptStack_t *stack) {
    int32_t type = CONCEPT_PRINT;
    if(!stack_is_empty(stack))
        printf("%s", ((char *)stack->operand_stack[stack->top]));
}

void * concept_pop(ConceptStack_t *stack) {
    int32_t type = CONCEPT_POP;
    void *val = stack_pop(stack);
    return val;
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
}


/*
 * File Reader Utilities and Lexer
 */


char** read_prog(char *file_path) {
    int lines_allocated = 128;
    int max_line_len = 100;

    /* Allocate lines of text */
    char **words = (char **)malloc(sizeof(char*)*lines_allocated);
    if (words==NULL) {
        fprintf(stderr,"Out of memory (1).\n");
        exit(1);
    }

    FILE *fp = fopen(file_path, "r");
    if (fp == NULL) {
        fprintf(stderr,"Error opening file.\n");
        exit(2);
    }

    int i;
    for (i=0;1;i++) {
        int j;

        /* Have we gone over our line allocation? */
        if (i >= lines_allocated) {
            int new_size;

            /* Double our allocation and re-allocate */
            new_size = lines_allocated*2;
            words = (char **)realloc(words,sizeof(char*)*new_size);
            if (words==NULL) {
                fprintf(stderr,"Out of memory.\n");
                exit(3);
            }
            lines_allocated = new_size;
        }
        /* Allocate space for the next line */
        words[i] = malloc(max_line_len);
        if (words[i]==NULL) {
            fprintf(stderr,"Out of memory (3).\n");
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

    return words;

    //int j;
    //for(j = 0; j < i; j++)
    //    printf("%s\n", words[j]);

    /* Good practice to free memory */
    //for (;i>=0;i--)
    //    free(words[i]);
    //free(words);
    //return 0;
}

// Iterating event loop
void event_loop(ConceptBytecode_t *cbp, ConceptStack_t *stack) { // TODO
    switch(cbp->instruction) {
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
            concept_cconst(stack, (char)(cbp->value));
            break;
        case CONCEPT_ICONST:
            concept_iconst(stack, (*(int32_t *)(cbp->value)));
            break;
        case CONCEPT_SCONST:
            concept_sconst(stack, (char *)(cbp->value));
            break;
        case CONCEPT_FCONST:
            concept_fconst(stack, (*(float *)(cbp->value)));
            break;
        case CONCEPT_BCONST:
            concept_bconst(stack, (BOOL)(cbp->value));
            break;
        case CONCEPT_VCONST:
            concept_vconst(stack, cbp->value);
            break;
        case CONCEPT_PRINT:
            concept_print(stack);
            break;
        case CONCEPT_POP:
            concept_pop(stack);
            break;
        default:
            break; // do nothing
    }
}

void run() {

}

int32_t main(int32_t argc, char **argv) { // test codes here!

    return 0; // TODO
}