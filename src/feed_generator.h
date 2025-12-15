#pragma once
#include <cstdint>
#include <cstddef>

#pragma pack(push,1)
struct RawMsg {
    uint64_t seq;
    uint64_t t_sent_ns;
    uint32_t symbol_id;
    uint32_t size;
    double   price;
};
#pragma pack(pop)
static_assert(sizeof(RawMsg) == 32, "RawMsg must be 32 bytes");

#include <atomic>
#include "spsc_ringbuffer.h"

void producer_thread_func(SPSCQueue<RawMsg> &q, std::atomic<bool> &run_flag, uint64_t target_msgs_per_sec);
