
// Copyright (c) Alex Fang. LICENSE included in memman.h header file.

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <ctype.h>

#include "memman.h"
#define FIRSTRUN_STACK_DEPTH 50

void **reg;
int firstrun = 1;

// Global malloc() register
void memreg(void *ptr) {
    if(firstrun) {
        firstrun = 0;
        reg = malloc(sizeof(void*)*FIRSTRUN_STACK_DEPTH);
        return;
    }

    if(sizeof(reg) > sizeof(void*)*FIRSTRUN_STACK_DEPTH) {
        void **newreg = malloc(sizeof(reg) + sizeof(void *)); // a growing stack
        newreg = reg;
        newreg[(sizeof(newreg) / sizeof(void *)) - 1] = ptr;
        free(reg);
        reg = newreg;
    }
}

// Global deallocator for all dynamic memory
void memfree() {
    for(int i = 0; i < (sizeof(reg) / sizeof(void *)); i++) {
        free(reg[i]);
    }

    free(reg);
}

// wrapper function(s)

void* rmalloc(size_t size) {
    void *mem = malloc(size);
    memreg(mem);
    return mem;
}

void rfree(void *ptr) {
    for (int i = 0; i < (sizeof(reg) / sizeof(void *)); i++) {
        if (reg[i] == ptr) {
            void **newreg = malloc(sizeof(reg) - sizeof(void *));
            for (int i = 0; i < sizeof(reg) / sizeof(void *); i++) {
                if (reg[i] != ptr) newreg[i] = reg[i];
            }
            free(reg);
            reg = newreg;
        }
    }
    free(ptr);
}

void* rrealloc(void *ptr, size_t size) {
    ptr = realloc(ptr, size);
    memreg(ptr);
    return ptr;
}