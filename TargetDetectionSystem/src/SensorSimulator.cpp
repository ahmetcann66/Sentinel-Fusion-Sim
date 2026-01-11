#include "SensorSimulator.h"
#include <iostream>
#include <algorithm>

EnhancedSensorSimulator::EnhancedSensorSimulator() 
    : rng(std::chrono::steady_clock::now().time_since_epoch().count()),
      gaussian_noise(0.0, 1.0) {
}

SensorConfig EnhancedSensorSimulator::getRadarConfig() const {
    return {
        .noise_level = 0.15,      // 15% noise
        .accuracy = 0.95,         // 95% accuracy
        .max_range = 500.0,        // 500 km max range
        .min_range = 1.0,          // 1 km min range
        .detection_probability = 0.92
    };
}

SensorConfig EnhancedSensorSimulator::getThermalConfig() const {
    return {
        .noise_level = 0.20,      // 20% noise
        .accuracy = 0.88,         // 88% accuracy
        .max_range = 150.0,        // 150 km max range
        .min_range = 0.5,          // 0.5 km min range
        .detection_probability = 0.85
    };
}

SensorConfig EnhancedSensorSimulator::getOpticalConfig() const {
    return {
        .noise_level = 0.25,      // 25% noise
        .accuracy = 0.90,         // 90% accuracy
        .max_range = 100.0,        // 100 km max range
        .min_range = 0.1,          // 100m min range
        .detection_probability = 0.80
    };
}

EnvironmentalFactors EnhancedSensorSimulator::generateEnvironmentalConditions() {
    std::uniform_int_distribution<int> weather_dist(0, 4);
    std::uniform_real_distribution<double> temp_dist(-10.0, 40.0);
    std::uniform_real_distribution<double> humidity_dist(20.0, 95.0);
    std::uniform_real_distribution<double> wind_dist(0.0, 25.0);
    std::uniform_real_distribution<double> pressure_dist(980.0, 1040.0);
    
    return {
        .weather = static_cast<WeatherCondition>(weather_dist(rng)),
        .temperature = temp_dist(rng),
        .humidity = humidity_dist(rng),
        .wind_speed = wind_dist(rng),
        .atmospheric_pressure = pressure_dist(rng)
    };
}

double EnhancedSensorSimulator::calculateWeatherImpact(WeatherCondition weather, double base_signal) {
    switch (weather) {
        case WeatherCondition::CLEAR:
            return 1.0;  // No impact
        case WeatherCondition::CLOUDY:
            return 0.85; // 15% signal loss
        case WeatherCondition::RAIN:
            return 0.70; // 30% signal loss
        case WeatherCondition::FOG:
            return 0.60; // 40% signal loss
        case WeatherCondition::STORM:
            return 0.45; // 55% signal loss
        default:
            return 1.0;
    }
}

double EnhancedSensorSimulator::addGaussianNoise(double value, double std_dev) {
    return value + gaussian_noise(rng) * std_dev;
}

double EnhancedSensorSimulator::calculateSignalAttenuation(double distance, double frequency, WeatherCondition weather) {
    // Free space path loss formula
    double path_loss = 20 * log10(distance) + 20 * log10(frequency) - 147.55;
    
    // Weather impact
    double weather_factor = calculateWeatherImpact(weather, 1.0);
    
    // Convert dB to linear scale
    return pow(10, -path_loss / 10.0) * weather_factor;
}

double EnhancedSensorSimulator::calculateEnvironmentalNoise(const EnvironmentalFactors& env, double base_noise) {
    double weather_noise = 0.0;
    
    switch (env.weather) {
        case WeatherCondition::RAIN:
            weather_noise = 0.1 + (env.wind_speed / 100.0);
            break;
        case WeatherCondition::STORM:
            weather_noise = 0.2 + (env.wind_speed / 50.0);
            break;
        case WeatherCondition::FOG:
            weather_noise = 0.15;
            break;
        default:
            weather_noise = 0.05;
    }
    
    double temperature_noise = std::abs(env.temperature - 20.0) / 100.0;
    double humidity_noise = env.humidity / 200.0;
    
    return base_noise + weather_noise + temperature_noise + humidity_noise;
}

std::vector<std::vector<double>> EnhancedSensorSimulator::generateRealisticRadarData(
    int num_targets, const EnvironmentalFactors& env) {
    
    std::vector<std::vector<double>> data;
    auto config = getRadarConfig();
    
    std::uniform_real_distribution<double> range_dist(config.min_range, config.max_range);
    std::uniform_real_distribution<double> angle_dist(-180.0, 180.0);
    std::uniform_real_distribution<double> altitude_dist(0.0, 20.0);
    std::uniform_real_distribution<double> velocity_dist(0.0, 300.0);
    
    double environmental_noise = calculateEnvironmentalNoise(env, config.noise_level);
    
    for (int i = 0; i < num_targets; i++) {
        // Detection probability check
        std::uniform_real_distribution<double> detection_check(0.0, 1.0);
        if (detection_check(rng) > config.detection_probability) {
            continue; // Target not detected
        }
        
        double range = range_dist(rng);
        double angle = angle_dist(rng) * M_PI / 180.0;
        double altitude = altitude_dist(rng);
        double velocity = velocity_dist(rng);
        
        // Convert polar to cartesian
        double x = range * cos(angle);
        double y = range * sin(angle);
        double z = altitude;
        
        // Add environmental effects
        double weather_impact = calculateWeatherImpact(env.weather, 1.0);
        double signal_strength = (1.0 - range / config.max_range) * weather_impact;
        signal_strength = addGaussianNoise(signal_strength, environmental_noise);
        signal_strength = std::max(0.0, std::min(1.0, signal_strength));
        
        // Add noise to position
        x = addGaussianNoise(x, environmental_noise * 5.0);
        y = addGaussianNoise(y, environmental_noise * 5.0);
        z = addGaussianNoise(z, environmental_noise * 0.5);
        velocity = addGaussianNoise(velocity, environmental_noise * 10.0);
        
        std::vector<double> reading = {x, y, z, signal_strength, velocity};
        data.push_back(reading);
    }
    
    return data;
}

