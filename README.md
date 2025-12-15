# fast-feed-parser

Small, modern C++20 project demonstrating a low-latency market-data parsing pipeline.

## Features
- Single-producer single-consumer (SPSC) lock-free ring buffer
- Fixed-size binary feed messages (32B)
- Simple feed generator producing synthetic ticks at configurable rate
- Consumer that decodes messages and records latencies
- Microbenchmark: throughput and latency percentiles (p50/p90/p99/p99.9)
- No external dependencies (uses C++20 & CMake)

## Build
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j
