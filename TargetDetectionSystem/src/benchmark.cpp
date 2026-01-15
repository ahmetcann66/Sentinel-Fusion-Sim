#include "TargetDetector.h"
#include <chrono>
#include <iostream>
#include <vector>
#include <random>

class PerformanceBenchmark {
private:
    TargetDetector detector;
    std::mt19937 gen;
    std::uniform_real_distribution<double> pos_dist;
    std::uniform_real_distribution<double> signal_dist;
    
public:
    PerformanceBenchmark() : gen(42), pos_dist(-100.0, 100.0), signal_dist(0.0, 1.0) {}
    
    std::vector<std::vector<double>> generateRadarData(size_t target_count) {
        std::vector<std::vector<double>> data;
        data.reserve(target_count);
        
        for (size_t i = 0; i < target_count; ++i) {
            data.push_back({
                pos_dist(gen),  // x
                pos_dist(gen),  // y
                pos_dist(gen) * 0.1,  // z
                signal_dist(gen)  // signal strength
            });
        }
        
        return data;
    }
    
    std::vector<std::vector<double>> generateThermalData(size_t target_count) {
        std::vector<std::vector<double>> data;
        data.reserve(target_count);
        
        for (size_t i = 0; i < target_count; ++i) {
            data.push_back({
                pos_dist(gen),  // x
                pos_dist(gen),  // y
                pos_dist(gen) * 0.1,  // z
                20.0 + signal_dist(gen) * 30.0  // temperature
            });
        }
        
        return data;
    }
    
    std::vector<std::vector<double>> generateOpticalData(size_t target_count) {
        std::vector<std::vector<double>> data;
        data.reserve(target_count);
        
        for (size_t i = 0; i < target_count; ++i) {
            data.push_back({
                pos_dist(gen),  // x
                pos_dist(gen),  // y
                pos_dist(gen) * 0.1,  // z
                signal_dist(gen),  // brightness
                signal_dist(gen)   // contrast
            });
        }
        
        return data;
    }
    
    void benchmarkDetection(size_t target_count, int iterations = 100) {
        std::cout << "\n=== PERFORMANCE BENCHMARK ===" << std::endl;
        std::cout << "Target Count: " << target_count << std::endl;
        std::cout << "Iterations: " << iterations << std::endl;
        std::cout << std::string(50, '-') << std::endl;
        
        auto radar_data = generateRadarData(target_count);
        auto thermal_data = generateThermalData(target_count);
        auto optical_data = generateOpticalData(target_count);
        
        // Benchmark radar detection
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            auto targets = detector.detectRadarTargets(radar_data);
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto radar_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        
        // Benchmark thermal detection
        start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            auto targets = detector.detectThermalTargets(thermal_data);
        }
        end = std::chrono::high_resolution_clock::now();
        auto thermal_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        
        // Benchmark optical detection
        start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            auto targets = detector.detectOpticalTargets(optical_data);
        }
        end = std::chrono::high_resolution_clock::now();
        auto optical_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        
        // Benchmark sensor fusion
        start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            auto radar_targets = detector.detectRadarTargets(radar_data);
            auto thermal_targets = detector.detectThermalTargets(thermal_data);
            auto optical_targets = detector.detectOpticalTargets(optical_data);
            auto fused = detector.fuseSensors(radar_targets, thermal_targets, optical_targets);
        }
        end = std::chrono::high_resolution_clock::now();
        auto fusion_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        
        // Print results
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Radar Detection:    " << radar_time / 1000.0 << " ms (" << radar_time / (double)iterations << " μs per call)" << std::endl;
        std::cout << "Thermal Detection:  " << thermal_time / 1000.0 << " ms (" << thermal_time / (double)iterations << " μs per call)" << std::endl;
        std::cout << "Optical Detection:  " << optical_time / 1000.0 << " ms (" << optical_time / (double)iterations << " μs per call)" << std::endl;
        std::cout << "Sensor Fusion:      " << fusion_time / 1000.0 << " ms (" << fusion_time / (double)iterations << " μs per call)" << std::endl;
        std::cout << "Total Pipeline:     " << (radar_time + thermal_time + optical_time + fusion_time) / 1000.0 << " ms" << std::endl;
        
        // Memory usage estimation
        size_t estimated_memory = (radar_data.size() + thermal_data.size() + optical_data.size()) * sizeof(std::vector<double>) * 4;
        std::cout << "Estimated Memory:   " << estimated_memory / 1024 << " KB" << std::endl;
    }
    
    void benchmarkScalability() {
        std::cout << "\n=== SCALABILITY BENCHMARK ===" << std::endl;
        std::cout << std::string(50, '-') << std::endl;
        
        std::vector<size_t> target_counts = {10, 50, 100, 500, 1000};
        
        for (size_t count : target_counts) {
            auto radar_data = generateRadarData(count);
            auto thermal_data = generateThermalData(count);
            auto optical_data = generateOpticalData(count);
            
            // Benchmark full pipeline
            auto start = std::chrono::high_resolution_clock::now();
            
            auto radar_targets = detector.detectRadarTargets(radar_data);
            auto thermal_targets = detector.detectThermalTargets(thermal_data);
            auto optical_targets = detector.detectOpticalTargets(optical_data);
            auto fused = detector.fuseSensors(radar_targets, thermal_targets, optical_targets);
            
            auto end = std::chrono::high_resolution_clock::now();
            auto total_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            
            std::cout << "Targets: " << std::setw(4) << count << " | "
                      << "Time: " << std::setw(6) << total_time << " μs | "
                      << "Per Target: " << std::setw(4) << (total_time / (double)count) << " μs | "
                      << "Fused: " << std::setw(3) << fused.size() << std::endl;
        }
    }
    
    void runFullBenchmark() {
        std::cout << "Sentinel Fusion Sim - Performance Benchmark" << std::endl;
        std::cout << "=============================================" << std::endl;
        
        // Different workload sizes
        std::vector<size_t> workloads = {50, 100, 500};
        
        for (size_t workload : workloads) {
            benchmarkDetection(workload);
        }
        
        benchmarkScalability();
        
        std::cout << "\n=== OPTIMIZATION SUMMARY ===" << std::endl;
        std::cout << "✓ Memory pre-allocation enabled" << std::endl;
        std::cout << "✓ Spatial proximity optimization" << std::endl;
        std::cout << "✓ Early exit algorithms" << std::endl;
        std::cout << "✓ Pre-computed sensor weights" << std::endl;
        std::cout << "✓ Move semantics for large objects" << std::endl;
        std::cout << "✓ Thread-local random generators" << std::endl;
    }
};

int main() {
    PerformanceBenchmark benchmark;
    benchmark.runFullBenchmark();
    
    return 0;
}