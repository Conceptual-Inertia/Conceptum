
#include "assembler.h"

#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <ctype.h>
#include <float.h>
#include <stdio.h>
#include "memman.h"
#include "conceptlint.h"

typedef struct {
    int32_t instr;
    void *payload;
} ConceptInstruction_t;

typedef struct {
    int32_t h_index;

} ConceptProgram_t;

struct RawProgram_t {
    char **code;
    int len;
} concept_program;



char* substring(char *string, int start, int end) {
    char* subbuff = malloc(sizeof(char) * (end - start));
    memcpy(subbuff, &string[start], (end - start));
    subbuff[end - start] = '\0';
    return subbuff;
}

void read_prog(char *file_path) {
    int lines_allocated = 128;
    int max_line_len = 100;

    /* Allocate lines of text */
    char **words = (char **)rmalloc(sizeof(char*)*lines_allocated);
    if (words==NULL) {
        fprintf(stderr,"Out of memory (1).\n");
        exit(1);
    }

    FILE *fp = fopen(file_path, "r");
    if (fp == NULL) {
        fprintf(stderr,"Error opening file.\n");
        exit(2);
    }

    int i;
    for (i=0;1;i++) {
        int j;

        /* Have we gone over our line allocation? */
        if (i >= lines_allocated) {
            int new_size;

            /* Double our allocation and re-allocate */
            new_size = lines_allocated*2;
            words = (char **)rrealloc(words,sizeof(char*)*new_size);
            if (words==NULL) {
                fprintf(stderr,"err read_file(): Out of memory.\n");
                exit(3);
            }
            lines_allocated = new_size;
        }
        /* Allocate space for the next line */
        words[i] = rmalloc(max_line_len);
        if (words[i]==NULL) {
            fprintf(stderr,"err read_file(): Out of memory (3).\n");
            exit(4);
        }
        if (fgets(words[i],max_line_len-1,fp)==NULL)
            break;

        /* Get rid of CR or LF at end of line */
        for (j=strlen(words[i])-1;j>=0 && (words[i][j]=='\n' || words[i][j]=='\r');j--);
        ; ; ;
        words[i][j+1]='\0';
    }
    /* Close file */
    fclose(fp);

    // words: string array
    // Every string in the array represents a line in the actual file.
    // The array itself represents the text file composed of multiple line(s).
    // The length here, (i) means the length of the string array (since a pointer of pointer can not be measured)

    concept_program.code = words;
    concept_program.len = i;

    //int j;
    //for(j = 0; j < i; j++)
    //    printf("%s\n", words[j]);

    /* Good practice to free memory */
    //for (;i>=0;i--)
    //    free(words[i]);
    //free(words);
    //return 0;
}
char **procedure_call_table;
// 1

