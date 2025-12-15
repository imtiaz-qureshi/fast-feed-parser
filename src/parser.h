#pragma once
#include "feed_generator.h"
#include <cstdint>
#include <vector>
#include <atomic>
#include "spsc_ringbuffer.h"

void consumer_thread_func(SPSCQueue<RawMsg> &q, std::atomic<bool> &run_flag, std::vector<uint64_t> &latencies_ns, size_t max_collect);

struct Tick {
    uint64_t seq;
    uint64_t t_sent_ns;
    uint64_t t_recv_ns;
    uint32_t symbol_id;
    uint32_t size;
    double price;
};
