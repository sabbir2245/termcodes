/*
 * LIMITATIONS / PROBLEMS
 * - The original sleep/wakeup slide is FLAWED: `if (count == 0) sleep();` is
 *   not atomic with the check, so a wakeup sent between the check and the
 *   sleep is lost and the consumer sleeps forever.
 * - This file fixes that with a mutex + condition variable and rechecks the
 *   predicate in a `while` loop.
 * - Still limited to one producer/consumer pair here; a condition variable
 *   must be paired with the same mutex, and signalling the wrong variable can
 *   still miss a wakeup.
 * - No fairness guarantee; one side can be starved.
 * - Bounded buffer means throughput depends on the buffer size.
 */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define N 5
#define ITEMS 20

int buffer[N];
int count = 0;
int in = 0;
int out = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t can_produce = PTHREAD_COND_INITIALIZER;
pthread_cond_t can_consume = PTHREAD_COND_INITIALIZER;

/* Corrected sleep/wakeup pattern.
 * The condition is rechecked under the mutex, so a wakeup cannot be lost. */
void *producer(void *arg) {
    (void)arg;

    for (int item = 1; item <= ITEMS; item++) {
        pthread_mutex_lock(&mutex);
        while (count == N) {
            pthread_cond_wait(&can_produce, &mutex);
        }

        buffer[in] = item;
        in = (in + 1) % N;
        count++;

        printf("produced %d\n", item);
        pthread_cond_signal(&can_consume);
        pthread_mutex_unlock(&mutex);

        usleep(1000);
    }

    return NULL;
}

void *consumer(void *arg) {
    (void)arg;

    for (int i = 0; i < ITEMS; i++) {
        pthread_mutex_lock(&mutex);
        while (count == 0) {
            pthread_cond_wait(&can_consume, &mutex);
        }

        int item = buffer[out];
        out = (out + 1) % N;
        count--;

        printf("consumed %d\n", item);
        pthread_cond_signal(&can_produce);
        pthread_mutex_unlock(&mutex);

        usleep(1000);
    }

    return NULL;
}

int main(void) {
    pthread_t producer_thread, consumer_thread;

    pthread_create(&producer_thread, NULL, producer, NULL);
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&can_produce);
    pthread_cond_destroy(&can_consume);

    printf("remaining items = %d\n", count);
    return 0;
}
