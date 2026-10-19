/*
 * LIMITATIONS / PROBLEMS
 * - The order of `sem_wait(&empty/full)` and `sem_wait(&mutex)` matters.
 *   Taking the mutex before the counting semaphore can deadlock.
 * - Correct for multiple producers/consumers, but only because the mutex
 *   protects just the buffer update; a missing post hangs a waiter.
 * - No fairness: a slow thread can be starved.
 * - If a thread dies while holding the mutex, or before posting, the system
 *   deadlocks.
 * - Semaphores provide no priority handling (priority inversion possible).
 */

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>

#define N 5
#define ITEMS 20

int buffer[N];
int in = 0;
int out = 0;

sem_t mutex;
sem_t empty;
sem_t full;

void *producer(void *arg) {
    (void)arg;

    for (int item = 1; item <= ITEMS; item++) {
        sem_wait(&empty);
        sem_wait(&mutex);

        buffer[in] = item;
        in = (in + 1) % N;
        printf("produced %d\n", item);

        sem_post(&mutex);
        sem_post(&full);
        usleep(1000);
    }

    return NULL;
}

void *consumer(void *arg) {
    (void)arg;

    for (int i = 0; i < ITEMS; i++) {
        sem_wait(&full);
        sem_wait(&mutex);

        int item = buffer[out];
        out = (out + 1) % N;
        printf("consumed %d\n", item);

        sem_post(&mutex);
        sem_post(&empty);
        usleep(1000);
    }

    return NULL;
}

int main(void) {
    pthread_t producer_thread, consumer_thread;

    sem_init(&mutex, 0, 1);
    sem_init(&empty, 0, N);
    sem_init(&full, 0, 0);

    pthread_create(&producer_thread, NULL, producer, NULL);
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    sem_destroy(&mutex);
    sem_destroy(&empty);
    sem_destroy(&full);

    return 0;
}
