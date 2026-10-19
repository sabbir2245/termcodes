/*
 * LIMITATIONS / PROBLEMS
 * - Requires careful bookkeeping of the "empty" replies; if the initial
 *   credits are wrong the producer or consumer blocks forever.
 * - send/receive are blocking and pipes have a finite capacity, so both sides
 *   can deadlock if the protocol gets out of step.
 * - Partial read/write is ignored here; real message passing must handle it.
 * - Copies data instead of sharing it, which costs memory bandwidth.
 * - Pipes only work on the same machine, so this is not the distributed
 *   message passing advertised in the slide.
 */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define ITEMS 20
#define INITIAL_EMPTY 5

struct Message {
    int item;
};

static int to_consumer[2];
static int to_producer[2];

void *producer(void *arg) {
    (void)arg;
    Message message;

    for (int item = 1; item <= ITEMS; item++) {
        if (read(to_producer[0], &message, sizeof(message)) != sizeof(message)) {
            break;
        }

        message.item = item;
        write(to_consumer[1], &message, sizeof(message));
        printf("sent %d\n", item);
    }

    return NULL;
}

void *consumer(void *arg) {
    (void)arg;
    Message message = {0};

    for (int i = 0; i < INITIAL_EMPTY; i++) {
        write(to_producer[1], &message, sizeof(message));
    }

    for (int i = 0; i < ITEMS; i++) {
        if (read(to_consumer[0], &message, sizeof(message)) != sizeof(message)) {
            break;
        }

        printf("received %d\n", message.item);

        message.item = 0;
        write(to_producer[1], &message, sizeof(message));
    }

    return NULL;
}

int main(void) {
    pthread_t producer_thread, consumer_thread;

    if (pipe(to_consumer) != 0 || pipe(to_producer) != 0) {
        perror("pipe");
        return 1;
    }

    pthread_create(&producer_thread, NULL, producer, NULL);
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    close(to_consumer[0]);
    close(to_consumer[1]);
    close(to_producer[0]);
    close(to_producer[1]);

    return 0;
}
