/*
 * main.c
 * Copyright (c) 2016 Ruijie Fang <ruijief@acm.org>
 * ALL RIGHTS RESERVED.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <float.h>

/*
 * custom libs
 */

//#include "instruction_set.h" // The Instruction Set

#define CONCEPT_IADD 100 // Integer Addition OUTPUT: Integer
#define CONCEPT_IDIV 101 // Integer Division OUTPUT: Integer
#define CONCEPT_IMUL 102 // Integer Multiplication OUTPUT: Integer

#define CONCEPT_FADD 103 // Float Addition OUTPUT: Float
#define CONCEPT_FDIV 104 // Float Division OUTPUT: Float
#define CONCEPT_FMUL 105 // Float Multiplication OUTPUT: Float

#define CONCEPT_ILT 106 // Integer Less Than OUTPUT: Boolean
#define CONCEPT_IEQ 107 // Integer Equal To OUTPUT: Boolean
#define CONCEPT_FLT 108 // Float Less Than OUTPUT: Boolean
#define CONCEPT_FEQ 109 // Float Equal To OUTPUT: Boolean

#define CONCEPT_CCONST 110 // Initialize Char Constant OUTPUT: Void
#define CONCEPT_ICONST 111 // Initialize Integer Constant OUTPUT: Void
#define CONCEPT_SCONST 112 // Initialize String Constant OUTPUT: Void
#define CONCEPT_FCONST 113 // Initialize Float COnstant OUTPUT: Void
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

