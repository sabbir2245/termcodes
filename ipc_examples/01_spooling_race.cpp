/*
 * LIMITATIONS / PROBLEMS
 * - `next_free = in; ...; in = next_free + 1;` is not atomic. The threads can
 *   interleave between reading and writing `in`.
 * - Two producers can read the same `in`, insert into the same slot, and the
 *   final count loses updates (see the wrong total at the end).
 * - In C++ this is a data race (undefined behaviour), not just a wrong value.
 * - `slot[]` can also be overwritten because the index is not reserved.
 * - Fix: protect the whole read-modify-write with a mutex or use atomics.
 */

#include <pthread.h>
#include <sched.h>
#include <stdio.h>

#define BUFFER_SIZE 8
#define INSERTS_PER_THREAD 50000

int slot[BUFFER_SIZE];
int in = 0;

void *producer(void *arg) {
    const char *name = (const char *)arg;

    for (int i = 0; i < INSERTS_PER_THREAD; i++) {
        int next_free = in;
        sched_yield();
        slot[next_free % BUFFER_SIZE] = next_free;
        in = next_free + 1;
    }

    printf("%s done\n", name);
    return NULL;
}

int main(void) {
    pthread_t p1, p2;

    pthread_create(&p1, NULL, producer, (void *)"A");
    pthread_create(&p2, NULL, producer, (void *)"B");

    pthread_join(p1, NULL);
    pthread_join(p2, NULL);

    printf("in = %d, expected = %d\n", in, 2 * INSERTS_PER_THREAD);
    return 0;
}
