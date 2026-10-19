/*
 * LIMITATIONS / PROBLEMS
 * - Real "disable interrupts" is only possible in kernel (privileged) mode.
 * - On a multiprocessor, disabling interrupts on one CPU does not stop the
 *   other CPUs from touching the shared data.
 * - A long interrupt-disabled section delays the whole system (clock, I/O)
 *   and can lose interrupts/events.
 * - This file only simulates the idea with a mutex; it does NOT actually
 *   disable interrupts.
 * - Interrupt disabling has no ownership, so it is easy to leak the lock.
 */

#include <pthread.h>
#include <stdio.h>

#define ITERATIONS 100000

volatile long counter = 0;
pthread_mutex_t kernel_lock = PTHREAD_MUTEX_INITIALIZER;

/* User-space simulation of disabling interrupts.
 * Real interrupt disabling is only possible in kernel mode. */
static void disable_interrupts(void) {
    pthread_mutex_lock(&kernel_lock);
}

static void enable_interrupts(void) {
    pthread_mutex_unlock(&kernel_lock);
}

void *worker(void *arg) {
    (void)arg;

    for (int i = 0; i < ITERATIONS; i++) {
        disable_interrupts();
        counter++;
        enable_interrupts();
        /* remainder section */
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
