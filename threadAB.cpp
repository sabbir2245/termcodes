/*
 * LIMITATIONS / PROBLEMS
 * - No synchronisation: the order of "A" and "B", and their interleaving
 *   with main's prints, is nondeterministic.
 * - Return values of pthread_create/pthread_join are not checked; failures
 *   are silently ignored.
 * - Fixed to two threads that share no data, so it only demonstrates thread
 *   creation and joining, not cooperation or mutual exclusion.
 * - No cleanup or cancellation handling.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void *mythread(void *arg) {
    printf("%s\n", (char *) arg);
    return NULL;
}

int main(int argc, char *argv[]) {
    (void)argv;
    if (argc != 1) {
        fprintf(stderr, "usage: main\n");
        exit(1);
    }
    
    pthread_t p1, p2;
    printf("main: begin\n");

    // Create two concurrent threads
    pthread_create(&p1, NULL, mythread, (void *)"A");
    pthread_create(&p2, NULL, mythread, (void *)"B");

    // pthread_join waits for threads to complete execution
    pthread_join(p1, NULL);
    pthread_join(p2, NULL);

    printf("main: end\n");
    return 0;
}
