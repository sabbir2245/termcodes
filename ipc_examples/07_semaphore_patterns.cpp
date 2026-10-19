/*
 * LIMITATIONS / PROBLEMS
 * - Semaphores are low-level: there is no ownership, so any thread can post
 *   and accidentally break mutual exclusion.
 * - Swapping down/up or forgetting one easily deadlocks or corrupts state.
 * - Busy-wait alternatives burn CPU; blocking semaphores can still deadlock
 *   and are hard to debug.
 * - The hello/world pattern hard-codes strict alternation; if one side stalls
 *   or dies the other blocks forever.
 * - Counting semaphores need correct initial values; a wrong value breaks the
 *   whole protocol.
 */

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>

struct MutexContext {
    sem_t *mutex;
    long *counter;
};

static void *increment(void *arg) {
    MutexContext *ctx = (MutexContext *)arg;

    for (int i = 0; i < 50000; i++) {
        sem_wait(ctx->mutex);
        (*ctx->counter)++;
        sem_post(ctx->mutex);
    }

    return NULL;
}

static void demo_mutual_exclusion(void) {
    sem_t mutex;
    pthread_t t1, t2;
    long counter = 0;

    sem_init(&mutex, 0, 1);

    MutexContext ctx = {&mutex, &counter};
    pthread_create(&t1, NULL, increment, &ctx);
    pthread_create(&t2, NULL, increment, &ctx);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("7.1 mutex counter = %ld\n", counter);
    sem_destroy(&mutex);
}

static sem_t synch;

static void *run_s1(void *arg) {
    (void)arg;
    printf("S1\n");
    sem_post(&synch);
    return NULL;
}

static void *run_s2(void *arg) {
    (void)arg;
    sem_wait(&synch);
    printf("S2\n");
    return NULL;
}

static void demo_sequencing(void) {
    pthread_t t1, t2;

    sem_init(&synch, 0, 0);
    pthread_create(&t1, NULL, run_s1, NULL);
    pthread_create(&t2, NULL, run_s2, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    sem_destroy(&synch);
}

static sem_t hello_done;
static sem_t world_done;

static void *say_hello(void *arg) {
    (void)arg;
    for (int i = 0; i < 5; i++) {
        sem_wait(&world_done);
        printf("hello\n");
        sem_post(&hello_done);
    }
    return NULL;
}

static void *say_world(void *arg) {
    (void)arg;
    for (int i = 0; i < 5; i++) {
        sem_wait(&hello_done);
        printf("world\n");
        sem_post(&world_done);
    }
    return NULL;
}

static void demo_hello_world(void) {
    pthread_t t1, t2;

    sem_init(&hello_done, 0, 0);
    sem_init(&world_done, 0, 1);

    pthread_create(&t1, NULL, say_hello, NULL);
    pthread_create(&t2, NULL, say_world, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    sem_destroy(&hello_done);
    sem_destroy(&world_done);
}

int main(void) {
    demo_mutual_exclusion();
    demo_sequencing();
    demo_hello_world();
    return 0;
}
