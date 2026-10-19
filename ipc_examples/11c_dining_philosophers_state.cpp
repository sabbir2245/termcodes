/*
 * LIMITATIONS / PROBLEMS
 * - Correct and deadlock-free, but a global `mutex` serialises every state
 *   change, so all `test()` calls are serialised.
 * - Maximum parallelism is floor(N/2) eaters, not N.
 * - More complex to reason about; a wrong state transition or a missing/extra
 *   sem_post can deadlock or let two neighbours eat at once.
 * - No fairness: a philosopher can be starved by its neighbours.
 * - Requires every access to `state[]` to be under the mutex.
 */

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>

#define N 5
#define MEALS 10

#define LEFT(i) (((i) + N - 1) % N)
#define RIGHT(i) (((i) + 1) % N)

#define THINKING 0
#define HUNGRY 1
#define EATING 2

int state[N];
sem_t mutex;
sem_t s[N];

void test(int i) {
    if (state[i] == HUNGRY &&
        state[LEFT(i)] != EATING &&
        state[RIGHT(i)] != EATING) {
        state[i] = EATING;
        sem_post(&s[i]);
    }
}

void take_forks(int i) {
    sem_wait(&mutex);
    state[i] = HUNGRY;
    test(i);
    sem_post(&mutex);
    sem_wait(&s[i]);
}

void put_forks(int i) {
    sem_wait(&mutex);
    state[i] = THINKING;
    test(LEFT(i));
    test(RIGHT(i));
    sem_post(&mutex);
}

void *philosopher(void *arg) {
    int i = *(int *)arg;

    for (int meal = 0; meal < MEALS; meal++) {
        printf("philosopher %d is thinking\n", i);
        usleep(1000);

        take_forks(i);

        printf("philosopher %d is eating\n", i);
        usleep(1000);

        put_forks(i);
    }

    return NULL;
}

int main(void) {
    pthread_t philosophers[N];
    int ids[N];

    sem_init(&mutex, 0, 1);
    for (int i = 0; i < N; i++) {
        sem_init(&s[i], 0, 0);
        state[i] = THINKING;
        ids[i] = i;
    }

    for (int i = 0; i < N; i++) {
        pthread_create(&philosophers[i], NULL, philosopher, &ids[i]);
    }

    for (int i = 0; i < N; i++) {
        pthread_join(philosophers[i], NULL);
    }

    for (int i = 0; i < N; i++) {
        sem_destroy(&s[i]);
    }
    sem_destroy(&mutex);

    return 0;
}
