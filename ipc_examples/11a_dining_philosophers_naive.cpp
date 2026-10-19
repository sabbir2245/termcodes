/*
 * LIMITATIONS / PROBLEMS
 * - The slide's naive order (everyone grabs left, then right) can deadlock:
 *   all philosophers hold their left fork and wait forever for the right.
 * - This demo uses pthread_mutex_timedlock with a timeout and retries, so it
 *   cannot hang, but that is NOT the original naive algorithm.
 * - Retrying can cause livelock and wasted CPU, and is unfair.
 * - Even without the timeout, a busy retry loop wastes CPU.
 * - Proper fixes: asymmetric ordering, a global mutex, or the state-based
 *   solution (see 11b and 11c).
 */

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define N 5
#define MEALS 3
#define LOCK_TIMEOUT_MS 100

pthread_mutex_t forks[N];

/* Naive ordering: every philosopher takes the left fork first.
 * A timed lock is used for the second fork so this demo cannot hang forever. */
static bool timed_lock(pthread_mutex_t *mutex) {
    struct timespec deadline;
    clock_gettime(CLOCK_REALTIME, &deadline);
    deadline.tv_nsec += LOCK_TIMEOUT_MS * 1000000L;
    if (deadline.tv_nsec >= 1000000000L) {
        deadline.tv_sec++;
        deadline.tv_nsec -= 1000000000L;
    }
    return pthread_mutex_timedlock(mutex, &deadline) == 0;
}

void *philosopher(void *arg) {
    int id = *(int *)arg;
    int left = id;
    int right = (id + 1) % N;

    for (int meal = 0; meal < MEALS; meal++) {
        printf("philosopher %d is thinking\n", id);
        usleep(1000);

        while (true) {
            pthread_mutex_lock(&forks[left]);
            if (timed_lock(&forks[right])) {
                break;
            }

            pthread_mutex_unlock(&forks[left]);
            usleep(1000);
        }

        printf("philosopher %d is eating\n", id);
        usleep(1000);

        pthread_mutex_unlock(&forks[right]);
        pthread_mutex_unlock(&forks[left]);
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

    return 0;
}
