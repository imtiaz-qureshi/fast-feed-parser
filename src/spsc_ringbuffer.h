#pragma once
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <memory>
#include <new>
#include <type_traits>

/*
 Simple single-producer single-consumer ring buffer for fixed-size elements.
 Capacity must be power of two.
*/

template<typename T>
class alignas(64) SPSCQueue {
public:
    static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable for zero-copy semantics");

    explicit SPSCQueue(size_t capacity_pow2) {
        assert((capacity_pow2 & (capacity_pow2 - 1)) == 0 && "capacity must be power of two");
        capacity = capacity_pow2;
        mask = capacity - 1;
        buffer = static_cast<T*>(std::aligned_alloc(alignof(T), capacity * sizeof(T)));
        head.store(0, std::memory_order_relaxed);
        tail.store(0, std::memory_order_relaxed);
    }

    ~SPSCQueue() {
        std::free(buffer);
    }

    // Producer: try to push, returns false if full
    bool try_push(const T &item) {
        size_t t = tail.load(std::memory_order_relaxed);
        size_t nt = t + 1;
        if (nt - head.load(std::memory_order_acquire) > capacity) return false; // full
        buffer[t & mask] = item;
        tail.store(nt, std::memory_order_release);
        return true;
    }

    // Consumer: try to pop, returns false if empty
    bool try_pop(T &out) {
        size_t h = head.load(std::memory_order_relaxed);
        if (h == tail.load(std::memory_order_acquire)) return false; // empty
        out = buffer[h & mask];
        head.store(h + 1, std::memory_order_release);
        return true;
    }

    // approximate size (may be slightly off)
    size_t approx_size() const {
        return tail.load(std::memory_order_acquire) - head.load(std::memory_order_acquire);
    }

private:
    T *buffer;
    size_t capacity;
    size_t mask;
    alignas(64) std::atomic<size_t> head;
    alignas(64) std::atomic<size_t> tail;
};