std::vector<std::vector<double>> EnhancedSensorSimulator::generateRealisticThermalData(
    int num_targets, const EnvironmentalFactors& env) {
    
    std::vector<std::vector<double>> data;
    auto config = getThermalConfig();
    
    std::uniform_real_distribution<double> range_dist(config.min_range, config.max_range);
    std::uniform_real_distribution<double> angle_dist(-180.0, 180.0);
    std::uniform_real_distribution<double> altitude_dist(0.0, 15.0);
    std::uniform_real_distribution<double> velocity_dist(0.0, 200.0);
    
    double environmental_noise = calculateEnvironmentalNoise(env, config.noise_level);
    
    for (int i = 0; i < num_targets; i++) {
        std::uniform_real_distribution<double> detection_check(0.0, 1.0);
        if (detection_check(rng) > config.detection_probability) {
            continue;
        }
        
        double range = range_dist(rng);
        double angle = angle_dist(rng) * M_PI / 180.0;
        double altitude = altitude_dist(rng);
        double velocity = velocity_dist(rng);
        
        double x = range * cos(angle);
        double y = range * sin(angle);
        double z = altitude;
        
        // Thermal signature calculation
        double base_temperature = 25.0 + (velocity / 20.0); // Heat from friction
        double weather_impact = calculateWeatherImpact(env.weather, 1.0);
        double temperature = base_temperature * weather_impact + env.temperature * 0.1;
        temperature = addGaussianNoise(temperature, environmental_noise * 2.0);
        
        // Add position noise
        x = addGaussianNoise(x, environmental_noise * 8.0);
        y = addGaussianNoise(y, environmental_noise * 8.0);
        z = addGaussianNoise(z, environmental_noise * 1.0);
        
        std::vector<double> reading = {x, y, z, temperature, velocity};
        data.push_back(reading);
    }
    
    return data;
}

std::vector<std::vector<double>> EnhancedSensorSimulator::generateRealisticOpticalData(
    int num_targets, const EnvironmentalFactors& env) {
    
    std::vector<std::vector<double>> data;
    auto config = getOpticalConfig();
    
    std::uniform_real_distribution<double> range_dist(config.min_range, config.max_range);
    std::uniform_real_distribution<double> angle_dist(-180.0, 180.0);
    std::uniform_real_distribution<double> altitude_dist(0.0, 10.0);
    std::uniform_real_distribution<double> velocity_dist(0.0, 150.0);
    
    double environmental_noise = calculateEnvironmentalNoise(env, config.noise_level);
    
    for (int i = 0; i < num_targets; i++) {
        std::uniform_real_distribution<double> detection_check(0.0, 1.0);
        if (detection_check(rng) > config.detection_probability) {
            continue;
        }
        
        double range = range_dist(rng);
        double angle = angle_dist(rng) * M_PI / 180.0;
        double altitude = altitude_dist(rng);
        double velocity = velocity_dist(rng);
        
        double x = range * cos(angle);
        double y = range * sin(angle);
        double z = altitude;
        
        // Optical signature
        double weather_impact = calculateWeatherImpact(env.weather, 1.0);
        double brightness = (1.0 - range / config.max_range) * weather_impact;
        double contrast = brightness * (0.7 + 0.3 * weather_impact);
        
        brightness = addGaussianNoise(brightness, environmental_noise * 0.2);
        contrast = addGaussianNoise(contrast, environmental_noise * 0.15);
        brightness = std::max(0.0, std::min(1.0, brightness));
        contrast = std::max(0.0, std::min(1.0, contrast));
        
        // Add position noise (optical sensors are more precise)
        x = addGaussianNoise(x, environmental_noise * 3.0);
        y = addGaussianNoise(y, environmental_noise * 3.0);
        z = addGaussianNoise(z, environmental_noise * 0.3);
        
        std::vector<double> reading = {x, y, z, brightness, contrast};
        data.push_back(reading);
    }
    
    return data;
}

double EnhancedSensorSimulator::calculateSignalToNoiseRatio(double signal, double noise) {
    if (noise <= 0) return 100.0; // Perfect SNR
    return 20 * log10(signal / noise);
}

bool EnhancedSensorSimulator::isDetectable(double snr, double threshold) {
    return snr >= threshold;
}

std::string EnhancedSensorSimulator::weatherToString(WeatherCondition weather) {
    switch (weather) {
        case WeatherCondition::CLEAR: return "Clear";
        case WeatherCondition::CLOUDY: return "Cloudy";
        case WeatherCondition::RAIN: return "Rain";
        case WeatherCondition::FOG: return "Fog";
        case WeatherCondition::STORM: return "Storm";
        default: return "Unknown";
    }
}

void EnhancedSensorSimulator::simulateSensorFailure(double failure_probability) {
    std::uniform_real_distribution<double> failure_check(0.0, 1.0);
    if (failure_check(rng) < failure_probability) {
        std::cout << "⚠️  Sensor failure simulated!" << std::endl;
    }
}

double EnhancedSensorSimulator::getSensorReliability() const {
    return 0.95; // 95% reliability
}