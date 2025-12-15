#include "parser.h"
#include "spsc_ringbuffer.h"
#include <chrono>
#include <atomic>
#include <vector>
#include <thread>

using namespace std::chrono;

void consumer_thread_func(SPSCQueue<RawMsg> &q, std::atomic<bool> &run_flag,
                          std::vector<uint64_t> &latencies_ns, size_t max_collect) {
    RawMsg m;
    while (run_flag.load(std::memory_order_relaxed)) {
        if (!q.try_pop(m)) {
            std::this_thread::yield();
            continue;
        }
        uint64_t t_recv = duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
        uint64_t latency = t_recv - m.t_sent_ns;
        if (latencies_ns.size() < max_collect) latencies_ns.push_back(latency);
        // "parse" into Tick (no allocation)
        Tick tk;
        tk.seq = m.seq;
        tk.t_sent_ns = m.t_sent_ns;
        tk.t_recv_ns = t_recv;
        tk.symbol_id = m.symbol_id;
        tk.size = m.size;
        tk.price = m.price;
        // (In a real pipeline, push Tick downstream)
        (void)tk;
    }
}
