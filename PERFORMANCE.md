# Performance Tuning Guide

This guide provides comprehensive instructions for optimizing the Fast Feed Parser for maximum performance in production environments.

## Table of Contents

- [System Configuration](#system-configuration)
- [Compilation Optimizations](#compilation-optimizations)
- [Runtime Configuration](#runtime-configuration)
- [Hardware Considerations](#hardware-considerations)
- [Monitoring and Profiling](#monitoring-and-profiling)
- [Troubleshooting](#troubleshooting)

## System Configuration

### CPU Configuration

#### CPU Governor
Set CPU governor to performance mode for consistent performance:

```bash
# Check current governor
cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor

# Set to performance mode
echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor

# Disable CPU frequency scaling (Intel)
echo 1 | sudo tee /sys/devices/system/cpu/intel_pstate/no_turbo

# Disable CPU frequency scaling (AMD)
echo 0 | sudo tee /sys/devices/system/cpu/cpufreq/boost
```

#### CPU Affinity
Isolate CPU cores for dedicated processing:

```bash
# Isolate cores 2-7 for application use
# Add to kernel boot parameters: isolcpus=2-7 nohz_full=2-7 rcu_nocbs=2-7

# Run application on isolated cores
taskset -c 2,3 ./fast-feed-parser

# Set process priority
nice -n -20 ./fast-feed-parser
```

#### NUMA Configuration
For NUMA systems, bind process to specific NUMA node:

```bash
# Check NUMA topology
numactl --hardware

# Run on specific NUMA node
numactl --cpunodebind=0 --membind=0 ./fast-feed-parser
```

### Memory Configuration

#### Huge Pages
Configure huge pages for reduced TLB misses:

```bash
# Check current huge page configuration
cat /proc/meminfo | grep Huge

# Configure 2MB huge pages
echo 1024 | sudo tee /proc/sys/vm/nr_hugepages

# Configure 1GB huge pages (if supported)
echo 4 | sudo tee /sys/kernel/mm/hugepages/hugepages-1048576kB/nr_hugepages

# Mount huge page filesystem
sudo mkdir -p /mnt/huge
sudo mount -t hugetlbfs nodev /mnt/huge
```

#### Memory Limits
Increase memory limits for the application:

```bash
# Increase locked memory limit
ulimit -l unlimited

# Increase virtual memory limit
ulimit -v unlimited

# Set in /etc/security/limits.conf for persistent configuration
echo "* soft memlock unlimited" | sudo tee -a /etc/security/limits.conf
echo "* hard memlock unlimited" | sudo tee -a /etc/security/limits.conf
```

#### Swap Configuration
Disable swap to prevent performance degradation:

```bash
# Disable swap temporarily
sudo swapoff -a

# Disable swap permanently (comment out swap entries in /etc/fstab)
sudo sed -i '/swap/s/^/#/' /etc/fstab

# Set swappiness to minimum
echo 1 | sudo tee /proc/sys/vm/swappiness
```

### Network Configuration (if applicable)

#### Network Interface Optimization
```bash
# Increase network buffer sizes
echo 'net.core.rmem_max = 134217728' | sudo tee -a /etc/sysctl.conf
echo 'net.core.wmem_max = 134217728' | sudo tee -a /etc/sysctl.conf
echo 'net.ipv4.tcp_rmem = 4096 87380 134217728' | sudo tee -a /etc/sysctl.conf
echo 'net.ipv4.tcp_wmem = 4096 65536 134217728' | sudo tee -a /etc/sysctl.conf

# Apply changes
sudo sysctl -p
```

## Compilation Optimizations

### Build Configuration

#### Release Build with Maximum Optimizations
```bash
mkdir build && cd build

# GCC optimizations
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_FLAGS="-O3 -march=native -mtune=native -flto -ffast-math -funroll-loops" \
      ..

# Clang optimizations  
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_FLAGS="-O3 -march=native -mtune=native -flto -ffast-math -funroll-loops" \
      ..

cmake --build . -j$(nproc)
```

#### Profile-Guided Optimization (PGO)
```bash
# Step 1: Build with profiling instrumentation
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_FLAGS="-O3 -march=native -fprofile-generate" \
      ..
cmake --build . -j$(nproc)

# Step 2: Run representative workload
./fast-feed-parser 1000000 30 17

# Step 3: Rebuild with profile data
cmake -DCMAKE_CXX_FLAGS="-O3 -march=native -fprofile-use" ..
cmake --build . -j$(nproc)
```

### Compiler-Specific Optimizations

#### GCC Specific
```bash
export CXXFLAGS="-O3 -march=native -mtune=native -flto=auto -fno-fat-lto-objects \
                 -ffast-math -funroll-loops -fprefetch-loop-arrays \
                 -fomit-frame-pointer -finline-functions"
```

#### Clang Specific
```bash
export CXXFLAGS="-O3 -march=native -mtune=native -flto=thin \
                 -ffast-math -funroll-loops -fvectorize \
                 -fomit-frame-pointer -finline-functions"
```

#### Intel Compiler (if available)
```bash
export CXXFLAGS="-O3 -xHost -ipo -no-prec-div -fp-model fast=2 \
                 -fomit-frame-pointer -inline-level=2"
```

## Runtime Configuration

### Application Parameters

#### Buffer Size Optimization
Choose buffer size based on expected throughput and latency requirements:

```bash
# For high throughput (>1M msgs/sec): Use larger buffers
./fast-feed-parser 2000000 60 20  # 1M element buffer

# For low latency (<1μs p99): Use smaller buffers  
./fast-feed-parser 500000 60 15   # 32K element buffer

# For balanced performance: Use medium buffers
./fast-feed-parser 1000000 60 17  # 128K element buffer
```

#### Message Rate Tuning
```bash
# Start with conservative rate and increase gradually
./fast-feed-parser 100000 10 16   # 100K msgs/sec baseline
./fast-feed-parser 500000 10 16   # 500K msgs/sec
./fast-feed-parser 1000000 10 16  # 1M msgs/sec
./fast-feed-parser 2000000 10 16  # 2M msgs/sec (if hardware supports)
```

### Environment Variables

#### Memory Allocation
```bash
# Use jemalloc for better memory allocation performance
export LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libjemalloc.so.2

# Configure jemalloc for low latency
export MALLOC_CONF="background_thread:true,metadata_thp:auto,dirty_decay_ms:0,muzzy_decay_ms:0"
```

#### CPU Scheduling
```bash
# Set real-time scheduling priority
export SCHED_PRIORITY=99
chrt -f $SCHED_PRIORITY ./fast-feed-parser
```

## Hardware Considerations

### CPU Requirements

#### Minimum Requirements
- x86-64 architecture with SSE4.2 support
- 2+ cores (1 for producer, 1 for consumer)
- 2.0+ GHz base frequency

#### Recommended Configuration
- Intel Xeon or AMD EPYC processors
- 4+ cores with hyperthreading disabled
- 3.0+ GHz base frequency
- Large L3 cache (>8MB)

#### Optimal Configuration
- Latest generation Intel Xeon or AMD EPYC
- 8+ dedicated cores
- 3.5+ GHz all-core turbo
- Large L3 cache (>32MB)
- NUMA-aware configuration

### Memory Requirements

#### Minimum Requirements
- 4GB RAM
- DDR4-2400 or faster
- Single channel acceptable

#### Recommended Configuration
- 16GB+ RAM
- DDR4-3200 or faster
- Dual channel configuration
- ECC memory for production

#### Optimal Configuration
- 32GB+ RAM
- DDR4-3600 or DDR5
- Quad channel configuration
- ECC memory with error correction

### Storage Requirements

#### For Logging and Monitoring
- SSD storage for log files
- 100GB+ available space
- High IOPS capability (>10K IOPS)

## Monitoring and Profiling

### Performance Monitoring

#### Real-time Monitoring
```bash
# Monitor CPU usage
top -p $(pgrep fast-feed-parser)

# Monitor memory usage
cat /proc/$(pgrep fast-feed-parser)/status | grep -E "(VmRSS|VmSize)"

# Monitor cache misses
perf stat -e cache-misses,cache-references ./fast-feed-parser 1000000 10 16
```

#### Detailed Profiling
```bash
# CPU profiling with perf
perf record -g ./fast-feed-parser 1000000 30 16
perf report

# Memory profiling with valgrind
valgrind --tool=massif ./fast-feed-parser 100000 10 15
ms_print massif.out.*

# Cache analysis
perf stat -e L1-dcache-loads,L1-dcache-load-misses,LLC-loads,LLC-load-misses \
    ./fast-feed-parser 1000000 10 16
```

### Latency Analysis

#### Histogram Generation
```bash
# Generate latency histogram data
./fast-feed-parser 1000000 60 16 > latency_results.txt

# Analyze with custom scripts or tools like HdrHistogram
```

#### Continuous Monitoring
```bash
# Long-running performance test
while true; do
    echo "$(date): Starting benchmark run"
    ./fast-feed-parser 1000000 300 17 | tee -a performance_log.txt
    sleep 60
done
```

## Troubleshooting

### Common Performance Issues

#### High Latency Spikes

**Symptoms:**
- p99 latency > 10μs
- Irregular latency distribution

**Causes and Solutions:**
1. **CPU frequency scaling**: Disable CPU governor
2. **Memory allocation**: Use memory pools or jemalloc
3. **System interrupts**: Isolate CPU cores
4. **Thermal throttling**: Improve cooling

#### Low Throughput

**Symptoms:**
- Cannot achieve target message rates
- High CPU usage with low throughput

**Causes and Solutions:**
1. **Buffer size too small**: Increase buffer size
2. **Memory bandwidth**: Check NUMA configuration
3. **Compiler optimizations**: Enable -O3 and -march=native
4. **System contention**: Isolate application resources

#### Memory Issues

**Symptoms:**
- High memory usage
- Memory allocation failures

**Causes and Solutions:**
1. **Buffer size too large**: Reduce buffer size
2. **Memory fragmentation**: Use huge pages
3. **Memory leaks**: Profile with valgrind
4. **Insufficient memory**: Increase system RAM

### Debugging Tools

#### Performance Analysis
```bash
# Intel VTune (if available)
vtune -collect hotspots ./fast-feed-parser 1000000 30 16

# AMD uProf (if available)
AMDuProfCLI collect --config tbp ./fast-feed-parser 1000000 30 16

# Linux perf tools
perf top -p $(pgrep fast-feed-parser)
```

#### Memory Analysis
```bash
# Check for memory leaks
valgrind --leak-check=full ./fast-feed-parser 100000 10 15

# Analyze memory access patterns
valgrind --tool=cachegrind ./fast-feed-parser 100000 10 15
```

### Performance Baselines

#### Expected Performance Ranges

| Hardware Class | Throughput | p50 Latency | p99 Latency |
|---------------|------------|-------------|-------------|
| Desktop (i7) | 500K-1M msgs/sec | 200-500ns | 1-5μs |
| Server (Xeon) | 1M-3M msgs/sec | 100-300ns | 500ns-2μs |
| HPC (Optimized) | 2M-5M msgs/sec | 50-200ns | 200ns-1μs |

#### Regression Testing
```bash
# Baseline performance test
./fast-feed-parser 1000000 60 17 > baseline_results.txt

# Compare against baseline
./fast-feed-parser 1000000 60 17 > current_results.txt
diff baseline_results.txt current_results.txt
```

## Advanced Optimizations

### Lock-Free Optimizations

#### Memory Ordering Tuning
For expert users, consider adjusting memory ordering in the SPSC queue based on target architecture.

#### Cache Line Optimization
Ensure data structures are properly aligned to cache line boundaries (64 bytes on x86-64).

### Kernel Bypass Techniques

#### User-Space Networking (if applicable)
- DPDK for network I/O bypass
- SPDK for storage I/O bypass

#### Real-Time Kernel
Consider using RT kernel patches for guaranteed latency bounds:
```bash
# Install RT kernel (Ubuntu)
sudo apt-get install linux-image-rt-amd64
```

This guide provides a comprehensive foundation for optimizing Fast Feed Parser performance. Always measure before and after applying optimizations to ensure they provide the expected benefits in your specific environment.