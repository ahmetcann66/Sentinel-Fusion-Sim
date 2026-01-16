#include "SensorSimulator.h"
#include <iostream>
#include <algorithm>
#include <thread>
#include <future>

// Modern constructor with seed parameter
EnhancedSensorSimulator::EnhancedSensorSimulator(uint32_t seed) 
    : rng(seed), gaussian_noise(0.0, 1.0) {
}

// Thread-safe sensor configurations
SensorConfig EnhancedSensorSimulator::getRadarConfig() const noexcept {
    return {
        .noise_level = 0.15,      // 15% noise
        .accuracy = 0.95,         // 95% accuracy
        .max_range = 500.0,        // 500 km max range
        .min_range = 1.0,          // 1 km min range
        .detection_probability = 0.92
    };
}

SensorConfig EnhancedSensorSimulator::getThermalConfig() const noexcept {
    return {
        .noise_level = 0.20,      // 20% noise
        .accuracy = 0.88,         // 88% accuracy
        .max_range = 150.0,        // 150 km max range
        .min_range = 0.5,          // 0.5 km min range
        .detection_probability = 0.85
    };
}

SensorConfig EnhancedSensorSimulator::getOpticalConfig() const noexcept {
    return {
        .noise_level = 0.25,      // 25% noise
        .accuracy = 0.90,         // 90% accuracy
        .max_range = 100.0,        // 100 km max range
        .min_range = 0.1,          // 100m min range
        .detection_probability = 0.80
    };
}

// Thread-safe environmental factors generation
EnvironmentalFactors EnhancedSensorSimulator::generateEnvironmentalConditions() {
    std::lock_guard<std::mutex> lock(rng_mutex);
    
    std::uniform_int_distribution<int> weather_dist(0, 4);
    std::uniform_real_distribution<double> temp_dist(-10.0, 40.0);
    std::uniform_real_distribution<double> humidity_dist(20.0, 95.0);
    std::uniform_real_distribution<double> wind_dist(0.0, 25.0);
    std::uniform_real_distribution<double> pressure_dist(980.0, 1040.0);
    
    return EnvironmentalFactors{
        .weather = static_cast<WeatherCondition>(weather_dist(rng)),
        .temperature = temp_dist(rng),
        .humidity = humidity_dist(rng),
        .wind_speed = wind_dist(rng),
        .atmospheric_pressure = pressure_dist(rng)
    };
}

// Thread-safe environmental noise calculation
double EnhancedSensorSimulator::calculateEnvironmentalNoise(
    const EnvironmentalFactors& env, double base_noise) const noexcept {
    
    double weather_factor = 1.0;
    switch (env.weather) {
        case WeatherCondition::CLEAR: weather_factor = 1.0; break;
        case WeatherCondition::CLOUDY: weather_factor = 1.2; break;
        case WeatherCondition::RAIN: weather_factor = 1.5; break;
        case WeatherCondition::FOG: weather_factor = 2.0; break;
        case WeatherCondition::STORM: weather_factor = 2.5; break;
    }
    
    const double temperature_factor = std::abs(env.temperature - 20.0) / 30.0;
    const double humidity_factor = env.humidity / 100.0;
    const double wind_factor = std::min(env.wind_speed / 25.0, 1.0);
    
    return base_noise * weather_factor * (1.0 + temperature_factor + humidity_factor + wind_factor) / 4.0;
}

// Enhanced radar data generation with environmental effects
std::vector<std::vector<double>> EnhancedSensorSimulator::generateRealisticRadarData(
    int num_targets, const EnvironmentalFactors& env) {
    
    std::vector<std::vector<double>> radar_data;
    radar_data.reserve(num_targets);
    
    const auto config = getRadarConfig();
    const double env_noise = calculateEnvironmentalNoise(env, config.noise_level);
    
    std::lock_guard<std::mutex> lock(rng_mutex);
    
    std::uniform_real_distribution<double> x_dist(-config.max_range, config.max_range);
    std::uniform_real_distribution<double> y_dist(-config.max_range, config.max_range);
    std::uniform_real_distribution<double> z_dist(0.0, config.max_range / 2.0);
    std::uniform_real_distribution<double> signal_dist(0.3, 1.0);
    std::uniform_real_distribution<double> detection_dist(0.0, 1.0);
    
    for (int i = 0; i < num_targets; ++i) {
        if (detection_dist(rng) > config.detection_probability * sensor_reliability.load()) {
            continue; // Skip undetected targets
        }
        
        const double x = x_dist(rng);
        const double y = y_dist(rng);
        const double z = z_dist(rng);
        const double distance = std::sqrt(x*x + y*y + z*z);
        
        if (distance < config.min_range || distance > config.max_range) {
            continue;
        }
        
        double signal_strength = signal_dist(rng);
        
        // Apply environmental noise
        signal_strength += addGaussianNoise(signal_strength, env_noise);
        signal_strength = std::clamp(signal_strength, 0.0, 1.0);
        
        // Apply signal attenuation based on distance
        const double attenuation = calculateSignalAttenuation(distance, 10e9, env.weather);
        signal_strength *= attenuation;
        
        radar_data.push_back({x, y, z, signal_strength});
    }
    
    return radar_data;
}

