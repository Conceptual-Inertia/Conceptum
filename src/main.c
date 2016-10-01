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

#define CONCEPT_STATE_INFO 90
#define CONCEPT_STATE_WARNING 91
#define CONCEPT_STATE_ERROR 92
#define CONCEPT_STATE_CATASTROPHE 93
#define CONCEPT_WARN_NOEXIT 94
#define CONCEPT_WARN_EXITNOW 95
#define CONCEPT_NOWARNING_EXIT 96
#define CONCEPT_HALT 0
#define CONCEPT_ABORT 97

static int if_handles_exception(int if_exception) {
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

static void on_error(int error, char *msg, int action, int if_exception) {  // TODO TODO Add Memory free!!
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
typedef int BOOL;


// Conceptual String
typedef struct {
    char (*value);
    int len;
} ConceptString_t;


// Conceptual Stack
typedef struct {
    int top;
    int size;
    void *(*operand_stack);
} ConceptStack_t;

// Conceptual bytecode
typedef struct{
    int instruction;
    void *value; // if any
} ConceptBytecode_t;
/*
 * Stack Operations Functions
 */

// Allocate stack
static void stack_alloc(ConceptStack_t *stack, int bt_size) {
    // size of a void pointer * maximum size
    void *stackContents = malloc(sizeof(void *) * bt_size);
    stack->operand_stack = stackContents;
    stack->size = bt_size;
    stack->top = (-1);
}

// Deallocate (reset) stack
static void stack_dealloc(ConceptStack_t *stack) {
    // free objects stored in stack first
    for(int i = 0; i <= stack->top; i++) {
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
    // Exit when full
    if(stack_is_full(stack))
        on_error(CONCEPT_STACK_OVERFLOW, "Stack is full, operation abort.", CONCEPT_STATE_ERROR, CONCEPT_WARN_EXITNOW);

    // Push while incrementing top value
    stack->operand_stack[++stack->top] = content_ptr; // Increase by one BEFORE pushing

}

// Pop a content pointer out of the stack
static void* stack_pop(ConceptStack_t *stack) {
    if(stack_is_empty(stack)) {
        on_error(CONCEPT_GENERAL_ERROR, "Stack is empty. Returning a NULL.", CONCEPT_STATE_INFO, CONCEPT_WARN_NOEXIT);
        return NULL; // Nothing is stored yet!
    }
    return stack->operand_stack[stack->top--]; // Decrease by one AFTER popping
}

// Conceptual Processor Functions
// TODO TODO TODO

// IADD Integer addition function
void concept_iadd(ConceptStack_t *stack) {
    int type = CONCEPT_IADD; // for debugging purposes
    int a = *((int *)stack_pop(stack));
    int b = *((int *)stack_pop(stack)); // pop again for another value

    if(a + b <= INT_MAX && a + b >= INT_MIN && !stack_is_full(stack)) {
        int c = a + b;
        stack_push(stack, &c);
    } else {
        // Exceeds maximum limit, quit
        on_error(CONCEPT_BUFFER_OVERFLOW, "IADD Operation exceeds INT_MAX limit, Aborting...", CONCEPT_STATE_ERROR, CONCEPT_ABORT);
    }
}

// IDIV Integer division function
void concept_idiv(ConceptStack_t *stack) {
    int type = CONCEPT_IDIV;
    int a = *((int *)stack_pop(stack));
    int b = *((int *)stack_pop(stack)); // pop again for another value

    if(a / b <= INT_MAX && a / b >= INT_MIN && !stack_is_full(stack)) {
        int c = a / b;
        stack_push(stack, &c);
    } else {
        // Exceeds maximum limit, quit
        on_error(CONCEPT_BUFFER_OVERFLOW, "IDIV Operation exceeds INT_MAX limit, Aborting...", CONCEPT_STATE_ERROR, CONCEPT_ABORT);
    }
}

// IMUL Integer Multiplication function
void concept_imul(ConceptStack_t *stack) {
    int type = CONCEPT_IMUL;
    int a = *((int *)stack_pop(stack));
    int b = *((int *)stack_pop(stack)); // pop again for another value

    if(a * b <= INT_MAX && a * b >= INT_MIN && !stack_is_full(stack)) {
        int c = a * b;
        stack_push(stack, &c);
    } else {
        // Exceeds maximum limit, quit
        on_error(CONCEPT_BUFFER_OVERFLOW, "IMUL Operation exceeds INT_MAX limit, Aborting...", CONCEPT_STATE_ERROR, CONCEPT_ABORT);
    }
}


// FADD Floating point addition function
void concept_fadd(ConceptStack_t *stack) {
    int type = CONCEPT_FADD;
    float a = *((float *)stack_pop(stack));
    float b = *((float *)stack_pop(stack));

    if(a + b <= FLT_MAX && a + b >= FLT_MIN && !stack_is_full(stack)) {
        int c = a + b;
        stack_push(stack, &c);
    } else {
        // Exceeds maximum limit, quit
        on_error(CONCEPT_BUFFER_OVERFLOW, "FADD Operation exceeds INT_MAX limit, Aborting...", CONCEPT_STATE_ERROR, CONCEPT_ABORT);
    }
}

// FDIV Floating point division function
void concept_fdiv(ConceptStack_t *stack) {
    int type = CONCEPT_FDIV;
    float a = *((float *)stack_pop(stack));
    float b = *((float *)stack_pop(stack));

    if(a / b <= FLT_MAX && a / b >= FLT_MIN && !stack_is_full(stack)) {
        int c = a / b;
        stack_push(stack, &c);
    } else {
        // Exceeds maximum limit, quit
        on_error(CONCEPT_BUFFER_OVERFLOW, "FADD Operation exceeds INT_MAX limit, Aborting...", CONCEPT_STATE_ERROR, CONCEPT_ABORT);
    }
}

// FMUL Floating point multiplication function
void concept_fmul(ConceptStack_t *stack) {
    int type = CONCEPT_FMUL;
    float a = *((float *)stack_pop(stack));
    float b = *((float *)stack_pop(stack));

    if(a * b <= FLT_MAX && a * b >= FLT_MIN && !stack_is_full(stack)) {
        int c = a * b;
        stack_push(stack, &c);
    } else {
        // Exceeds maximum limit, quit
        on_error(CONCEPT_BUFFER_OVERFLOW, "FADD Operation exceeds INT_MAX limit, Aborting...", CONCEPT_STATE_ERROR, CONCEPT_ABORT);
    }
}


// ILT Integer Less Than comparison function
void concept_ilt(ConceptStack_t *stack) {
    int type = CONCEPT_ILT;
    int a = *((int *)stack_pop(stack));
    int b = *((int *)stack_pop(stack));

    if(a < b && !stack_is_full(stack)) {
        int c = TRUE;
        stack_push(stack, &c);
    } else {
        int c = FALSE;
        stack_push(stack, &c);
    }
}

// IEQ Integer Equality comparison function
void concept_ieq(ConceptStack_t *stack) {
    int type = CONCEPT_IEQ;
    int a = *((int *)stack_pop(stack));
    int b = *((int *)stack_pop(stack));

    if(a == b && !stack_is_full(stack)) {
        int c = TRUE;
        stack_push(stack, &c);
    } else {
        int c = FALSE;
        stack_push(stack, &c);
    }
}

// IGT Integer Greater Than comparison function
void concept_igt(ConceptStack_t *stack) {
    int type = CONCEPT_IGT;
    int a = *((int *)stack_pop(stack));
    int b = *((int *)stack_pop(stack));

    if(a > b && !stack_is_full(stack)) {
        int c = TRUE;
        stack_push(stack, &c);
    } else {
        int c = FALSE;
        stack_push(stack, &c);
    }
}

// FLT Floating point Less Than comparison function
void concept_flt(ConceptStack_t *stack) {
    int type = CONCEPT_FLT;
    float a = *((float *)stack_pop(stack));
    float b = *((float *)stack_pop(stack));

    if(a < b) {
        int c = TRUE;
        stack_push(stack, &c);
    } else {
        int c = FALSE;
        stack_push(stack, &c);
    }
}

// FEQ Floating point Equality comparison function
void concept_feq(ConceptStack_t *stack) {
    int type = CONCEPT_FEQ;
    float a = *((float *)stack_pop(stack));
    float b = *((float *)stack_pop(stack));

    if(a == b) {
        int c = TRUE;
        stack_push(stack, &c);
    } else {
        int c = FALSE;
        stack_push(stack, &c);
    }
}

// FGT Floating point Greater Than comparison function
void concept_fgt(ConceptStack_t *stack) {
    int type = CONCEPT_FGT;
    float a = *((float *)stack_pop(stack));
    float b = *((float *)stack_pop(stack));

    if(a > b) {
        int c = TRUE;
        stack_push(stack, &c);
    } else {
        int c = FALSE;
        stack_push(stack, &c);
    }
}

// TODO TODO TODO

// AND
void concept_and(ConceptStack_t *stack) {
    int type = CONCEPT_AND; // TODO
}

// OR
void concept_or(ConceptStack_t *stack) {
    int type = CONCEPT_OR; // TODO
}

// XOR
void concept_xor(ConceptStack_t *stack) {
    int type = CONCEPT_XOR; // TODO
}

// NE
void concept_ne(ConceptStack_t *stack) {
    int type = CONCEPT_NE; // TODO
}

// IF
void concept_if(ConceptStack_t *stack) {
    int type = CONCEPT_IF; // TODO
}

void concept_cconst(ConceptStack_t *stack, char c) {
    int type = CONCEPT_CCONST;
    stack_push(stack, &c);
}

void concept_iconst(ConceptStack_t *stack, int i) {
    int type = CONCEPT_ICONST;
    stack_push(stack, &i);
}

void concept_sconst(ConceptStack_t *stack, char *s) {
    int type = CONCEPT_SCONST;
    stack_push(stack, &s);
}

void concept_fconst(ConceptStack_t *stack, float f) {
    int type = CONCEPT_FCONST;
    stack_push(stack, &f);
}


void concept_bconst(ConceptStack_t *stack, BOOL b) {
    int type = CONCEPT_BCONST;
    if(!stack_is_full(stack))
        stack_push(stack, &b);
}

void concept_vconst(ConceptStack_t *stack, void *v) {
    int type = CONCEPT_VCONST;
    if(!stack_is_full(stack))
        stack_push(stack, &v);
}

void concept_print(ConceptStack_t *stack) {
    int type = CONCEPT_PRINT;
    if(!stack_is_empty)
        printf(stack->operand_stack[stack->top]);
}

void * concept_pop(ConceptStack_t *stack) {
    int type = CONCEPT_POP;
    void *val = stack_pop(stack);
    return val;
}

void feeder() {

}

void preprocessor() {

}

// Iterating event loop
void event_loop(ConceptBytecode_t *cbp, ConceptStack_t *stack) {
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
            concept_iconst(stack, (int)(cbp->value));
            break;
        case CONCEPT_SCONST:
            concept_sconst(stack, (char *)(cbp->value));
            break;
        case CONCEPT_FCONST:
            concept_fconst(stack, (float)(cbp->value));
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

int main(int argc, char **argv) {
    return 0; // TODO
}