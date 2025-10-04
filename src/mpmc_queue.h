#pragma once
#include <atomic>
#include <array>
#include <cassert>
#include <cstddef>
#include <type_traits>

// Multi-producer multi-consumer queue using no dynamic allocation.
// This prevents heap allocation and bitwise& optimization is guaranteed by
// enforcing a power of 2 queue capacity.
// All constraints are enforced at compile-time and a cache-local
// fixed-size buffer helps with performance.

template<typename T, size_t Capacity>
class MPMCQueue {
    static_assert(Capacity >= 2, "Capacity must be >= 2");
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be a power of 2");

    // Hardcoding 64 since it will work on x86-64 CPUs.
    // Otherwise need std::hardware_destructive_interference_size,
    // which requires additional compiler flags.
    static constexpr size_t cache_line = 64;
    static constexpr size_t data_size = sizeof(std::atomic<size_t>) + sizeof(T);
    static constexpr size_t padding_size = (data_size < cache_line) ? (cache_line - data_size) : 0;

public:
    MPMCQueue() {
        // Initally we store the index of the buffer in each piece of Data.
        // Later we check if this value matches the buffer's index,
        // and if it does we know we can store data here, otherwise it's in
        // use and we go to the next spot in the queue.
        for (size_t i = 0; i < Capacity; ++i) {
            buffer[i].sequence.store(i, std::memory_order_relaxed);
        }
    }

    bool enqueue(const T& value) {
        Data* data;
        size_t pos = tail.value.load(std::memory_order_relaxed);

        // Claim first, then act.
        // The while loop simply finds a spot in the queue we can write our data.
        // After we break from the while loop, we actually write the data.
        // This separates the logic of claiming a spot and writing data in it.
        while (true) {
            data = &buffer[pos & (Capacity - 1)];
            size_t seq = data->sequence.load(std::memory_order_acquire);
            intptr_t diff = static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos);
            
            if (diff == 0) {
                // This slot in the queue is free for use.
                if (tail.value.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
                    // We've successfully claimed this spot in the queue.
                    // We'll write the data outside.
                    break;
                }
            } 
            else if (diff < 0) {
                // Queue is full.
                return false;
            } 
            else {
                // Try again next iteration using another position in the queue
                pos = tail.value.load(std::memory_order_relaxed);
            }
        }

        // We've claimed a spot in the queue, actually write the data.
        data->value = value;
        data->sequence.store(pos + 1, std::memory_order_release);
        return true;
    }

    bool dequeue(T& value) {
        Data* data;
        size_t pos = head.value.load(std::memory_order_relaxed);

        // Claim first, then act.
        // The while loop simply finds the next piece of data to dequeue.
        // After we break from the while loop, we actually grab the data and
        // mark that spot in the queue as available for use.
        while (true) {
            data = &buffer[pos & (Capacity - 1)];
            size_t seq = data->sequence.load(std::memory_order_acquire);
            intptr_t diff = static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos + 1);
            
            if (diff == 0) {
                // This is the next slot with data to dequeue.
                if (head.value.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
                    // We've successfully claimed this spot in the queue.
                    // We'll grab the data outside.                    
                    break;
                }
            } 
            else if (diff < 0) {
                // Queue is empty
                return false;
            } 
            else {
                // Try again next iteration using another position in the queue
                pos = head.value.load(std::memory_order_relaxed);
            }
        }

        // Grab the data and mark this spot in the queue as free-to-use.
        value = std::move(data->value);
        data->sequence.store(pos + Capacity, std::memory_order_release);
        return true;
    }

private:
    // We want to maximize performance by preventing false sharing and having
    // our data cache-line aligned. This is why we have char padding if necessary.
    // The PaddedAtomic struct ensures that head and tail are cache-line isolated.
    
    struct alignas(cache_line) Data {
        std::atomic<size_t> sequence;
        T value;
        char padding[padding_size > 0 ? padding_size : 0];
    };

    struct alignas(cache_line) PaddedAtomic {
        std::atomic<size_t> value;
        char padding[cache_line - sizeof(std::atomic<size_t>)];
    };

    std::array<Data, Capacity> buffer;
    PaddedAtomic head{};
    PaddedAtomic tail{};
};
