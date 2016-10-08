/*
 * main.c
 * The "Conceptum" Turing-Complete VM source code file
 * Copyright (c) 2016 Ruijie Fang <ruijief@acm.org>
 * All versions released under The GNU General Public License v3.0.
 * A LICENSE Copy can be found in the project repository.
 * ALL RIGHTS RESERVED.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <float.h>

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
typedef struct{
    int32_t instruction;
    void *value; // if any
} ConceptBytecode_t;

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

// Conceptual Processor Functions
// TODO TODO TODO

// IADD Integer addition function
void concept_iadd(ConceptStack_t *stack) {
    int32_t type = CONCEPT_IADD; // for debugging purposes
    int32_t a = *((int32_t *)stack_pop(stack));
    int32_t b = *((int32_t *)stack_pop(stack)); // pop again for another value

    if(DEBUG) { // print DEBUG info
        printf("\nIADD\n");
        printf("\n%d\t", a);
        printf("%d\t", b);
    }

    if(a + b <= INT_MAX && a + b >= INT_MIN && !stack_is_full(stack)) {
        int32_t c = a + b;
        stack_push(stack, (void *)&c);

        if(DEBUG) {
            int32_t d = *((int *)stack_pop(stack));
            printf("\nIADD finished. Result %d", d);
            printf("\tMEM address: %p", &d);
            stack_push(stack, (void *)&d);
        }

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

    if(a / b <= INT_MAX && a / b >= INT_MIN && !stack_is_full(stack)) {
        int32_t c = a / b;
        stack_push(stack, &c);
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

    if(a * b <= INT_MAX && a * b >= INT_MIN && !stack_is_full(stack)) {
        int32_t c = a * b;
        stack_push(stack, &c);
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

    if(a + b <= FLT_MAX && a + b >= FLT_MIN && !stack_is_full(stack)) {
        int32_t c = a + b;
        stack_push(stack, &c);
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

    if(a / b <= FLT_MAX && a / b >= FLT_MIN && !stack_is_full(stack)) {
        int32_t c = a / b;
        stack_push(stack, &c);
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

    if(a * b <= FLT_MAX && a * b >= FLT_MIN && !stack_is_full(stack)) {
        int32_t c = a * b;
        stack_push(stack, &c);
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

    if(a < b && !stack_is_full(stack)) {
        int32_t c = TRUE;
        stack_push(stack, &c);
    } else {
        int32_t c = FALSE;
        stack_push(stack, &c);
    }
}

// IEQ Integer Equality comparison function
void concept_ieq(ConceptStack_t *stack) {
    int32_t type = CONCEPT_IEQ;
    int32_t a = *((int32_t *)stack_pop(stack));
    int32_t b = *((int32_t *)stack_pop(stack));

    if(a == b && !stack_is_full(stack)) {
        int32_t c = TRUE;
        stack_push(stack, &c);
    } else {
        int32_t c = FALSE;
        stack_push(stack, &c);
    }
}

// IGT Integer Greater Than comparison function
void concept_igt(ConceptStack_t *stack) {
    int32_t type = CONCEPT_IGT;
    int32_t a = *((int32_t *)stack_pop(stack));
    int32_t b = *((int32_t *)stack_pop(stack));

    if(a > b && !stack_is_full(stack)) {
        int32_t c = TRUE;
        stack_push(stack, &c);
    } else {
        int32_t c = FALSE;
        stack_push(stack, &c);
    }
}

// FLT Floating point Less Than comparison function
void concept_flt(ConceptStack_t *stack) {
    int32_t type = CONCEPT_FLT;
    float a = *((float *)stack_pop(stack));
    float b = *((float *)stack_pop(stack));

    if(a < b) {
        int32_t c = TRUE;
        stack_push(stack, &c);
    } else {
        int32_t c = FALSE;
        stack_push(stack, &c);
    }
}

// FEQ Floating point Equality comparison function
void concept_feq(ConceptStack_t *stack) {
    int32_t type = CONCEPT_FEQ;
    float a = *((float *)stack_pop(stack));
    float b = *((float *)stack_pop(stack));

    if(a == b) {
        int32_t c = TRUE;
        stack_push(stack, &c);
    } else {
        int32_t c = FALSE;
        stack_push(stack, &c);
    }
}

// FGT Floating point Greater Than comparison function
void concept_fgt(ConceptStack_t *stack) {
    int32_t type = CONCEPT_FGT;
    float a = *((float *)stack_pop(stack));
    float b = *((float *)stack_pop(stack));

    if(a > b) {
        int32_t c = TRUE;
        stack_push(stack, &c);
    } else {
        int32_t c = FALSE;
        stack_push(stack, &c);
    }
}

// TODO TODO TODO

// AND
void concept_and(ConceptStack_t *stack) {
    int32_t type = CONCEPT_AND;
    BOOL and;
    if(!stack_is_full(stack)) {
        and = (*(int32_t *)stack_pop(stack) & *(int32_t *)stack_pop(stack));
        stack_push(stack, &and);
    }
}

// OR
void concept_or(ConceptStack_t *stack) {
    int32_t type = CONCEPT_OR;
    BOOL or;
    if(!stack_is_full(stack)) {
        or = (*(int32_t *)stack_pop(stack) | *(int32_t *)stack_pop(stack));
        stack_push(stack, &or);
    }
}

// XOR
void concept_xor(ConceptStack_t *stack) {
    int32_t type = CONCEPT_XOR;
    BOOL xor;
    if(!stack_is_full(stack)) {
        int32_t p = *(int32_t *) stack_pop(stack);
        int32_t q = *(int32_t *) stack_pop(stack);
        xor = (p & (!q)) | ((!p) & q);
        stack_push(stack, &xor);
    }
}

// NE
void concept_ne(ConceptStack_t *stack) {
    int32_t type = CONCEPT_NE;
    BOOL ne;
    if(!stack_is_full(stack)) {
        ne = !(*(int32_t *)stack_pop(stack));
        stack_push(stack, &ne);
    }
}

// IF
void concept_if(ConceptStack_t *stack) {
    int32_t type = CONCEPT_IF;
    BOOL cp_if;
    if(!stack_is_full(stack)) {
        int32_t p = *(int32_t *) stack_pop(stack);
        int32_t q = *(int32_t *) stack_pop(stack);
        cp_if = ((!p) | q);
        stack_push(stack, &cp_if);
    }
}

void concept_cconst(ConceptStack_t *stack, char c) {
    int32_t type = CONCEPT_CCONST;
    stack_push(stack, &c);
}

void concept_iconst(ConceptStack_t *stack, int32_t i) {
    int32_t type = CONCEPT_ICONST;
    stack_push(stack, &i);
}

void concept_sconst(ConceptStack_t *stack, char *s) {
    int32_t type = CONCEPT_SCONST;
    stack_push(stack, &s);
}

void concept_fconst(ConceptStack_t *stack, float f) {
    int32_t type = CONCEPT_FCONST;
    stack_push(stack, &f);
}


void concept_bconst(ConceptStack_t *stack, BOOL b) {
    int32_t type = CONCEPT_BCONST;
    if(!stack_is_full(stack))
        stack_push(stack, &b);
}

void concept_vconst(ConceptStack_t *stack, void *v) {
    int32_t type = CONCEPT_VCONST;
    if(!stack_is_full(stack))
        stack_push(stack, &v);
}

void concept_print(ConceptStack_t *stack) {
    int32_t type = CONCEPT_PRINT;
    if(!stack_is_empty(stack))
        printf((char *)stack->operand_stack[stack->top]);
}

void * concept_pop(ConceptStack_t *stack) {
    int32_t type = CONCEPT_POP;
    void *val = stack_pop(stack);
    return val;
}

/*
 * File Reader Utilities and Lexer
 */
