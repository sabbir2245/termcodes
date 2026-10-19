/*
 * LIMITATIONS / PROBLEMS
 * - Only works for exactly 2 processes/threads.
 * - Violates the progress requirement: if the thread whose turn it is is slow
 *   or not scheduled, the other cannot enter even when the CS is free.
 * - Busy waiting wastes CPU.
 * - `turn` must be atomic/volatile with correct memory ordering; a plain
 *   shared int is a data race in C++.
 * - Poor fit when the two processes have very different workloads.
 */

#include <pthread.h>
#include <sched.h>
#include <stdio.h>

#define ITERATIONS 50000

volatile int turn = 0;
volatile long counter = 0;

void *process(void *arg) {
    int me = *(int *)arg;

    for (int i = 0; i < ITERATIONS; i++) {
        while (turn != me) {
            sched_yield();
        }

        counter++;
        turn = 1 - me;
    }

    return NULL;
}

int main(void) {
    pthread_t p0, p1;
    int id0 = 0;
    int id1 = 1;

    pthread_create(&p0, NULL, process, &id0);
    pthread_create(&p1, NULL, process, &id1);

    pthread_join(p0, NULL);
    pthread_join(p1, NULL);

    printf("counter = %ld, expected = %d\n", counter, 2 * ITERATIONS);
    return 0;
}
