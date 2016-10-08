/*
 * conceptlint.c
 * Conceptual Lint File
 * Copyright (C) Ruijie Fang <ruijief@acm.org> 2016.
 */

#include "conceptlint.h"
char* remove_spaces(char *src){
    char *dst;
    int s, d=0;
    for (s=0; src[s] != 0; s++)
        if (src[s] != ' ' && src[s] != '\t') {
            dst[d] = src[s];
            d++;
        }
    dst[d] = 0;
    return dst;
}

char* substring(char *string, int start, int end) {
    char* subbuff = malloc(sizeof(char) * (end - start));
    memcpy(subbuff, &string[start], (end - start));
    subbuff[end - start] = '\0';
    return subbuff;
}


int is_void(char *s) {
    while (*s != '\0') {
        if (!isspace(*s))
            return 0;
        s++;
    }
    return 1;
}

int is_int(char *tbd) {
    char *rmvd_tbd = remove_spaces(tbd);
    return isdigit(atoi(rmvd_tbd));
}

int is_char(char *tbd) {
    char *rmvd_tbd = remove_spaces(tbd);
    return (strlen(rmvd_tbd) == 1 | strlen(rmvd_tbd) == 0);
}

int is_float(char *tbd) {
    double d = FLT_MAX;
    d = strtod(tbd, NULL);
    return ((d > 0 && (d > FLT_MAX || d < FLT_MIN))
            || (d < 0 && (d < -FLT_MAX || d > -FLT_MIN)));
}

/*
int is_valid_func_def(char *tbd) {
    char *rmvd_tbd = remove_spaces(tbd);
    if(rmvd_tbd[0] == 'd' && rmvd_tbd[1] == 'e' && rmvd_tbd[2] == 'f') {
        int counter = 2;
        int upper_limit = 32;
        for(counter; counter <= 32; counter++) {
            if(rmvd_tbd[counter] == ':') {
                if(counter == 3) return 0;
                return 1; // TODO TODO
            }
        }
        return 0;
    } else {
        return 0;
    }
}
*/