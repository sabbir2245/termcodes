/*
 * LIMITATIONS / PROBLEMS
 * - The asymmetric fork order prevents deadlock, but not starvation: a
 *   philosopher can keep losing the race for its forks.
 * - No fairness/ageing, no priority handling and no timeout.
 * - If a philosopher is preempted or crashes while holding a fork, others
 *   may wait a long time; a mutex offers no liveness guarantee.
 * - Fixed to 10 meals and N philosophers; the eating and thinking times are
 *   placeholders.
 * - printf output from threads can interleave.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define N 5 // Number of philosophers and forks
#define MEALS 10 // Number of meals each philosopher eats

pthread_mutex_t forks[N];
pthread_t philosophers[N];
int ids[N];

void *philosopher(void *num) {
    int id = *(int *)num;
    int left = id;
    int right = (id + 1) % N;

    for (int meal = 0; meal < MEALS; meal++) {
        // Thinking state
        printf("Philosopher %d is thinking.\n", id);
        sleep(1);

        // --- SOLVING DEADLOCK VIA ASYMMETRY ---
        // Even-numbered philosophers pick up left fork first, then right.
        // Odd-numbered philosophers pick up right fork first, then left.
        if (id % 2 == 0) {
            pthread_mutex_lock(&forks[left]);
            pthread_mutex_lock(&forks[right]);
        } else {
            pthread_mutex_lock(&forks[right]);
            pthread_mutex_lock(&forks[left]);
        }

        // Eating state
        printf("Philosopher %d is eating.\n", id);
        sleep(1);

        // Put down both forks
        pthread_mutex_unlock(&forks[left]);
        pthread_mutex_unlock(&forks[right]);
        
        printf("Philosopher %d finished eating and is back to thinking.\n", id);
    }

    return NULL;
}

int main() {
    // Initialize a mutex for each fork
    for (int i = 0; i < N; i++) {
        pthread_mutex_init(&forks[i], NULL);
    }

    // Create philosopher threads
    for (int i = 0; i < N; i++) {
        ids[i] = i;
        pthread_create(&philosophers[i], NULL, philosopher, &ids[i]);
    }

    // Wait for all philosophers to finish their meals
    for (int i = 0; i < N; i++) {
        pthread_join(philosophers[i], NULL);
    }

    // Cleanup mutexes
    for (int i = 0; i < N; i++) {
        pthread_mutex_destroy(&forks[i]);
    }

    return 0;
}