// Enhanced thermal data generation
std::vector<std::vector<double>> EnhancedSensorSimulator::generateRealisticThermalData(
    int num_targets, const EnvironmentalFactors& env) {
    
    std::vector<std::vector<double>> thermal_data;
    thermal_data.reserve(num_targets);
    
    const auto config = getThermalConfig();
    const double env_noise = calculateEnvironmentalNoise(env, config.noise_level);
    
    std::lock_guard<std::mutex> lock(rng_mutex);
    
    std::uniform_real_distribution<double> x_dist(-config.max_range, config.max_range);
    std::uniform_real_distribution<double> y_dist(-config.max_range, config.max_range);
    std::uniform_real_distribution<double> z_dist(0.0, config.max_range / 3.0);
    std::uniform_real_distribution<double> temp_dist(25.0, 150.0); // Temperature in Celsius
    std::uniform_real_distribution<double> detection_dist(0.0, 1.0);
    
    for (int i = 0; i < num_targets; ++i) {
        if (detection_dist(rng) > config.detection_probability * sensor_reliability.load()) {
            continue;
        }
        
        const double x = x_dist(rng);
        const double y = y_dist(rng);
        const double z = z_dist(rng);
        const double distance = std::sqrt(x*x + y*y + z*z);
        
        if (distance < config.min_range || distance > config.max_range) {
            continue;
        }
        
        double temperature = temp_dist(rng);
        
        // Environmental effects on temperature detection
        const double weather_impact = calculateWeatherImpact(env.weather, temperature);
        temperature += addGaussianNoise(temperature, env_noise);
        temperature = std::clamp(temperature * weather_impact, -50.0, 200.0);
        
        thermal_data.push_back({x, y, z, temperature});
    }
    
    return thermal_data;
}

// Enhanced optical data generation
std::vector<std::vector<double>> EnhancedSensorSimulator::generateRealisticOpticalData(
    int num_targets, const EnvironmentalFactors& env) {
    
    std::vector<std::vector<double>> optical_data;
    optical_data.reserve(num_targets);
    
    const auto config = getOpticalConfig();
    const double env_noise = calculateEnvironmentalNoise(env, config.noise_level);
    
    std::lock_guard<std::mutex> lock(rng_mutex);
    
    std::uniform_real_distribution<double> x_dist(-config.max_range, config.max_range);
    std::uniform_real_distribution<double> y_dist(-config.max_range, config.max_range);
    std::uniform_real_distribution<double> z_dist(0.0, config.max_range / 4.0);
    std::uniform_real_distribution<double> brightness_dist(0.1, 1.0);
    std::uniform_real_distribution<double> contrast_dist(0.1, 1.0);
    std::uniform_real_distribution<double> detection_dist(0.0, 1.0);
    
    for (int i = 0; i < num_targets; ++i) {
        if (detection_dist(rng) > config.detection_probability * sensor_reliability.load()) {
            continue;
        }
        
        const double x = x_dist(rng);
        const double y = y_dist(rng);
        const double z = z_dist(rng);
        const double distance = std::sqrt(x*x + y*y + z*z);
        
        if (distance < config.min_range || distance > config.max_range) {
            continue;
        }
        
        double brightness = brightness_dist(rng);
        double contrast = contrast_dist(rng);
        
        // Environmental effects on optical detection
        const double weather_impact = calculateWeatherImpact(env.weather, brightness);
        brightness += addGaussianNoise(brightness, env_noise);
        contrast += addGaussianNoise(contrast, env_noise);
        
        brightness = std::clamp(brightness * weather_impact, 0.0, 1.0);
        contrast = std::clamp(contrast * weather_impact, 0.0, 1.0);
        
        optical_data.push_back({x, y, z, brightness, contrast});
    }
    
    return optical_data;
}

