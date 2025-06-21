#include "utils/print_tuple.h"
#include "mpmc_queue.h"

#include <atomic>
#include <chrono>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

int main() {
    constexpr size_t QUEUE_SIZE = 1024;
    constexpr size_t NUM_PRODUCERS = 40;
    constexpr size_t NUM_CONSUMERS = 40;
    constexpr size_t ITEMS_PER_PRODUCER = 100000;

    MPMCQueue<size_t, QUEUE_SIZE> queue;
    std::atomic<size_t> producedCount{0};
    std::atomic<size_t> consumedCount{0};
    std::atomic<bool> done{false};

    auto producer = [&](size_t id) {
        for (size_t i = 0; i < ITEMS_PER_PRODUCER; ++i) {
            size_t item = id * 100000 + i;
            while (!queue.enqueue(item)) {
                std::this_thread::yield();
            }
            size_t total = ++producedCount;
            if (total % 10000 == 0) {
                std::cout << "[Producer " << id << "] Produced total: " << total << "\n";
            }
        }
    };

    auto consumer = [&](size_t id) {
        size_t value;
        while (true) {
            if (done.load(std::memory_order_acquire)) break;
            if (queue.dequeue(value)) {
                size_t total = ++consumedCount;
                if (total % 10000 == 0) {
                    std::cout << "[Consumer " << id << "] Consumed total: " << total
                              << ", value: " << value << "\n";
                }
                if (total >= NUM_PRODUCERS * ITEMS_PER_PRODUCER) {
                    done.store(true, std::memory_order_release);
                }
            } else {
                std::this_thread::yield();
            }
        }
    };

    std::vector<std::thread> producers, consumers;
    for (size_t i = 0; i < NUM_PRODUCERS; ++i) {
        producers.emplace_back(producer, i);
    }
    
    for (size_t i = 0; i < NUM_CONSUMERS; ++i) {
        consumers.emplace_back(consumer, i);
    }

    for (auto& p : producers) p.join();
    for (auto& c : consumers) c.join();

    std::cout << "✅ All done. Produced: " << producedCount << ", Consumed: " << consumedCount << "\n";
    return 0;
}