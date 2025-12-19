#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <memory>
#include <new>
#include <type_traits>

/**
 * @file spsc_ringbuffer.h
 * @brief Lock-free single-producer single-consumer ring buffer implementation
 * @author Imtiaz Qureshi (Enterprise Solutions Team)
 * @version 1.0.0
 * @date 2025
 * 
 * This file contains a high-performance, lock-free ring buffer optimized for
 * single-producer single-consumer scenarios commonly found in financial
 * trading systems and real-time data processing pipelines.
 */

/**
 * @class SPSCQueue
 * @brief Lock-free single-producer single-consumer ring buffer
 * 
 * A high-performance circular buffer implementation that provides lock-free
 * operations between a single producer and single consumer thread. The buffer
 * uses memory ordering guarantees to ensure thread safety without locks.
 * 
 * Key characteristics:
 * - Zero-allocation after construction
 * - Cache-line aligned for optimal performance
 * - Power-of-two capacity requirement for efficient modulo operations
 * - Memory ordering optimized for x86-64 architectures
 * 
 * @tparam T Element type (must be trivially copyable)
 * 
 * @note This class is optimized for scenarios where exactly one thread
 *       produces data and exactly one thread consumes data. Using it with
 *       multiple producers or consumers will result in undefined behavior.
 * 
 * @warning The capacity must be a power of two for correct operation.
 * 
 * Example usage:
 * @code
 * SPSCQueue<int> queue(1024);  // 1024 element capacity
 * 
 * // Producer thread
 * if (queue.try_push(42)) {
 *     // Successfully pushed
 * }
 * 
 * // Consumer thread  
 * int value;
 * if (queue.try_pop(value)) {
 *     // Successfully popped, value contains the data
 * }
 * @endcode
 */
template<typename T>
class alignas(64) SPSCQueue {
public:
    static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable for zero-copy semantics");

    /**
     * @brief Constructs a new SPSC queue with specified capacity
     * 
     * @param capacity_pow2 Buffer capacity (must be power of 2)
     * 
     * @throws std::bad_alloc if memory allocation fails
     * @throws std::invalid_argument if capacity is not power of 2
     * 
     * @note The actual memory allocated will be capacity * sizeof(T) bytes,
     *       aligned to the natural alignment of T for optimal performance.
     */
    explicit SPSCQueue(size_t capacity_pow2) {
        assert((capacity_pow2 & (capacity_pow2 - 1)) == 0 && "capacity must be power of two");
        capacity = capacity_pow2;
        mask = capacity - 1;
        buffer = static_cast<T*>(std::aligned_alloc(alignof(T), capacity * sizeof(T)));
        if (!buffer) {
            throw std::bad_alloc();
        }
        head.store(0, std::memory_order_relaxed);
        tail.store(0, std::memory_order_relaxed);
    }

    /**
     * @brief Destructor - frees allocated buffer memory
     */
    ~SPSCQueue() {
        std::free(buffer);
    }

    // Non-copyable and non-movable for safety
    SPSCQueue(const SPSCQueue&) = delete;
    SPSCQueue& operator=(const SPSCQueue&) = delete;
    SPSCQueue(SPSCQueue&&) = delete;
    SPSCQueue& operator=(SPSCQueue&&) = delete;

    /**
     * @brief Attempts to push an item to the queue (producer side)
     * 
     * This method is lock-free and wait-free. It will return immediately
     * whether the operation succeeds or fails.
     * 
     * @param item The item to push (will be copied)
     * @return true if item was successfully pushed, false if queue is full
     * 
     * @note This method should only be called from the producer thread.
     *       Calling from multiple threads will result in undefined behavior.
     * 
     * @warning The item is copied using assignment operator. Ensure T's
     *          assignment operator is thread-safe if T contains pointers.
     */
    bool try_push(const T &item) {
        size_t t = tail.load(std::memory_order_relaxed);
        size_t nt = t + 1;
        if (nt - head.load(std::memory_order_acquire) > capacity) {
            return false; // Queue is full
        }
        buffer[t & mask] = item;
        tail.store(nt, std::memory_order_release);
        return true;
    }

    /**
     * @brief Attempts to pop an item from the queue (consumer side)
     * 
     * This method is lock-free and wait-free. It will return immediately
     * whether the operation succeeds or fails.
     * 
     * @param out Reference to store the popped item
     * @return true if item was successfully popped, false if queue is empty
     * 
     * @note This method should only be called from the consumer thread.
     *       Calling from multiple threads will result in undefined behavior.
     * 
     * @warning The output parameter is only modified on successful pop.
     *          Check return value before using the output parameter.
     */
    bool try_pop(T &out) {
        size_t h = head.load(std::memory_order_relaxed);
        if (h == tail.load(std::memory_order_acquire)) {
            return false; // Queue is empty
        }
        out = buffer[h & mask];
        head.store(h + 1, std::memory_order_release);
        return true;
    }

    /**
     * @brief Returns approximate number of elements in the queue
     * 
     * @return Approximate queue size (may be slightly inaccurate due to concurrent access)
     * 
     * @note This method can be called from any thread but the result may be
     *       stale by the time it's used due to concurrent modifications.
     *       Use only for monitoring/debugging purposes.
     */
    size_t approx_size() const {
        return tail.load(std::memory_order_acquire) - head.load(std::memory_order_acquire);
    }

    /**
     * @brief Returns the maximum capacity of the queue
     * 
     * @return Maximum number of elements the queue can hold
     */
    size_t get_capacity() const noexcept {
        return capacity;
    }

    /**
     * @brief Checks if the queue is approximately empty
     * 
     * @return true if queue appears empty (may be inaccurate due to concurrency)
     */
    bool empty() const noexcept {
        return head.load(std::memory_order_acquire) == tail.load(std::memory_order_acquire);
    }

private:
    T *buffer;                                    ///< Aligned buffer for storing elements
    size_t capacity;                              ///< Buffer capacity (power of 2)
    size_t mask;                                  ///< Bitmask for efficient modulo (capacity - 1)
    alignas(64) std::atomic<size_t> head;        ///< Consumer index (cache-line aligned)
    alignas(64) std::atomic<size_t> tail;        ///< Producer index (cache-line aligned)
};
