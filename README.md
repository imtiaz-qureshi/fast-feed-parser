# Fast Feed Parser

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![CMake](https://img.shields.io/badge/CMake-3.15+-green.svg)](https://cmake.org/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

A high-performance, enterprise-grade C++20 market data feed parser demonstrating ultra-low latency message processing with lock-free data structures.

## Overview

The fast-feed-parser demonstrates how a lock-free SPSC queue can outperform lock-based synchronization in latency-sensitive pipelines.

By relying only on atomic operations and well-defined memory ordering, the producer and consumer communicate without entering critical sections or triggering kernel-mediated blocking. This reduces scheduler interference and variance, leading to more predictable latency characteristics compared to mutex- or semaphore-based designs on a general-purpose OS.

### Key Features

- **Lock-Free Architecture**: SPSC ring buffer with memory ordering guarantees
- **Zero-Copy Design**: Fixed 32-byte binary message format for optimal cache performance  
- **Configurable Throughput**: Synthetic feed generator with adjustable message rates
- **Comprehensive Metrics**: Latency percentiles (p50/p90/p99/p99.9) and throughput analysis
- **Enterprise Ready**: Professional code structure, documentation, and error handling
- **Zero Dependencies**: Pure C++20 implementation with CMake build system

### Performance Characteristics

- **Throughput**: Supports 500K+ messages/second on modern hardware
- **Latency**: Sub-microsecond processing latency (p99 < 1μs typical)
- **Memory**: Bounded memory usage with configurable ring buffer sizes
- **CPU**: Optimized for modern x86-64 architectures with SIMD instructions

## Architecture

```
┌─────────────┐    ┌──────────────┐    ┌─────────────┐
│   Producer  │──▶ │ SPSC Buffer  │───▶│  Consumer   │
│ (Feed Gen)  │    │ (Lock-Free)  │    │  (Parser)   │
└─────────────┘    └──────────────┘    └─────────────┘
```

The system consists of three main components:

1. **Feed Generator**: Produces synthetic market data at configurable rates
2. **SPSC Ring Buffer**: Lock-free circular buffer for zero-contention message passing
3. **Message Parser**: Consumes and processes messages with latency tracking

## Quick Start

### Prerequisites

- C++20 compatible compiler (GCC 10+, Clang 12+, MSVC 2019+)
- CMake 3.15 or higher
- Modern x86-64 processor (for optimal performance)

### Building

```bash
# Clone the repository
git clone <repository-url>
cd fast-feed-parser

# Create build directory
mkdir build && cd build

# Configure with optimizations
cmake -DCMAKE_BUILD_TYPE=Release ..

# Build the project
cmake --build . -j$(nproc)
```

### Running

```bash
# Run with default settings (500K msgs/sec, 5 seconds, 64K buffer)
./fast-feed-parser

# Custom configuration: 1M msgs/sec, 10 seconds, 128K buffer
./fast-feed-parser 1000000 10 17

# Low latency test: 100K msgs/sec, 30 seconds, 32K buffer  
./fast-feed-parser 100000 30 15
```

### Command Line Arguments

| Argument | Description | Default | Range |
|----------|-------------|---------|-------|
| `msgs_per_sec` | Target message rate | 500,000 | 1 - 10,000,000 |
| `total_seconds` | Test duration | 5 | 1 - 3600 |
| `buffer_pow2` | Buffer size (2^N) | 16 (64K) | 10 - 24 |

## Performance Tuning

### System Configuration

For optimal performance in production environments:

```bash
# Set CPU governor to performance mode
echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor

# Disable CPU frequency scaling
echo 1 | sudo tee /sys/devices/system/cpu/intel_pstate/no_turbo

# Set process affinity (isolate cores)
taskset -c 2,3 ./fast-feed-parser

# Increase process priority
nice -n -20 ./fast-feed-parser
```

### Memory Configuration

```bash
# Increase locked memory limits
ulimit -l unlimited

# Configure huge pages (optional)
echo 1024 | sudo tee /proc/sys/vm/nr_hugepages
```

## API Documentation

### Core Classes

#### `SPSCQueue<T>`
Lock-free single-producer single-consumer ring buffer.

```cpp
template<typename T>
class SPSCQueue {
public:
    explicit SPSCQueue(size_t capacity_pow2);
    bool try_push(const T& item);
    bool try_pop(T& out);
    size_t approx_size() const;
};
```

#### `RawMsg`
Fixed-size market data message structure (32 bytes).

```cpp
struct RawMsg {
    uint64_t seq;        // Sequence number
    uint64_t t_sent_ns;  // Send timestamp (nanoseconds)
    uint32_t symbol_id;  // Instrument identifier
    uint32_t size;       // Order/trade size
    double   price;      // Price level
};
```

## Testing

### Unit Tests
```bash
# Build and run tests
cmake -DBUILD_TESTING=ON ..
make test
```

### Benchmarking
```bash
# Latency benchmark
./fast-feed-parser 100000 60 16

# Throughput benchmark  
./fast-feed-parser 2000000 30 20

# Memory pressure test
./fast-feed-parser 1000000 300 24
```

## Production Deployment

### Monitoring

The application provides real-time metrics:
- Queue depth monitoring
- Latency percentile tracking
- Throughput measurement
- Memory usage statistics

### Logging

Configure logging levels via environment variables:
```bash
export LOG_LEVEL=INFO
export LOG_FILE=/var/log/feed-parser.log
```

### Health Checks

The system supports graceful shutdown via SIGINT and provides health status endpoints for monitoring systems.

## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details on our code of conduct and the process for submitting pull requests.

### Code Style

- Follow [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
- Use `clang-format` with provided configuration
- Maintain 100% test coverage for new features
- Document all public APIs with Doxygen comments

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Support

For enterprise support and consulting services, please contact [imtiaz.qureshi@gmail.com](mailto:imtiaz.qureshi@gmail.com).

### Known Issues

- Windows support requires MSVC 2019+ with C++20 features enabled
- ARM64 support is experimental and may have reduced performance
- Buffer sizes > 2^24 may cause memory allocation failures on some systems

## Changelog

See [CHANGELOG.md](CHANGELOG.md) for a detailed history of changes and releases.