int if_handles_exception(int if_exception) {
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

void on_error(int error, char *msg, int action, int if_exception) {  // TODO TODO Add Memory free!!
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


/*
 * Stack Operations Functions
 */

// Allocate stack
void stack_alloc(ConceptStack_t *stack, int bt_size) {
    // size of a void pointer * maximum size
    void *stackContents = malloc(sizeof(void *) * bt_size);
    stack->operand_stack = stackContents;
    stack->size = bt_size;
    stack->top = (-1);
}

// Deallocate (reset) stack
void stack_dealloc(ConceptStack_t *stack) {
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
void stack_free(ConceptStack_t *stack) {
    // Deallocate the stack's contents and properties
    stack_dealloc(stack);
    // Free everything
    free(stack);
}

// Check if stack is empty TRUE: empty FALSE: not empty
BOOL stack_is_empty(ConceptStack_t *stack) {
    return (stack->top == (-1));
}

// Check if stack is full TRUE: full FALSE: not full
BOOL stack_is_full(ConceptStack_t *stack) {
    return (stack->top >= stack->size - 1);
}

// Push a content pointer into stack
void stack_push(ConceptStack_t *stack, void *content_ptr) {
    // Exit when full
    if(stack_is_full(stack))
        on_error(CONCEPT_STACK_OVERFLOW, "Stack is full, operation abort.", CONCEPT_STATE_ERROR, CONCEPT_WARN_EXITNOW);

    // Push while incrementing top value
    stack->operand_stack[++stack->top] = content_ptr; // Increase by one BEFORE pushing

}

void* stack_pop(ConceptStack_t *stack) {
    if(stack_is_empty(stack)) {
        on_error(CONCEPT_GENERAL_ERROR, "Stack is empty. Returning a NULL.", CONCEPT_STATE_INFO, CONCEPT_WARN_NOEXIT);
        return NULL; // Nothing is stored yet!
    }
    return stack->operand_stack[stack->top--]; // Decrease by one AFTER popping
}

// Conceptual Processor Functions
// TODO TODO TODO

void concept_iadd(ConceptStack_t *stack) {
    int a = *((int *)stack_pop(stack));
    int b = *((int *)stack_pop(stack)); // pop again for another value

    if(a + b <= INT_MAX && a + b >= INT_MIN) {
        int c = a + b;
        stack_push(stack, &c);
    } else {
        // Exceeds maximum limit, quit
        on_error(CONCEPT_BUFFER_OVERFLOW, "IADD Operation exceeds INT_MAX limit, Aborting...", CONCEPT_STATE_ERROR, CONCEPT_ABORT);
    }
}

void concept_idiv(ConceptStack_t *stack) {
    int a = *((int *)stack_pop(stack));
    int b = *((int *)stack_pop(stack)); // pop again for another value

    if(a / b <= INT_MAX && a / b >= INT_MIN) {
        int c = a / b;
        stack_push(stack, &c);
    } else {
        // Exceeds maximum limit, quit
        on_error(CONCEPT_BUFFER_OVERFLOW, "IDIV Operation exceeds INT_MAX limit, Aborting...", CONCEPT_STATE_ERROR, CONCEPT_ABORT);
    }
}

void concept_imul(ConceptStack_t *stack) {
    int a = *((int *)stack_pop(stack));
    int b = *((int *)stack_pop(stack)); // pop again for another value

    if(a * b <= INT_MAX && a * b >= INT_MIN) {
        int c = a * b;
        stack_push(stack, &c);
    } else {
        // Exceeds maximum limit, quit
        on_error(CONCEPT_BUFFER_OVERFLOW, "IMUL Operation exceeds INT_MAX limit, Aborting...", CONCEPT_STATE_ERROR, CONCEPT_ABORT);
    }
}

void concept_fadd(ConceptStack_t *stack) {
    float a = *((float *)stack_pop(stack));
    float b = *((float *)stack_pop(stack));

    if(a + b <= FLT_MAX && a + b >= FLT_MIN) {
        int c = a + b;
        stack_push(stack, &c);
    } else {
        // Exceeds maximum limit, quit
        on_error(CONCEPT_BUFFER_OVERFLOW, "FADD Operation exceeds INT_MAX limit, Aborting...", CONCEPT_STATE_ERROR, CONCEPT_ABORT);
    }
}

void concept_fdiv(ConceptStack_t *stack) {
    float a = *((float *)stack_pop(stack));
    float b = *((float *)stack_pop(stack));

    if(a / b <= FLT_MAX && a / b >= FLT_MIN) {
        int c = a / b;
        stack_push(stack, &c);
    } else {
        // Exceeds maximum limit, quit
        on_error(CONCEPT_BUFFER_OVERFLOW, "FADD Operation exceeds INT_MAX limit, Aborting...", CONCEPT_STATE_ERROR, CONCEPT_ABORT);
    }
}

void concept_fmul(ConceptStack_t *stack) {
    float a = *((float *)stack_pop(stack));
    float b = *((float *)stack_pop(stack));

    if(a * b <= FLT_MAX && a * b >= FLT_MIN) {
        int c = a * b;
        stack_push(stack, &c);
    } else {
        // Exceeds maximum limit, quit
        on_error(CONCEPT_BUFFER_OVERFLOW, "FADD Operation exceeds INT_MAX limit, Aborting...", CONCEPT_STATE_ERROR, CONCEPT_ABORT);
    }
}

void concept_ilt(ConceptStack_t *stack) {
    int a = *((int *)stack_pop(stack));
    int b = *((int *)stack_pop(stack));

    if(a < b) {
        int c = TRUE;
        stack_push(stack, &c);
    } else {
        int c = FALSE;
        stack_push(stack, &c);
    }
}

void concept_ieq(ConceptStack_t *stack) {
    int a = *((int *)stack_pop(stack));
    int b = *((int *)stack_pop(stack));

    if(a == b) {
        int c = TRUE;
        stack_push(stack, &c);
    } else {
        int c = FALSE;
        stack_push(stack, &c);
    }
}

void concept_flt(ConceptStack_t *stack) {
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

void concept_feq(ConceptStack_t *stack) {
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

void concept_cconst(ConceptStack_t *stack, char c) {
    stack_push(stack, &c);
}

void concept_iconst(ConceptStack_t *stack, int i) {
    stack_push(stack, &i);
}

void concept_sconst(ConceptStack_t *stack, char *s) {
    stack_push(stack, &s);
}

void concept_fconst(ConceptStack_t *stack, float f) {
    stack_push(stack, &f);
}

void event_loop() {
    // TODO TODO TODO
}

int main(int argc, char **argv) {
    return 0; // TODO
}