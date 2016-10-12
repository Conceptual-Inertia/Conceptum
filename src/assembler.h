
#ifndef CONCEPTLINT_H_
#define CONCEPTLINT_H_

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
#define CONCEPT_IF_ICMPLE 133 // if_icmple
#define CONCEPT_GOTO 134 // Goto Statement
#define CONCEPT_RETURN 135 // Return


#endif