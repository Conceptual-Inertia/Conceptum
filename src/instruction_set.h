
/*
 *
 * FANNGGOVITCH STANDARD BYTECODE FORMAT - .fng files
 *
 * COPYRIGHT (C) RUIJIE FANG <ruijief@acm.org> 2016
 * ALL RIGHTS RESERVED.
 *
 * PUBLISHED UNDER THE GNU GENERAL PUBLIC LICENSE v3.0.
 *
 * instruction set for the Conceptum & Inertia VM
 *
 * INSTRUCTION         |         DESCRIPTION
 * ====================+===========================
 * iadd                |           Integer Addition
 * isub                |           Integer Division
 * imul                |     Integer Multiplication
 *
 * fadd                            Float   Addition
 * fsub                |           Float   Division
 * fmul                |     Float   Multiplication
 *
 * ilt                 |          Integer Less Than
 * ieq                 |          Integer Equal  to
 *
 * flt                 |           Float  Less Than
 * feq                 |           Float Equal   to
 *
 * cconst              |           char const
 * iconst              |           integer const
 *
 * sconst              |           string const
 * fconst              |           Float const
 *
 *
 * ============= General Instructions ==============
 * call f()            |           Call function f()
 * ret                 |           return a value
 *
 * br a
 * brt a               |           Branch to a
 * brf a
 *
 * gload a             |           Load global a
 * gstore a            |           Store global a
 *
 * load i
 * store i
 *
 * fload i
 * fstore i
 *
 * print
 *
 * struct n
 *
 * pbear              | Prints an ASCII bear to stdout
 *
 * null               | The null value
 *
 * pop
 *
 * halt
 *
 * debug              | Allocates memory to explode
 *
 *
 */

/*
 * COMPILER GLOBALS
 */

#ifndef OPRND_STACK_SIZE
  #define OPRND_STACK_SIZE = 100
#endif

#ifnedf CALL_STACK_SIZE
  #define CALL_STACK_SIZE = 1000
#endif

static char * read_return_code() {

}
/*
 * An implementation of a stack
 */

#include "stack.h"

/*
 * An implementation of the register
 */

#include "register.h"

/*===============================================
 * BEGIN THE CONCEPTUM VM
 */



/*===============================================
 * BEGIN THE INERTIA VM
 */

/*===============================================
 * MISCELLANEOUS
 */

char * print_bear() {
    return "BEAR BEAR BEAR BEAR BEAR BEAR BEAR BEAR BEAR BEAR";
}