char **concept_file;

int32_t read_file(char *file_path, char *read_type) {
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t line_length;

    fp = fopen(file_path, read_type);
    if (fp == NULL)
        return CONCEPT_FILE_EMPTY;

    int32_t counter = 0;
    while ((line_length = getline(&line, &len, fp)) != -1) {
        //printf("Retrieved line of length %zu :\n", line_length);
        //printf("%s", line);

        concept_file[counter] = (char *)malloc(line_length);
        concept_file[counter] = line;

        counter ++;
    }

    fclose(fp);

    return 0;
}


void feeder() {
  // TODO
}

void preprocessor() {
  // TODO
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
            concept_iconst(stack, (int32_t)(cbp->value));
            break;
        case CONCEPT_SCONST:
            concept_sconst(stack, (char *)(cbp->value));
            break;
        case CONCEPT_FCONST:
            concept_fconst(stack, (*(float *)(cbp->value))); // http://stackoverflow.com/questions/15313658/void-is-literally-float-how-to-cast
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

int32_t main(int32_t argc, char **argv) { // test codes here!

    if(DEBUG) printf("\nConceptum Runtime DEBUG environment\n");

    ConceptStack_t stack_test;
    stack_alloc(&stack_test, 300);

    int32_t i = 8;
    int32_t j = 9;

    stack_push(&stack_test, (void *)&i);
    stack_push(&stack_test, (void *)&j);

    int32_t k = *((int32_t *)stack_pop(&stack_test));

    printf("\n%d\n", k);

    stack_push(&stack_test, (void *)&k);

    concept_iadd(&stack_test);

    int32_t n = *((int32_t *) (stack_pop(&stack_test)));
    printf("\n%d\n", n);

    return 0; // TODO
}