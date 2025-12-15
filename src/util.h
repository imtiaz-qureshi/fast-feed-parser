#pragma once
#include <vector>
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <cmath>

inline double percentile(const std::vector<uint64_t> &v_in, double p) {
    if (v_in.empty()) return 0.0;
    std::vector<uint64_t> v = v_in;
    std::sort(v.begin(), v.end());
    double idx = p * (v.size() - 1);
    size_t lo = static_cast<size_t>(std::floor(idx));
    size_t hi = static_cast<size_t>(std::ceil(idx));
    if (lo == hi) return v[lo];
    double frac = idx - lo;
    return v[lo] * (1.0 - frac) + v[hi] * frac;
}

inline void print_stats(const std::vector<uint64_t> &lat) {
    if (lat.empty()) {
        std::cout << "No latency samples collected\n";
        return;
    }
    double avg = std::accumulate(lat.begin(), lat.end(), 0.0) / lat.size();
    std::cout << "samples: " << lat.size() << "\n";
    std::cout << "avg (us): " << avg / 1000.0 << "\n";
    std::cout << "p50 (us): " << percentile(lat, 0.50) / 1000.0 << "\n";
    std::cout << "p90 (us): " << percentile(lat, 0.90) / 1000.0 << "\n";
    std::cout << "p99 (us): " << percentile(lat, 0.99) / 1000.0 << "\n";
    std::cout << "p99.9 (us): " << percentile(lat, 0.999) / 1000.0 << "\n";
}
