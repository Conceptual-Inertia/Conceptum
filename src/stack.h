/*
 * stack.h - custom impl of the stack
 *
 * Copyright (C) 2016 Ruijie Fang <ruijief@acm.org> ALL RIGHTS RESERVED.
 *
 */

typedef char stackElementT;  /* Give it a generic name--makes  */
/* changing the type of things in */
/* the stack easy.                */

typedef struct {
    stackElementT *contents;
    int top;
    /* Other fields? */
} stackT;

/*
 * prototypes
 */

void StackInit(stackT *stackP, int maxSize); // init the stack
void StackDestroy(stackT *stackP); // destroy the stack
int StackIsEmpty(stackT stack);
int StackIsFull(stackT stack);
void StackPush(stackT *stackP, stackElementT element);
stackElementT StackPop(stackT *stackP);


void StackInit(stackT *stackP, int maxSize)
{
    stackElementT *newContents;

    /* Allocate a new array to hold the contents. */

    newContents = (stackElementT *)malloc(sizeof(stackElementT)
                                          * maxSize);

    if (newContents == NULL) {
        fprintf(stderr, "Insufficient memory to initialize stack.\n");
        exit(1);  /* Exit, returning error code. */
    }

    stackP->contents = newContents;
    stackP->maxSize = maxSize;
    stackP->top = -1;  /* I.e., empty */
}

void StackDestroy(stackT *stackP)
{
    /* Get rid of array. */
    free(stackP->contents);

    stackP->contents = NULL;
    stackP->maxSize = 0;
    stackP->top = -1;  /* I.e., empty */
}

int StackIsEmpty(stackT *stackP)
{
    return stackP->top < 0;
}

int StackIsFull(stackT *stackP)
{
    return stackP->top >= stackP->maxSize - 1;
}

void StackPush(stackT *stackP, stackElementT element)
{
    if (StackIsFull(stackP)) {
        fprintf(stderr, "Can't push element on stack: stack is full.\n");
        exit(1);  /* Exit, returning error code. */
    }

    /* Put information in array; update top. */

    stackP->contents[++stackP->top] = element;
}

stackElementT StackPop(stackT *stackP)
{
    if (StackIsEmpty(stackP)) {
        fprintf(stderr, "Can't pop element from stack: stack is empty.\n");
        exit(1);  /* Exit, returning error code. */
    }

    return stackP->contents[stackP->top--];
}
