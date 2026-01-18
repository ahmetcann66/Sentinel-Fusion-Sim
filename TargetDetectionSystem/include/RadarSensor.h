#ifndef RADAR_SENSOR_H
#define RADAR_SENSOR_H

#include <vector>
#include <cmath>
#include <chrono>
#include <memory>
#include <random>
#include <mutex>
#include <atomic>
#include <span>
#include <optional>
#include <complex>

namespace SentinelFusion {

struct RadarConfig {
    double frequency = 10e9;          // 10 GHz X-band
    double bandwidth = 50e6;          // 50 MHz
    double max_range = 50000.0;       // 50 km
    double min_range = 100.0;         // 100 m
    double resolution = 1.0;         // 1 m
    double pulse_width = 1e-6;       // 1 microsecond
    double prf = 1000.0;             // Pulse Repetition Frequency
    double antenna_gain = 30.0;       // dBi
    double noise_figure = 3.0;       // dB
    double scan_rate = 12.0;         // RPM
    double azimuth_fov = 360.0;       // degrees
    double elevation_fov = 45.0;      // degrees
};

struct RadarDetection {
    double range;
    double azimuth;
    double elevation;
    double radial_velocity;
    double rcs;                     // Radar Cross Section (m²)
    double signal_strength;
    double snr;
    std::chrono::system_clock::time_point timestamp;
    uint32_t detection_id;
    bool is_valid = true;
};

struct RadarSignal {
    std::complex<double> i_data;    // In-phase component
    std::complex<double> q_data;     // Quadrature component
    double magnitude;
    double phase;
    double frequency;
};

class RadarSensor {
private:
    RadarConfig config;
    mutable std::mutex sensor_mutex;
    std::mt19937 rng;
    std::normal_distribution<double> noise_gen;
    
    // Performance metrics
    std::atomic<uint64_t> total_transmissions{0};
    std::atomic<uint64_t> valid_detections{0};
    std::atomic<double> average_noise_floor{1e-12};
    
    // Signal processing buffers
    std::vector<RadarSignal> signal_buffer;
    std::vector<RadarDetection> detection_buffer;
    
    // Physics constants
    static constexpr double SPEED_OF_LIGHT = 299792458.0;  // m/s
    static constexpr double BOLTZMANN_CONSTANT = 1.380649e-23;  // J/K
    static constexpr double STANDARD_TEMPERATURE = 290.0;  // Kelvin
    
    // Private methods
    double calculateDopplerShift(double radial_velocity) const noexcept;
    double calculateReceivedPower(double tx_power, double range, double rcs) const noexcept;
    double calculateNoisePower(double bandwidth) const noexcept;
    double calculateSNR(double signal_power, double noise_power) const noexcept;
    double applyAtmosphericAttenuation(double range, double frequency) const noexcept;
    double calculateMultipathFading(double range, double elevation) const noexcept;
    std::complex<double> generateThermalNoise() const noexcept;
    RadarSignal processReturnSignal(const RadarSignal& transmitted, double range, double rcs, double velocity) const;
    bool validateDetection(const RadarDetection& detection) const noexcept;
    
public:
    explicit RadarSensor(const RadarConfig& cfg = RadarConfig{});
    ~RadarSensor() = default;
    
    // Rule of Five
    RadarSensor(const RadarSensor&) = delete;
    RadarSensor& operator=(const RadarSensor&) = delete;
    RadarSensor(RadarSensor&&) = default;
    RadarSensor& operator=(RadarSensor&&) = default;
    
    // Core radar operations
    [[nodiscard]] std::vector<RadarDetection> performScan(
        double azimuth_start = 0.0,
        double azimuth_end = 360.0,
        double azimuth_step = 1.0
    );
    
    [[nodiscard]] std::vector<RadarDetection> trackTarget(
        double range,
        double azimuth, 
        double elevation,
        double duration_seconds = 1.0
    );
    
    // Environmental modeling
    [[nodiscard]] std::vector<RadarDetection> detectWithClutter(
        const std::vector<std::pair<double, double>>& targets,  // range, rcs pairs
        double clutter_density = 0.1,
        double weather_factor = 1.0
    );
    
    // Noise and interference modeling
    [[nodiscard]] RadarDetection addRealisticNoise(const RadarDetection& clean_detection) const;
    [[nodiscard]] std::vector<RadarDetection> simulateJamming(
        const std::vector<RadarDetection>& original_detections,
        double jammer_power,
        double jammer_range,
        double jammer_azimuth
    ) const;
    
    // Configuration
    void updateConfig(const RadarConfig& new_config) noexcept;
    [[nodiscard]] const RadarConfig& getConfig() const noexcept;
    
    // Performance monitoring
    [[nodiscard]] double getDetectionProbability(double rcs, double range) const noexcept;
    [[nodiscard]] double getFalseAlarmRate() const noexcept;
    [[nodiscard]] std::optional<RadarDetection> predictNextDetection(
        const RadarDetection& current_detection,
        double time_step = 0.1
    ) const;
    
    // Utility methods
    [[nodiscard]] std::string detectionToString(const RadarDetection& detection) const noexcept;
    void printDetectionReport(const std::vector<RadarDetection>& detections) const;
    [[nodiscard]] double calculateResolutionAtRange(double range) const noexcept;
    [[nodiscard]] bool isInRange(double range) const noexcept;
    
    // Advanced features
    [[nodiscard]] std::vector<RadarDetection> performMTIScan();  // Moving Target Indication
    [[nodiscard]] std::vector<RadarDetection> performPDScan();    // Pulse Doppler
    [[nodiscard]] std::vector<RadarDetection> performSARScan();    // Synthetic Aperture Radar
    
    // Statistics and health monitoring
    [[nodiscard]] double getAverageNoiseFloor() const noexcept;
    [[nodiscard]] uint64_t getTotalTransmissions() const noexcept;
    [[nodiscard]] uint64_t getValidDetections() const noexcept;
    void resetStatistics() noexcept;
    [[nodiscard]] bool isHealthy() const noexcept;
};

} // namespace SentinelFusion

#endif // RADAR_SENSOR_H