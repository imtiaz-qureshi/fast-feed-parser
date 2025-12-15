#include "feed_generator.h"
#include "spsc_ringbuffer.h"
#include <thread>
#include <chrono>
#include <atomic>
#include <random>

using namespace std::chrono;

void producer_thread_func(SPSCQueue<RawMsg> &q, std::atomic<bool> &run_flag, uint64_t target_msgs_per_sec) {
    uint64_t seq = 1;
    // synthetic price generator
    std::mt19937_64 rng(12345);
    std::uniform_int_distribution<uint32_t> sym(1, 1000);
    std::uniform_int_distribution<uint32_t> qty(1, 1000);
    std::uniform_real_distribution<double> price(100.0, 200.0);

    const auto period = (target_msgs_per_sec == 0) ? nanoseconds(0) :
                        nanoseconds(1'000'000'000ULL / target_msgs_per_sec);

    while (run_flag.load(std::memory_order_relaxed)) {
        RawMsg m;
        m.seq = seq++;
        m.t_sent_ns = duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
        m.symbol_id = sym(rng);
        m.size = qty(rng);
        m.price = price(rng);
        // busy spin until push succeeds (simple backpressure)
        while (!q.try_push(m)) {
            // brief pause to avoid burning 100% CPU if full
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
        if (period.count() > 0) std::this_thread::sleep_for(period);
    }
}
