/*
 * memman.h
 *
 * Dynamic Memory Management and GC
 * Copyright (C) Alex Fang <ruijief@acm.org> 2016
 */

#ifndef MEMMAN_H_
#define MEMMAN_H_

#include <stdlib.h>

/**
 *
 * @param ptr void*
 * @return void
 */
void memreg(void *ptr);
/**
 * @return void
 */
void memfree();
/**
 *
 * @param size size_t
 * @return void*
 */
void* rmalloc(size_t size);
/**
 *
 * @param ptr void*
 * @reutrn void
 */
void rfree(void *ptr);
/**
 *
 * @param ptr void*
 * @return void*
 */
void* rrealloc(void *ptr, size_t size);
#endif