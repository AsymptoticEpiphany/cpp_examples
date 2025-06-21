#include "mpmc_queue.h"

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

// Test that enqueue/dequeue works with a single thread using ints
TEST(MPMCQueueTest, SingleThread) {
    MPMCQueue<int, 8> q;
    EXPECT_TRUE(q.enqueue(42));
    EXPECT_TRUE(q.enqueue(7));
    int v;
    EXPECT_TRUE(q.dequeue(v)); EXPECT_EQ(v, 42);
    EXPECT_TRUE(q.dequeue(v)); EXPECT_EQ(v, 7);
    EXPECT_FALSE(q.dequeue(v));  // queue empty
}

// Test that the queue works correctly when full using ints
TEST(MPMCQueueTest, FullCapacity) {
    MPMCQueue<int, 4> q;
    EXPECT_TRUE(q.enqueue(1));
    EXPECT_TRUE(q.enqueue(2));
    EXPECT_TRUE(q.enqueue(3));
    EXPECT_TRUE(q.enqueue(4));
    EXPECT_FALSE(q.enqueue(5)); // Queue should be full now
    int v;
    EXPECT_TRUE(q.dequeue(v)); EXPECT_EQ(v, 1);
    EXPECT_TRUE(q.enqueue(5)); // Free space in queue
    EXPECT_FALSE(q.enqueue(6)); // Queue full again
}

// Test that the queue correctly wraps around using ints
TEST(MPMCQueueTest, WrapAround) {
    MPMCQueue<int, 4> q;
    for (int i = 0; i < 10; ++i) {
        EXPECT_TRUE(q.enqueue(i));
        int v;
        EXPECT_TRUE(q.dequeue(v));
        EXPECT_EQ(v, i);
    }
}

// Test full multi producer and multi consumer using size_t
TEST(MPMCQueueTest, RealWorldTest) {
    constexpr size_t QUEUE_SIZE = 128;
    constexpr size_t PRODUCERS = 40;
    constexpr size_t CONSUMERS = 4;
    constexpr size_t ITEMS_PER = 10000;

    MPMCQueue<size_t, QUEUE_SIZE> q;
    std::atomic<size_t> produced{0}, consumed{0};
    std::vector<std::thread> producers, consumers;

    for (size_t producer_id = 0; producer_id < PRODUCERS; ++producer_id) {
        // We use local_id in the lambda to prevent stack-use-after-scope errors.
        // If we capture producer_id by reference in the lambda, when the loop
        // increments producer_id and the lambda is running in other threads, 
        // producer_id becomes invalid.
        size_t local_id = producer_id;
        producers.emplace_back([local_id, &q, &produced](){
            for (size_t i = 0; i < ITEMS_PER; ++i) {
                while (!q.enqueue(local_id * ITEMS_PER + i)) {
                    std::this_thread::yield();
                }
                ++produced;
            }
        });
    }

    std::atomic<bool> done{false};
    for (size_t consumer_id = 0; consumer_id < CONSUMERS; ++consumer_id) {
        size_t local_id = consumer_id;
        consumers.emplace_back([local_id, &q, &consumed, &done](){
            size_t value;
            while (!done.load()) {
                if (q.dequeue(value)) {
                    ++consumed;
                    if (consumed == PRODUCERS * ITEMS_PER) {
                        done.store(true);
                    }
                } 
                else {
                    std::this_thread::yield();
                }
            }
        });
    }

    for (auto &t : producers) t.join();
    for (auto &t : consumers) t.join();

    EXPECT_EQ(produced.load(), PRODUCERS * ITEMS_PER);
    EXPECT_EQ(consumed.load(), PRODUCERS * ITEMS_PER);
}

// This test will always pass in Github actions,
// but it can be changed and run for benchmarking purposes.
TEST(MPMCQueueTest, BenchmarkPerformance) {
    constexpr size_t QUEUE_SIZE = 1024;
    constexpr size_t BIG_NUMBER = 1'000'000;

    MPMCQueue<size_t, QUEUE_SIZE> q;
    std::atomic<bool> start{false};
    std::atomic<size_t> consumed{0};

    auto producer = [&]() {
        while (!start.load());
        for (size_t i = 0; i < BIG_NUMBER; ++i) {
            while (!q.enqueue(i)) {}
        }
    };

    auto consumer = [&]() {
        size_t value;
        while (!start.load());
        while (consumed < BIG_NUMBER) {
            if (q.dequeue(value)) {
                ++consumed;
            }
        }
    };

    std::thread p(producer), c(consumer);
    auto t0 = std::chrono::steady_clock::now();
    start.store(true);
    p.join();
    c.join();
    auto t1 = std::chrono::steady_clock::now();

    std::chrono::duration<double> duration = t1 - t0;
    std::cout << "Benchmark: " << BIG_NUMBER << " ops in " << duration.count() << " seconds ("
              << (BIG_NUMBER / duration.count()) << " ops/sec)\n";

    EXPECT_EQ(consumed.load(), BIG_NUMBER);
}