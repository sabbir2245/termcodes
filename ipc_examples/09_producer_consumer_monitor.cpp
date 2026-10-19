/*
 * LIMITATIONS / PROBLEMS
 * - All access MUST go through the monitor; one direct read/write of the
 *   buffer outside the class breaks mutual exclusion.
 * - `notify_one` wakes an arbitrary waiter. With multiple producers/consumers
 *   a thread may wake but find the predicate false and sleep again; that is
 *   why the wait uses a predicate check.
 * - No fairness: notify_one can keep waking the same thread.
 * - C++ has no built-in monitor, so the mutex/condition variables are manual
 *   and easy to get wrong.
 * - Bounded buffer implies back-pressure; buffer size affects throughput.
 */

#include <condition_variable>
#include <cstdio>
#include <mutex>
#include <thread>
#include <unistd.h>

#define N 5
#define ITEMS 20

class Monitor {
public:
    void insert(int value) {
        std::unique_lock<std::mutex> lock(mutex_);

        not_full_.wait(lock, [this] { return count_ < N; });

        buffer_[hi_] = value;
        hi_ = (hi_ + 1) % N;
        count_++;

        not_empty_.notify_one();
    }

    int remove() {
        std::unique_lock<std::mutex> lock(mutex_);

        not_empty_.wait(lock, [this] { return count_ > 0; });

        int value = buffer_[lo_];
        lo_ = (lo_ + 1) % N;
        count_--;

        not_full_.notify_one();
        return value;
    }

private:
    std::mutex mutex_;
    std::condition_variable not_full_;
    std::condition_variable not_empty_;
    int buffer_[N];
    int count_ = 0;
    int lo_ = 0;
    int hi_ = 0;
};

int main() {
    Monitor monitor;

    std::thread producer([&monitor] {
        for (int item = 1; item <= ITEMS; item++) {
            monitor.insert(item);
            printf("produced %d\n", item);
            usleep(1000);
        }
    });

    std::thread consumer([&monitor] {
        for (int i = 0; i < ITEMS; i++) {
            int item = monitor.remove();
            printf("consumed %d\n", item);
            usleep(1000);
        }
    });

    producer.join();
    consumer.join();

    return 0;
}
