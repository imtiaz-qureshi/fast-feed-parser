/**
 * @file main.cpp
 * @brief Main entry point for the fast feed parser benchmark application
 * @author Imtiaz Qureshi (Enterprise Solutions Team)
 * @version 1.0.0
 * @date 2025
 * 
 * This application demonstrates a high-performance market data processing
 * pipeline using lock-free data structures. It measures end-to-end latency
 * and throughput characteristics of a producer-consumer architecture commonly
 * used in financial trading systems.
 * 
 * The application creates two threads:
 * - Producer: Generates synthetic market data at configurable rates
 * - Consumer: Processes messages and collects latency statistics
 * 
 * Command line arguments:
 *   ./fast-feed-parser [msgs_per_sec] [total_seconds] [buffer_pow2]
 * 
 * Example:
 *   ./fast-feed-parser 1000000 10 17
 *   (1M msgs/sec, 10 seconds, 128K buffer size)
 */

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
#include <iomanip>
#include <stdexcept>

/**
 * @brief Global flag for graceful shutdown coordination
 * 
 * This atomic flag is shared between the main thread and worker threads
 * to coordinate graceful shutdown. When set to false, all threads will
 * complete their current operations and exit cleanly.
 */
std::atomic<bool> g_run{true};

/**
 * @brief Signal handler for SIGINT (Ctrl+C)
 * 
 * Provides graceful shutdown capability when user interrupts the program.
 * Sets the global run flag to false, allowing threads to complete their
 * current work and exit cleanly.
 * 
 * @param signal Signal number (unused)
 */
void sigint_handler(int signal) {
    (void)signal;  // Suppress unused parameter warning
    std::cout << "\n[INFO] Shutdown signal received, stopping gracefully...\n";
    g_run.store(false, std::memory_order_release);
}

/**
 * @brief Prints usage information and command line argument help
 */
void print_usage(const char* program_name) {
    std::cout << "\nUsage: " << program_name << " [msgs_per_sec] [total_seconds] [buffer_pow2]\n\n";
    std::cout << "Arguments:\n";
    std::cout << "  msgs_per_sec  - Target message rate (default: 500000)\n";
    std::cout << "                  Range: 1 to 10,000,000\n";
    std::cout << "  total_seconds - Test duration in seconds (default: 5)\n";
    std::cout << "                  Range: 1 to 3600\n";
    std::cout << "  buffer_pow2   - Buffer size as power of 2 (default: 16 = 65536)\n";
    std::cout << "                  Range: 10 to 24 (1K to 16M elements)\n\n";
    std::cout << "Examples:\n";
    std::cout << "  " << program_name << "                    # Default: 500K msgs/s, 5s, 64K buffer\n";
    std::cout << "  " << program_name << " 1000000 10 17      # 1M msgs/s, 10s, 128K buffer\n";
    std::cout << "  " << program_name << " 100000 30 15       # 100K msgs/s, 30s, 32K buffer\n\n";
}

/**
 * @brief Main application entry point
 * 
 * Initializes the benchmark environment, creates producer and consumer threads,
 * monitors execution, and reports performance statistics.
 * 
 * @param argc Argument count
 * @param argv Argument values
 * @return 0 on success, non-zero on error
 */
int main(int argc, char** argv) {
    try {
        // Display application header
        std::cout << "========================================\n";
        std::cout << "Fast Feed Parser - Performance Benchmark\n";
        std::cout << "Version 1.0.0\n";
        std::cout << "========================================\n\n";

        // Parse command line arguments with validation
        uint64_t msgs_per_sec = 500000;  // Default: 500K msgs/s
        int total_seconds = 5;            // Default: 5 seconds
        size_t buf_pow2 = 1 << 16;       // Default: 65536 elements

        if (argc >= 2) {
            msgs_per_sec = std::stoull(argv[1]);
            if (msgs_per_sec == 0 || msgs_per_sec > 10000000) {
                std::cerr << "[ERROR] Invalid message rate. Must be between 1 and 10,000,000\n";
                print_usage(argv[0]);
                return 1;
            }
        }

        if (argc >= 3) {
            total_seconds = std::stoi(argv[2]);
            if (total_seconds <= 0 || total_seconds > 3600) {
                std::cerr << "[ERROR] Invalid duration. Must be between 1 and 3600 seconds\n";
                print_usage(argv[0]);
                return 1;
            }
        }

        if (argc >= 4) {
            uint32_t pow2 = std::stoul(argv[3]);
            if (pow2 < 10 || pow2 > 24) {
                std::cerr << "[ERROR] Invalid buffer size. Power must be between 10 and 24\n";
                print_usage(argv[0]);
                return 1;
            }
            buf_pow2 = static_cast<size_t>(1ULL << pow2);
        }

        // Display configuration
        std::cout << "[CONFIG] Test Parameters:\n";
        std::cout << "  Message rate:  " << std::setw(10) << msgs_per_sec << " msgs/sec\n";
        std::cout << "  Duration:      " << std::setw(10) << total_seconds << " seconds\n";
        std::cout << "  Buffer size:   " << std::setw(10) << buf_pow2 << " elements\n";
        std::cout << "  Expected msgs: " << std::setw(10) << (msgs_per_sec * total_seconds) << " total\n\n";

        // Register signal handler for graceful shutdown
        signal(SIGINT, sigint_handler);

        // Initialize SPSC queue
        std::cout << "[INFO] Initializing lock-free SPSC queue...\n";
        SPSCQueue<RawMsg> q(buf_pow2);

        // Pre-allocate latency collection vector
        std::vector<uint64_t> latencies;
        size_t max_samples = msgs_per_sec * total_seconds / 2;
        latencies.reserve(max_samples);
        std::cout << "[INFO] Reserved space for " << max_samples << " latency samples\n";

        // Launch producer and consumer threads
        std::cout << "[INFO] Starting producer and consumer threads...\n\n";
        std::thread prod([&]{ 
            producer_thread_func(q, g_run, msgs_per_sec); 
        });
        
        std::thread cons([&]{ 
            consumer_thread_func(q, g_run, latencies, max_samples); 
        });

        // Monitor execution and display progress
        std::cout << "========================================\n";
        std::cout << "Benchmark Running...\n";
        std::cout << "========================================\n";
        
        using namespace std::chrono_literals;
        for (int i = 0; i < total_seconds && g_run.load(std::memory_order_acquire); ++i) {
            std::this_thread::sleep_for(1s);
            size_t approx = q.approx_size();
            std::cout << "[t=" << std::setw(3) << (i + 1) << "s] Queue depth: " 
                     << std::setw(8) << approx << " messages\n";
        }

        // Signal threads to stop and wait for completion
        std::cout << "\n[INFO] Stopping threads...\n";
        g_run.store(false, std::memory_order_release);
        
        prod.join();
        cons.join();
        
        std::cout << "[INFO] All threads stopped successfully\n";

        // Display results
        std::cout << "\n========================================\n";
        std::cout << "Benchmark Complete\n";
        std::cout << "========================================\n";
        std::cout << "Total samples collected: " << latencies.size() << "\n";
        
        // Print detailed statistics
        print_stats(latencies);

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "\n[ERROR] Exception caught: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "\n[ERROR] Unknown exception caught\n";
        return 1;
    }
}
