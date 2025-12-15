#include "spsc_ringbuffer.h"
#include "feed_generator.h"
#include "parser.h"
#include "util.h"

#include <thread>
#include <chrono>
#include <iostream>
#include <atomic>
#include <csignal>
#include <vector>

std::atomic<bool> g_run{true};

void sigint_handler(int) {
    g_run.store(false);
}

int main(int argc, char** argv) {
    // simple args: msgs_per_sec total_seconds buffer_pow2
    uint64_t msgs_per_sec = 500000; // default 500k msgs/s
    int total_seconds = 5;
    size_t buf_pow2 = 1<<16; // 65536

    if (argc >= 2) msgs_per_sec = std::stoull(argv[1]);
    if (argc >= 3) total_seconds = std::stoi(argv[2]);
    if (argc >= 4) buf_pow2 = static_cast<size_t>(1ULL << std::stoul(argv[3]));

    signal(SIGINT, sigint_handler);

    SPSCQueue<RawMsg> q(buf_pow2);

    std::vector<uint64_t> latencies;
    latencies.reserve(msgs_per_sec * total_seconds / 10); // sample subset

    std::thread prod([&]{ producer_thread_func(q, g_run, msgs_per_sec); });
    std::thread cons([&]{ consumer_thread_func(q, g_run, latencies, msgs_per_sec * total_seconds / 2); });

    using namespace std::chrono_literals;
    for (int i=0;i<total_seconds && g_run.load();++i) {
        std::this_thread::sleep_for(1s);
        size_t approx = q.approx_size();
        std::cout << "t=" << (i+1) << "s, queue_approx=" << approx << "\n";
    }

    g_run.store(false);
    prod.join();
    cons.join();

    std::cout << "Finished. Collected " << latencies.size() << " samples.\n";
    print_stats(latencies);
    return 0;
}
