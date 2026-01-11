#ifndef SENSOR_SIMULATOR_H
#define SENSOR_SIMULATOR_H

#include <vector>
#include <chrono>
#include <random>
#include <cmath>

enum class WeatherCondition {
    CLEAR,
    CLOUDY,
    RAIN,
    FOG,
    STORM
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
    std::mt19937 rng;
    std::normal_distribution<double> gaussian_noise;
    
    double calculateWeatherImpact(WeatherCondition weather, double base_signal);
    double addGaussianNoise(double value, double std_dev);
    double calculateDopplerShift(double velocity, double frequency);
    double calculateSignalAttenuation(double distance, double frequency, WeatherCondition weather);
    
public:
    EnhancedSensorSimulator();
    
    // Sensor configurations
    SensorConfig getRadarConfig() const;
    SensorConfig getThermalConfig() const;
    SensorConfig getOpticalConfig() const;
    
    // Environmental simulation
    EnvironmentalFactors generateEnvironmentalConditions();
    double calculateEnvironmentalNoise(const EnvironmentalFactors& env, double base_noise);
    
    // Enhanced data generation with noise
    std::vector<std::vector<double>> generateRealisticRadarData(
        int num_targets, 
        const EnvironmentalFactors& env = {}
    );
    
    std::vector<std::vector<double>> generateRealisticThermalData(
        int num_targets,
        const EnvironmentalFactors& env = {}
    );
    
    std::vector<std::vector<double>> generateRealisticOpticalData(
        int num_targets,
        const EnvironmentalFactors& env = {}
    );
    
    // Advanced target movement simulation
    std::vector<std::vector<double>> simulateMovingTarget(
        double start_x, double start_y, double start_z,
        double velocity_x, double velocity_y, double velocity_z,
        int time_steps, 
        const EnvironmentalFactors& env = {}
    );
    
    // Utility methods
    double calculateSignalToNoiseRatio(double signal, double noise);
    bool isDetectable(double snr, double threshold = 6.0);
    std::string weatherToString(WeatherCondition weather);
    
    // Performance metrics
    void simulateSensorFailure(double failure_probability);
    double getSensorReliability() const;
};

#endif