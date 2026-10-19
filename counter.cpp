/*
 * LIMITATIONS / PROBLEMS
 * - `counter = counter + 1` is a read-modify-write. Two threads can read the
 *   same value, both add one, and both write back, so updates are lost.
 * - `volatile` does NOT make the operation atomic; it only stops some
 *   compiler caching. The race (and undefined behaviour in C++) remains.
 * - The final result is unpredictable and usually less than max * 2.
 * - This file intentionally shows the bug; see counterfixed.cpp for the
 *   mutex-protected version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int max;
volatile int counter = 0; // 'volatile' forces direct memory read/write (disables register caching)

void *mythread(void *arg) {
    char *letter = (char *)arg;
    int i; // Allocated on thread's local stack (private per thread)
    
    printf("%s: begin [addr of i: %p]\n", letter, &i);
    for (i = 0; i < max; i++) {
        counter = counter + 1; // Unsynchronized access to shared global variable
    }
    printf("%s: done\n", letter);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: main-first <loopcount>\n");
        exit(1);
    }
    max = atoi(argv[1]);
    
    pthread_t p1, p2;
    printf("main: begin [counter = %d]\n", counter);

    pthread_create(&p1, NULL, mythread, (void *)"A");
    pthread_create(&p2, NULL, mythread, (void *)"B");

    pthread_join(p1, NULL);
    pthread_join(p2, NULL);

    // Final result may NOT equal (2 * max) due to race conditions
    printf("main: done [counter: %d] [should be: %d]\n", counter, max * 2);
    return 0;
}