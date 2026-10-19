/*
 * LIMITATIONS / PROBLEMS
 * - Reader priority: if readers keep arriving, a waiting writer can starve
 *   indefinitely. This is the "first variation" only.
 * - `reader_count` must be protected; a reader that dies while holding `db`
 *   blocks all writers and readers.
 * - Writers are serialised and block all readers; long writes hurt latency.
 * - No fairness/ageing, no timeout and no deadlock detection.
 * - Assumes every thread follows the protocol; a rogue access to `data`
 *   breaks the invariant.
 */

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>

#define NUM_READERS 3
#define NUM_WRITERS 1
#define READS_PER_THREAD 3
#define WRITES_PER_THREAD 2

sem_t mutex;
sem_t db;
int reader_count = 0;
int data = 0;

void *reader(void *arg) {
    int id = *(int *)arg;

    for (int i = 0; i < READS_PER_THREAD; i++) {
        sem_wait(&mutex);
        reader_count++;
        if (reader_count == 1) {
            sem_wait(&db);
        }
        sem_post(&mutex);

        printf("reader %d reads data = %d\n", id, data);
        usleep(1000);

        sem_wait(&mutex);
        reader_count--;
        if (reader_count == 0) {
            sem_post(&db);
        }
        sem_post(&mutex);

        usleep(1000);
    }

    return NULL;
}

void *writer(void *arg) {
    int id = *(int *)arg;

    for (int i = 0; i < WRITES_PER_THREAD; i++) {
        usleep(2000);

        sem_wait(&db);
        data++;
        printf("writer %d writes data = %d\n", id, data);
        usleep(1000);
        sem_post(&db);
    }

    return NULL;
}

int main(void) {
    pthread_t readers[NUM_READERS];
    pthread_t writers[NUM_WRITERS];
    int reader_ids[NUM_READERS];
    int writer_ids[NUM_WRITERS];

    sem_init(&mutex, 0, 1);
    sem_init(&db, 0, 1);

    for (int i = 0; i < NUM_READERS; i++) {
        reader_ids[i] = i;
        pthread_create(&readers[i], NULL, reader, &reader_ids[i]);
    }
    for (int i = 0; i < NUM_WRITERS; i++) {
        writer_ids[i] = i;
        pthread_create(&writers[i], NULL, writer, &writer_ids[i]);
    }

    for (int i = 0; i < NUM_READERS; i++) {
        pthread_join(readers[i], NULL);
    }
    for (int i = 0; i < NUM_WRITERS; i++) {
        pthread_join(writers[i], NULL);
    }

    sem_destroy(&mutex);
    sem_destroy(&db);

    return 0;
}
