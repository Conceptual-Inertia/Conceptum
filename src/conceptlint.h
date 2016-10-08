/*
 * conceptlint.h
 * Conceptual Lint Header
 * Copyright (C) Ruijie Fang <ruijief@acm.org> 2016.
 */

#ifndef CONCEPTLINT_H_
#define CONCEPTLINT_H_
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <ctype.h>
#include <float.h>
char* remove_spaces(char *src);
char* substring(char *string, int start, int end);
int is_void(char *s);
int is_int(char *tbd);
int is_char(char *tbd);
int is_float(char *tbd);

/*
int is_valid_func_def(char *tbd);
*/
#endif