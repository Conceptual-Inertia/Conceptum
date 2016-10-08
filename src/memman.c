/*
 * memman.c
 *
 * Dynamic Memory Management and GC
 * Copyright (C) Alex Fang <ruijief@acm.org> 2016
 */

#include "memman.h"

static void **reg;

// Global malloc() register
void memreg(void *ptr) {
    void** newreg = malloc(sizeof(reg) + sizeof(void *)); // a growing stack
    newreg = reg;
    newreg[(sizeof(newreg)/sizeof(void *)) - 1] = ptr;
    free(reg);
    reg = newreg;
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
    for(int i = 0; i < (sizeof(reg / sizeof(void *))); i++) {
        if(reg[i] == ptr) {
            void **newreg = malloc(sizeof(reg) - sizeof(void *));
            for(int i = 0; i < sizeof(reg) / sizeof(void *); i++) {
                if(reg[i] != ptr) newreg[i] = reg[i];
            }
            free(reg);
            reg = newreg;
        }
    }
    free(ptr);
}

void* rrealloc(void *ptr, size_t size) {
    realloc(ptr, size);
    memreg(ptr);
}