// parse_procedures() reads in line by line, and finds the line declaring a procedure.
// After that the procedure is being parsed in to an array of linear bytecodes
// After that a bytecode array is constructed
// A similar array of strings ^^ procedure_call_table is also constructed in order to map the value of a function to its address in the bytecode
// array
// TODO: finally the interpreter will perform inline expansion on all calls to make the destination procedure's name NOT
// the String name, but the ACTUAL address of the bytecode procedure, which in turn makes an O(n) + O(1) complexity an O(1) complexity
void parse_procedures() {
   for(int j = 0; j < concept_program.len; j++) {
       if(strstr(concept_program.code[j], "procedure")) {
           ConceptInstruction_t *procedure; // ConceptInstruction_t
           int counter = 0;
           int i = j;
           for(j; !strstr(concept_program.code[j], "ret"); j++);
           int procedure_len = j - i;
           char *prog_name_line = concept_program.code[i];


           procedure = (ConceptInstruction_t *)rmalloc(procedure_len * sizeof(ConceptInstruction_t)); // including the return statement

           for(i = i + 1; i <= j; i++) { // from the first line of program to the ret statement, read every line and parse
               // parse, parse, parse!
               char *s_line = concept_program.code[i];
               int p;
               for(p = 0; p < strlen(s_line) && s_line[p] != ' ' && s_line[p] != '\t'; p++);
               char *instr = (char *)rmalloc(sizeof(char) * p);
               for(int q = 0; q < p; q++) instr[q] = s_line[q];
               int param_flag = 0;
               char *param;
               if(strlen(s_line) - p > 0) {
                   param_flag = 1;
                   param = (char *)rmalloc(sizeof(char)*(strlen(s_line)-p));
                   int r = 0;
                   for(p = p + 1; p < strlen(s_line); p++) {
                       param[r] = s_line[p];
                       r++;
                   }
               }

               // The advent of a gigantic if... C switches doesn't support char*
               if(!strcmp(instr, "iadd")) procedure[i].instr = CONCEPT_IADD;
               else if(!strcmp(instr, "idiv")) procedure[i].instr = CONCEPT_IDIV;
               else if(!strcmp(instr, "imul")) procedure[i].instr = CONCEPT_IMUL;
               else if(!strcmp(instr, "fadd")) procedure[i].instr = CONCEPT_FADD;
               else if(!strcmp(instr, "fdiv")) procedure[i].instr = CONCEPT_FDIV;
               else if(!strcmp(instr, "fmul")) procedure[i].instr = CONCEPT_FMUL;
               else if(!strcmp(instr, "ilt")) procedure[i].instr = CONCEPT_ILT;
               else if(!strcmp(instr, "ieq")) procedure[i].instr = CONCEPT_IEQ;
               else if(!strcmp(instr, "igt")) procedure[i].instr = CONCEPT_IGT;
               else if(!strcmp(instr, "flt")) procedure[i].instr = CONCEPT_FLT;
               else if(!strcmp(instr, "feq")) procedure[i].instr = CONCEPT_FEQ;
               else if(!strcmp(instr, "fgt")) procedure[i].instr = CONCEPT_FGT;
               else if(!strcmp(instr, "and")) procedure[i].instr = CONCEPT_AND;
               else if(!strcmp(instr, "or")) procedure[i].instr = CONCEPT_OR;
               else if(!strcmp(instr, "xor")) procedure[i].instr = CONCEPT_XOR;
               else if(!strcmp(instr, "ne")) procedure[i].instr = CONCEPT_NE;
               else if(!strcmp(instr, "if")) procedure[i].instr = CONCEPT_IF;
               else if(!strcmp(instr, "cconst")) {
                   procedure[i].instr = CONCEPT_CCONST;
                   if(!param_flag) exit(130);
                   char *c = rmalloc(sizeof(char));
                   *c = param[0];
                   procedure[i].payload = (void *)c;
               }
               else if(!strcmp(instr, "iconst")) {
                   procedure[i].instr = CONCEPT_ICONST;
                   int32_t *a = rmalloc(sizeof(int));
                   *a = atoi(param);
                   procedure[i].payload = (void *)a;
               }
               else if(!strcmp(instr, "sconst")) {
                   procedure[i].instr = CONCEPT_SCONST;
                   procedure[i].payload = (void *)param;
               }
               else if(!strcmp(instr, "fconst")) {
                   procedure[i].instr = CONCEPT_FCONST;
                   float *f = rmalloc(sizeof(float));
                   *f = (float)atof(param);
                   procedure[i].payload = (void *)f;
               }
               else if(!strcmp(instr, "bconst")) {
                   procedure[i].instr = CONCEPT_BCONST;
                   int32_t *b = rmalloc(sizeof(int32_t));
                   *b = atoi(param);
                   procedure[i].payload = (void *)b;
               }
               else if(!strcmp(instr, "vconst")) {
                   procedure[i].instr = CONCEPT_VCONST;
                   procedure[i].payload = (void *)void;
               }
               else if(!strcmp(instr, "print")) {
                   procedure[i].instr = CONCEPT_PRINT;
               }
               else if(!strcmp(instr, "pop")) {
                   procedure[i].instr = CONCEPT_POP;
               }
               else if(!strcmp(instr, "goto")) {
                   procedure[i].instr = CONCEPT_GOTO;
                   int32_t *goto_line_num = rmalloc(sizeof(int32_t));
                   *goto_line_num = atoi(param);
                   procedure[i].payload = goto_line_num;
               }
               else if(!strcmp(instr, "if_icmple")) {
                   procedure[i].instr = CONCEPT_IF_ICMPLE;
                   int32_t *goto_line_num = rmalloc(sizeof(int32_t));
                   *goto_line_num = atoi(param);
                   procedure[i].payload = goto_line_num;
               }
               else if(!strcmp(instr, "call")) {
                   procedure[i].instr = CONCEPT_CALL;
               }
               else if(!strcmp(instr, "gstore")) procedure[i].instr = CONCEPT_GSTORE;
               else if(!strcmp(instr, "gload")) procedure[i].instr = CONCEPT_GLOAD;
               else exit(130); // ABRT

           }
       }
   }
}
