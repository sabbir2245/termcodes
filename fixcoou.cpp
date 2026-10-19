/*
 * LIMITATIONS / PROBLEMS
 * - Correct now, but the mutex is coarse-grained: the whole increment is
 *   serialised, so the two threads cannot scale.
 * - Blocking on the mutex can cause contention and, with many threads,
 *   convoying.
 * - No fairness guarantee; one thread can be starved under heavy contention.
 * - `volatile` is no longer needed once every access is under the mutex.
 * - Mutexes are not robust: if a thread dies while holding the lock, the
 *   others block forever.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int max;
int counter = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *mythread(void *arg) {
    char *letter = (char *)arg;
    int i; // Allocated on thread's local stack (private per thread)
    
    printf("%s: begin [addr of i: %p]\n", letter, &i);
    for (i = 0; i < max; i++) {
        pthread_mutex_lock(&lock);
        counter = counter + 1;
        pthread_mutex_unlock(&lock); // synchronized access to shared global variable
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

    // Access is synchronized, so the final result should equal (2 * max)
    printf("main: done [counter: %d] [should be: %d]\n", counter, max * 2);
    return 0;
}