/*
 * main.c
 * Copyright (c) 2016 Ruijie Fang <ruijief@acm.org>
 * ALL RIGHTS RESERVED.
 */

#include <stdlib.h>

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

// Utils

// Conceptual Boolean

#define FALSE 0;
#define TRUE 1;
typedef int BOOL;

// Conceptual String

typedef struct {
    char (*value);
    int len;
} ConceptString_t;

// GCObject Definition (Temporary, TODO )

typedef void * GCObject;


// Conceptual Variable

typedef union {
    ConceptString_t s;
    int i;
    char c;
    float f;
    BOOL boo;
    GCObject gco;

} ConceptVar_t;

// Stack

typedef struct {
    int top;
    int size;
    ConceptVar_t (*operations);
} ConceptStack_t;

// Stack Operations

typedef struct {
    void (*alloc_ConceptStack)();
    void (*dealloc_ConceptStack)();
    void (*isfull_ConceptStack)();
} StackOps_t;

void realWorldConcept_alloc_ConceptStack(ConceptStack_t *stack, int size) {
    ConceptVar_t *stackContents = (ConceptVar_t *)malloc(sizeof(ConceptVar_t) * size);
    stack->operations = stackContents;
    stack->size = size;
    stack->top = (-1);

}

// Conceptual Processor Functions

typedef struct {
    int (*concept_iadd_func)(); // CONCEPT_IADD
    int (*concept_idiv_func)(); // CONCEPT_IDIV
    int (*concept_imul_func)(); // CONCEPT_IMUL
    float (*concept_fadd_func)(); // CONCEPT_FADD
    float (*concept_fdiv_func)(); // CONCEPT_FDIV
    float (*concept_fmul_func)(); // CONCEPT_FMUL
    BOOL (*concept_ilt_func)(); // CONCEPT_ILT
    BOOL (*concept_ieq_func)(); // CONCEPT_IEQ
    BOOL (*concept_flt_func)(); // CONCEPT_FLT
    BOOL (*concept_feq_func)(); // CONCEPT_FEQ
    void (*concept_cconst_func)(char); // CONCEPT_CCONST
    void (*concept_iconst_func)(int); // CONCEPT_ICONST
    void (*concept_sconst_func)(ConceptString_t); // CONCEPT_SCONST
    void (*concept_fconst_func)(float); // CONCEPT_FCONST
} ProcessorFuncs_t;

// init function
void initialize() {

}

// Processor

void concept_processor(void *bytecodes) {

}

int main(int argc, char **argv) {
    return 0; // TODO
}