/*
 * LIMITATIONS / PROBLEMS
 * - Classic check-then-set race: both threads can see lock == 0 before either
 *   writes 1, so both enter the critical section (see the wrong total).
 * - Busy waiting burns CPU while the lock is held.
 * - `volatile` only prevents some compiler caching; it does NOT make the
 *   read/modify/write atomic or provide the memory ordering the algorithm
 *   needs.
 * - No fairness: a thread can starve.
 * - Fix: an atomic test-and-set/compare-exchange or a real mutex.
 */

#include <pthread.h>
#include <sched.h>
#include <stdio.h>

#define ITERATIONS 20000

volatile int lock = 0;
volatile long counter = 0;

void *worker(void *arg) {
    (void)arg;

    for (int i = 0; i < ITERATIONS; i++) {
        while (lock) {
            sched_yield();
        }
        lock = 1;

        counter++;

        lock = 0;
    }

    return NULL;
}

int main(void) {
    pthread_t t1, t2;

    pthread_create(&t1, NULL, worker, NULL);
    pthread_create(&t2, NULL, worker, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("counter = %ld, expected = %d\n", counter, 2 * ITERATIONS);
    return 0;
}
