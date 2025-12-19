#pragma once

#include <vector>
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <cmath>
#include <iomanip>

/**
 * @file util.h
 * @brief Statistical analysis utilities for performance measurement
 * @author Imtiaz Qureshi (Enterprise Solutions Team)
 * @version 1.0.0
 * @date 2025
 * 
 * This module provides utility functions for statistical analysis of latency
 * measurements and performance metrics. It includes percentile calculations
 * and formatted output functions commonly used in financial system benchmarking.
 */

/**
 * @brief Calculates percentile value from a dataset using linear interpolation
 * 
 * Computes the specified percentile from a vector of values using the linear
 * interpolation method (R-7 quantile method). This method is commonly used
 * in financial performance analysis and provides smooth percentile estimates.
 * 
 * The function creates a copy of the input vector for sorting, preserving
 * the original data order. For large datasets, consider using in-place
 * sorting if the original order is not needed.
 * 
 * @param v_in Input vector of values (will not be modified)
 * @param p Percentile to calculate (0.0 to 1.0, e.g., 0.95 for 95th percentile)
 * @return Calculated percentile value, or 0.0 if input is empty
 * 
 * @note Time complexity: O(n log n) due to sorting
 * @note Space complexity: O(n) due to vector copy
 * 
 * @warning Percentile parameter p should be in range [0.0, 1.0].
 *          Values outside this range may produce unexpected results.
 * 
 * Example usage:
 * @code
 * std::vector<uint64_t> latencies = {100, 200, 150, 300, 250};
 * double p95 = percentile(latencies, 0.95);  // 95th percentile
 * double median = percentile(latencies, 0.50);  // Median (50th percentile)
 * @endcode
 */
inline double percentile(const std::vector<uint64_t> &v_in, double p) {
    if (v_in.empty()) {
        return 0.0;
    }
    
    // Create a copy for sorting (preserves original data)
    std::vector<uint64_t> v = v_in;
    std::sort(v.begin(), v.end());
    
    // Linear interpolation method (R-7 quantile)
    double idx = p * (v.size() - 1);
    size_t lo = static_cast<size_t>(std::floor(idx));
    size_t hi = static_cast<size_t>(std::ceil(idx));
    
    if (lo == hi) {
        return static_cast<double>(v[lo]);
    }
    
    double frac = idx - lo;
    return v[lo] * (1.0 - frac) + v[hi] * frac;
}

/**
 * @brief Prints comprehensive latency statistics in a formatted table
 * 
 * Analyzes and displays key statistical measures of latency data including
 * sample count, mean, and various percentiles. The output is formatted for
 * easy reading and includes units conversion from nanoseconds to microseconds.
 * 
 * Statistics displayed:
 * - Sample count: Total number of measurements
 * - Average: Arithmetic mean latency
 * - p50 (Median): 50th percentile
 * - p90: 90th percentile (common SLA threshold)
 * - p99: 99th percentile (tail latency)
 * - p99.9: 99.9th percentile (extreme tail latency)
 * 
 * @param lat Vector of latency measurements in nanoseconds
 * 
 * @note All latency values are converted from nanoseconds to microseconds
 *       for display (divided by 1000.0) as microseconds are more readable
 *       for typical system latencies.
 * 
 * @note If the input vector is empty, prints an informative message
 *       instead of attempting calculations.
 * 
 * Performance considerations:
 * - Time complexity: O(n log n) due to percentile calculations
 * - Memory usage: O(n) for sorting operations in percentile function
 * - For very large datasets (>1M samples), consider sampling for display
 * 
 * Example output:
 * @code
 * ================================
 * Latency Analysis Results
 * ================================
 * Samples collected: 1,000,000
 * Average latency:   1.23 μs
 * Median (p50):      0.98 μs
 * 90th percentile:   2.15 μs
 * 99th percentile:   4.67 μs
 * 99.9th percentile: 8.92 μs
 * ================================
 * @endcode
 */
inline void print_stats(const std::vector<uint64_t> &lat) {
    if (lat.empty()) {
        std::cout << "\n================================\n";
        std::cout << "Latency Analysis Results\n";
        std::cout << "================================\n";
        std::cout << "No latency samples collected\n";
        std::cout << "================================\n";
        return;
    }
    
    // Calculate statistics
    double avg = std::accumulate(lat.begin(), lat.end(), 0.0) / lat.size();
    
    // Format output with proper alignment and precision
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "\n================================\n";
    std::cout << "Latency Analysis Results\n";
    std::cout << "================================\n";
    std::cout << "Samples collected: " << std::setw(10) << lat.size() << "\n";
    std::cout << "Average latency:   " << std::setw(8) << (avg / 1000.0) << " μs\n";
    std::cout << "Median (p50):      " << std::setw(8) << (percentile(lat, 0.50) / 1000.0) << " μs\n";
    std::cout << "90th percentile:   " << std::setw(8) << (percentile(lat, 0.90) / 1000.0) << " μs\n";
    std::cout << "99th percentile:   " << std::setw(8) << (percentile(lat, 0.99) / 1000.0) << " μs\n";
    std::cout << "99.9th percentile: " << std::setw(8) << (percentile(lat, 0.999) / 1000.0) << " μs\n";
    std::cout << "================================\n";
}
