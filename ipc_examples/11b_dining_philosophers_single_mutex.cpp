/*
 * LIMITATIONS / PROBLEMS
 * - Correct but kills concurrency: only one philosopher can eat at a time,
 *   even when non-neighbours could eat together. It is a global bottleneck.
 * - The extra table mutex makes the per-fork mutexes redundant; locking order
 *   is still something to reason about.
 * - Throughput does not scale with the number of philosophers.
 * - Fairness is not guaranteed; one philosopher can starve.
 */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define N 5
#define MEALS 3

pthread_mutex_t table = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t forks[N];

void *philosopher(void *arg) {
    int id = *(int *)arg;
    int left = id;
    int right = (id + 1) % N;

    for (int meal = 0; meal < MEALS; meal++) {
        printf("philosopher %d is thinking\n", id);
        usleep(1000);

        pthread_mutex_lock(&table);
        pthread_mutex_lock(&forks[left]);
        pthread_mutex_lock(&forks[right]);

        printf("philosopher %d is eating\n", id);
        usleep(1000);

        pthread_mutex_unlock(&forks[right]);
        pthread_mutex_unlock(&forks[left]);
        pthread_mutex_unlock(&table);
    }

    return NULL;
}

int main(void) {
    pthread_t philosophers[N];
    int ids[N];

    for (int i = 0; i < N; i++) {
        pthread_mutex_init(&forks[i], NULL);
        ids[i] = i;
    }

    for (int i = 0; i < N; i++) {
        pthread_create(&philosophers[i], NULL, philosopher, &ids[i]);
    }

    for (int i = 0; i < N; i++) {
        pthread_join(philosophers[i], NULL);
    }

    for (int i = 0; i < N; i++) {
        pthread_mutex_destroy(&forks[i]);
    }

    pthread_mutex_destroy(&table);
    return 0;
}
