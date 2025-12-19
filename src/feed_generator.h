#pragma once

#include <cstdint>
#include <cstddef>
#include <atomic>
#include "spsc_ringbuffer.h"

/**
 * @file feed_generator.h
 * @brief Market data feed generator for synthetic message production
 * @author Imtiaz Qureshi (Enterprise Solutions Team)
 * @version 1.0.0
 * @date 2025
 * 
 * This module provides functionality to generate synthetic market data messages
 * at configurable rates for testing and benchmarking purposes. The generated
 * data mimics real market feed characteristics while being deterministic.
 */

/**
 * @struct RawMsg
 * @brief Fixed-size market data message structure
 * 
 * Represents a single market data message with all essential fields.
 * The structure is packed to exactly 32 bytes for optimal cache performance
 * and network transmission efficiency.
 * 
 * Memory layout (32 bytes total):
 * - seq (8 bytes): Message sequence number
 * - t_sent_ns (8 bytes): Send timestamp in nanoseconds since epoch
 * - symbol_id (4 bytes): Unique instrument identifier
 * - size (4 bytes): Order/trade quantity
 * - price (8 bytes): Price level as IEEE 754 double
 * 
 * @note The packed attribute ensures no padding between fields,
 *       making the structure suitable for binary serialization.
 */
#pragma pack(push,1)
struct RawMsg {
    uint64_t seq;        ///< Monotonic sequence number for message ordering
    uint64_t t_sent_ns;  ///< Send timestamp (nanoseconds since Unix epoch)
    uint32_t symbol_id;  ///< Instrument identifier (1-1000 range)
    uint32_t size;       ///< Order/trade size (1-1000 range)
    double   price;      ///< Price level (100.0-200.0 range for synthetic data)
};
#pragma pack(pop)

// Compile-time verification of message size
static_assert(sizeof(RawMsg) == 32, "RawMsg must be exactly 32 bytes for optimal performance");
static_assert(alignof(RawMsg) <= 8, "RawMsg alignment must not exceed 8 bytes");

/**
 * @brief Producer thread function for generating synthetic market data
 * 
 * This function runs in a dedicated thread and generates synthetic market
 * data messages at the specified rate. It uses high-quality random number
 * generation to create realistic-looking market data while maintaining
 * deterministic behavior for reproducible benchmarks.
 * 
 * The function implements precise rate limiting using high-resolution timers
 * and handles backpressure gracefully when the consumer cannot keep up.
 * 
 * @param q Reference to the SPSC queue for message delivery
 * @param run_flag Atomic flag to control thread execution (set to false to stop)
 * @param target_msgs_per_sec Target message generation rate (0 = unlimited)
 * 
 * @note This function should be called from exactly one thread (single producer).
 *       The function will run until run_flag is set to false.
 * 
 * @warning Setting target_msgs_per_sec to 0 will generate messages as fast as
 *          possible, which may overwhelm the consumer or exhaust system resources.
 * 
 * Performance characteristics:
 * - Typical CPU usage: 5-15% per core at 1M msgs/sec
 * - Memory usage: Constant (no allocations in steady state)
 * - Latency impact: <100ns per message generation
 * 
 * Example usage:
 * @code
 * SPSCQueue<RawMsg> queue(65536);
 * std::atomic<bool> running{true};
 * std::thread producer(producer_thread_func, 
 *                     std::ref(queue), 
 *                     std::ref(running), 
 *                     500000);  // 500K msgs/sec
 * 
 * // ... run for some time ...
 * running = false;
 * producer.join();
 * @endcode
 */
void producer_thread_func(SPSCQueue<RawMsg> &q, 
                         std::atomic<bool> &run_flag, 
                         uint64_t target_msgs_per_sec);
