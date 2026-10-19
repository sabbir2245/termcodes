/*
 * LIMITATIONS / PROBLEMS
 * - Only one producer and one consumer are created here; multiple of each
 *   would wake arbitrary waiters and add more contention.
 * - Correctness depends on rechecking the predicate with `while` (guards
 *   against spurious and lost wakeups).
 * - Condition variables must always be used with the same mutex.
 * - No fairness: one side can be starved, and throughput depends on
 *   BUFFER_SIZE.
 * - If a thread dies while holding the mutex, the other blocks forever.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE 5
#define NUM_ITEMS 20

int buffer[BUFFER_SIZE];
int count = 0;
int in = 0;
int out = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;

void *producer(void *arg) {
    (void)arg;

    for (int item = 1; item <= NUM_ITEMS; item++) {
        pthread_mutex_lock(&mutex);

        while (count == BUFFER_SIZE) {
            pthread_cond_wait(&not_full, &mutex);
        }

        buffer[in] = item;
        in = (in + 1) % BUFFER_SIZE;
        count++;

        printf("Produced: %d [buffer: %d]\n", item, count);
        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&mutex);

        usleep(50000);
    }

    return NULL;
}

void *consumer(void *arg) {
    (void)arg;

    for (int i = 0; i < NUM_ITEMS; i++) {
        pthread_mutex_lock(&mutex);

        while (count == 0) {
            pthread_cond_wait(&not_empty, &mutex);
        }

        int item = buffer[out];
        out = (out + 1) % BUFFER_SIZE;
        count--;

        printf("Consumed: %d [buffer: %d]\n", item, count);
        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&mutex);

        usleep(50000);
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
    pthread_cond_destroy(&not_empty);
    pthread_cond_destroy(&not_full);

    printf("Done. Buffer is empty: %s\n", count == 0 ? "yes" : "no");
    return 0;
}