// Moving target simulation with physics
std::vector<std::vector<double>> EnhancedSensorSimulator::simulateMovingTarget(
    double start_x, double start_y, double start_z,
    double velocity_x, double velocity_y, double velocity_z,
    int time_steps, const EnvironmentalFactors& env) {
    
    std::vector<std::vector<double>> trajectory;
    trajectory.reserve(time_steps);
    
    const auto config = getRadarConfig();
    const double env_noise = calculateEnvironmentalNoise(env, config.noise_level);
    
    std::lock_guard<std::mutex> lock(rng_mutex);
    
    double x = start_x;
    double y = start_y;
    double z = start_z;
    
    for (int t = 0; t < time_steps; ++t) {
        // Apply physics with environmental effects
        const double dt = 0.1; // Time step in seconds
        
        // Wind effects
        const double wind_x = env.wind_speed * std::cos(t * 0.1) * 0.01;
        const double wind_y = env.wind_speed * std::sin(t * 0.1) * 0.01;
        
        // Update position with wind and gravity effects
        x += (velocity_x + wind_x) * dt;
        y += (velocity_y + wind_y) * dt;
        z += (velocity_z - 9.81 * dt) * dt; // Gravity effect
        
        // Add noise to simulate sensor inaccuracy
        const double noisy_x = x + addGaussianNoise(0.0, env_noise * 10.0);
        const double noisy_y = y + addGaussianNoise(0.0, env_noise * 10.0);
        const double noisy_z = z + addGaussianNoise(0.0, env_noise * 5.0);
        
        trajectory.push_back({noisy_x, noisy_y, noisy_z, static_cast<double>(t)});
    }
    
    return trajectory;
}

// Thread-safe utility methods
double EnhancedSensorSimulator::calculateSignalToNoiseRatio(double signal, double noise) const noexcept {
    if (noise <= 0) return signal > 0 ? 100.0 : 0.0;
    return 20.0 * std::log10(signal / noise);
}

bool EnhancedSensorSimulator::isDetectable(double snr, double threshold) const noexcept {
    return snr >= threshold && sensor_reliability.load() > 0.5;
}

std::string EnhancedSensorSimulator::weatherToString(WeatherCondition weather) const noexcept {
    switch (weather) {
        case WeatherCondition::CLEAR: return "Clear";
        case WeatherCondition::CLOUDY: return "Cloudy";
        case WeatherCondition::RAIN: return "Rain";
        case WeatherCondition::FOG: return "Fog";
        case WeatherCondition::STORM: return "Storm";
        default: return "Unknown";
    }
}

// Thread-safe sensor failure simulation
void EnhancedSensorSimulator::simulateSensorFailure(double failure_probability) noexcept {
    std::lock_guard<std::mutex> lock(rng_mutex);
    
    std::uniform_real_distribution<double> failure_dist(0.0, 1.0);
    if (failure_dist(rng) < failure_probability) {
        sensor_reliability.store(std::uniform_real_distribution<double>(0.1, 0.8)(rng));
        failure_simulation.store(true);
    }
}

double EnhancedSensorSimulator::getSensorReliability() const noexcept {
    return sensor_reliability.load();
}

void EnhancedSensorSimulator::resetSensorReliability() noexcept {
    sensor_reliability.store(1.0);
    failure_simulation.store(false);
}

// Private helper methods
double EnhancedSensorSimulator::calculateWeatherImpact(WeatherCondition weather, double base_signal) {
    switch (weather) {
        case WeatherCondition::CLEAR: return 1.0;
        case WeatherCondition::CLOUDY: return 0.9;
        case WeatherCondition::RAIN: return 0.7;
        case WeatherCondition::FOG: return 0.5;
        case WeatherCondition::STORM: return 0.3;
        default: return 1.0;
    }
}

double EnhancedSensorSimulator::addGaussianNoise(double value, double std_dev) {
    std::lock_guard<std::mutex> lock(rng_mutex);
    return value + gaussian_noise(rng) * std_dev;
}

double EnhancedSensorSimulator::calculateDopplerShift(double velocity, double frequency) {
    const double c = 299792458.0; // Speed of light in m/s
    return frequency * (1.0 + velocity / c);
}

double EnhancedSensorSimulator::calculateSignalAttenuation(
    double distance, double frequency, WeatherCondition weather) {
    
    // Friis transmission equation with weather effects
    const double c = 299792458.0; // Speed of light
    const double wavelength = c / frequency;
    
    double weather_loss = 1.0;
    switch (weather) {
        case WeatherCondition::CLEAR: weather_loss = 1.0; break;
        case WeatherCondition::CLOUDY: weather_loss = 0.95; break;
        case WeatherCondition::RAIN: weather_loss = 0.85; break;
        case WeatherCondition::FOG: weather_loss = 0.75; break;
        case WeatherCondition::STORM: weather_loss = 0.6; break;
    }
    
    // Free space path loss
    const double path_loss = std::pow(4.0 * M_PI * distance / wavelength, 2);
    
    return weather_loss / path_loss;
}