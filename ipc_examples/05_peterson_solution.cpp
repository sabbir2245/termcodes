/*
 * LIMITATIONS / PROBLEMS
 * - Only correct for 2 processes; Peterson does not generalise to N.
 * - The original slide uses plain `int` flags. Without atomics/memory
 *   barriers the compiler/CPU may reorder the stores and loads and break the
 *   algorithm. This version uses std::atomic to stay correct.
 * - Busy waiting wastes CPU.
 * - If a process is preempted inside the critical section it can block the
 *   other (priority inversion / convoying).
 */

#include <atomic>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>

#define ITERATIONS 100000

std::atomic<int> turn(0);
std::atomic<int> interested[2];
int counter = 0;

void enter_region(int process) {
    int other = 1 - process;

    interested[process].store(true);
    turn.store(process);

    while (turn.load() == process && interested[other].load()) {
        sched_yield();
    }
}

void leave_region(int process) {
    interested[process].store(false);
}

void *worker(void *arg) {
    int process = *(int *)arg;

    for (int i = 0; i < ITERATIONS; i++) {
        enter_region(process);
        counter++;
        leave_region(process);
    }

    return NULL;
}

int main(void) {
    pthread_t p0, p1;
    int id0 = 0;
    int id1 = 1;

    interested[0].store(false);
    interested[1].store(false);

    pthread_create(&p0, NULL, worker, &id0);
    pthread_create(&p1, NULL, worker, &id1);

    pthread_join(p0, NULL);
    pthread_join(p1, NULL);

    printf("counter = %d, expected = %d\n", counter, 2 * ITERATIONS);
    return 0;
}
