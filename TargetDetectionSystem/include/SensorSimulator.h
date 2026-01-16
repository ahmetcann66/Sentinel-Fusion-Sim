#ifndef SENSOR_SIMULATOR_H
#define SENSOR_SIMULATOR_H

#include <vector>
#include <chrono>
#include <random>
#include <cmath>
#include <span>
#include <optional>
#include <atomic>
#include <mutex>

enum class WeatherCondition : uint8_t {
    CLEAR = 0,
    CLOUDY = 1,
    RAIN = 2,
    FOG = 3,
    STORM = 4
};

struct SensorConfig {
    double noise_level;
    double accuracy;
    double max_range;
    double min_range;
    double detection_probability;
};

struct EnvironmentalFactors {
    WeatherCondition weather;
    double temperature;
    double humidity;
    double wind_speed;
    double atmospheric_pressure;
};

class EnhancedSensorSimulator {
private:
    mutable std::mutex rng_mutex;
    std::mt19937 rng;
    std::normal_distribution<double> gaussian_noise;
    std::atomic<double> sensor_reliability{1.0};
    std::atomic<bool> failure_simulation{false};
    
    double calculateWeatherImpact(WeatherCondition weather, double base_signal);
    double addGaussianNoise(double value, double std_dev);
    double calculateDopplerShift(double velocity, double frequency);
    double calculateSignalAttenuation(double distance, double frequency, WeatherCondition weather);
    
public:
    explicit EnhancedSensorSimulator(uint32_t seed = std::random_device{}());
    ~EnhancedSensorSimulator() = default;
    
    // Rule of Five
    EnhancedSensorSimulator(const EnhancedSensorSimulator&) = delete;
    EnhancedSensorSimulator& operator=(const EnhancedSensorSimulator&) = delete;
    EnhancedSensorSimulator(EnhancedSensorSimulator&&) = delete;
    EnhancedSensorSimulator& operator=(EnhancedSensorSimulator&&) = delete;
    
    // Thread-safe sensor configurations
    [[nodiscard]] SensorConfig getRadarConfig() const noexcept;
    [[nodiscard]] SensorConfig getThermalConfig() const noexcept;
    [[nodiscard]] SensorConfig getOpticalConfig() const noexcept;
    
    // Environmental simulation
    [[nodiscard]] EnvironmentalFactors generateEnvironmentalConditions();
    [[nodiscard]] double calculateEnvironmentalNoise(const EnvironmentalFactors& env, double base_noise) const noexcept;
    
    // Enhanced data generation with noise and modern C++ signatures
    [[nodiscard]] std::vector<std::vector<double>> generateRealisticRadarData(
        int num_targets, 
        const EnvironmentalFactors& env = {}
    );
    
    [[nodiscard]] std::vector<std::vector<double>> generateRealisticThermalData(
        int num_targets,
        const EnvironmentalFactors& env = {}
    );
    
    [[nodiscard]] std::vector<std::vector<double>> generateRealisticOpticalData(
        int num_targets,
        const EnvironmentalFactors& env = {}
    );
    
    // Advanced target movement simulation with optimized return
    [[nodiscard]] std::vector<std::vector<double>> simulateMovingTarget(
        double start_x, double start_y, double start_z,
        double velocity_x, double velocity_y, double velocity_z,
        int time_steps, 
        const EnvironmentalFactors& env = {}
    );
    
    // Thread-safe utility methods
    [[nodiscard]] double calculateSignalToNoiseRatio(double signal, double noise) const noexcept;
    [[nodiscard]] bool isDetectable(double snr, double threshold = 6.0) const noexcept;
    [[nodiscard]] std::string weatherToString(WeatherCondition weather) const noexcept;
    
    // Performance metrics with atomic operations
    void simulateSensorFailure(double failure_probability) noexcept;
    [[nodiscard]] double getSensorReliability() const noexcept;
    void resetSensorReliability() noexcept;
};

#endif