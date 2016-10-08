
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    void *ptr;
} test_t;

void alterval(test_t *test, void *a) {
    test->ptr = a;
}

void* retval(test_t *test) {
    return test->ptr;
}

void passbyreference(test_t *test) {
    int a = 23; // some random val
    alterval(test, (void *)&a);
}

int main(int argc, char **argv) {
   test_t test;
   passbyreference(&test);
   void *res = retval(&test);
   int a = *((int *)res);
   printf("%d", a);
}
