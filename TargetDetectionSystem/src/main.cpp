#include "../include/TargetDetector.h"
#include "../include/SensorSimulator.h"
#include <vector>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <iterator>

void demonstrateEnhancedSystem() {
    TargetDetector detector;
    EnhancedSensorSimulator sensor_sim;
    
    std::cout << "=== SAVUNMA SANAYİ GELİŞMİŞ HEDEF TESPİT SİSTEMİ ===" << std::endl;
    std::cout << "Sistem başlatılıyor..." << std::endl << std::endl;
    
    // Generate environmental conditions
    auto env_conditions = sensor_sim.generateEnvironmentalConditions();
    std::cout << "🌍 Çevre Koşulları:" << std::endl;
    std::cout << "   Hava Durumu: " << sensor_sim.weatherToString(env_conditions.weather) << std::endl;
    std::cout << "   Sıcaklık: " << env_conditions.temperature << "°C" << std::endl;
    std::cout << "   Nem: %" << env_conditions.humidity << std::endl;
    std::cout << "   Rüzgar Hızı: " << env_conditions.wind_speed << " km/s" << std::endl;
    std::cout << "   Basınç: " << env_conditions.atmospheric_pressure << " hPa" << std::endl << std::endl;
    
    // Generate realistic sensor data with environmental effects
    auto radar_data = sensor_sim.generateRealisticRadarData(15, env_conditions);
    auto thermal_data = sensor_sim.generateRealisticThermalData(12, env_conditions);
    auto optical_data = sensor_sim.generateRealisticOpticalData(10, env_conditions);
    
    // Sensor configurations
    auto radar_config = sensor_sim.getRadarConfig();
    auto thermal_config = sensor_sim.getThermalConfig();
    auto optical_config = sensor_sim.getOpticalConfig();
    
    std::cout << "📡 Sensor Konfigürasyonları:" << std::endl;
    std::cout << "   Radar - Gürültü: %" << (radar_config.noise_level * 100) 
              << ", Doğruluk: %" << (radar_config.accuracy * 100) 
              << ", Menzil: " << radar_config.max_range << " km" << std::endl;
    std::cout << "   Termal - Gürültü: %" << (thermal_config.noise_level * 100) 
              << ", Doğruluk: %" << (thermal_config.accuracy * 100) 
              << ", Menzil: " << thermal_config.max_range << " km" << std::endl;
    std::cout << "   Optik - Gürültü: %" << (optical_config.noise_level * 100) 
              << ", Doğruluk: %" << (optical_config.accuracy * 100) 
              << ", Menzil: " << optical_config.max_range << " km" << std::endl << std::endl;
    
    std::cout << "🎯 Gerçekçi Sensor Verileri Toplandı:" << std::endl;
    std::cout << "   Radar okumaları: " << radar_data.size() << std::endl;
    std::cout << "   Termal okumalar: " << thermal_data.size() << std::endl;
    std::cout << "   Optik okumalar: " << optical_data.size() << std::endl << std::endl;
    
    auto radar_targets = detector.detectRadarTargets(radar_data);
    auto thermal_targets = detector.detectThermalTargets(thermal_data);
    auto optical_targets = detector.detectOpticalTargets(optical_data);
    
    std::cout << "Tespit edilen hedefler (onay öncesi):" << std::endl;
    std::cout << "Radar hedefleri: " << radar_targets.size() << std::endl;
    std::cout << "Termal hedefler: " << thermal_targets.size() << std::endl;
    std::cout << "Optik hedefler: " << optical_targets.size() << std::endl << std::endl;
    
    detector.filterNoise(radar_targets);
    detector.filterNoise(thermal_targets);
    detector.filterNoise(optical_targets);
    
    auto fused_targets = detector.fuseSensors(radar_targets, thermal_targets, optical_targets);
    detector.trackTargets(fused_targets);
    
detector = TargetDetector();
    
    for (const auto& target : fused_targets) {
        detector.addTarget(target);
    }
    
    detector.printTargets();
    
    std::cout << "=== SİSTEM ÖZETİ ===" << std::endl;
    std::cout << "Toplam füze hedefi: " << fused_targets.size() << std::endl;
    std::cout << "Yüksek öncelikli hedefler (>70% güven): " << std::count_if(fused_targets.begin(), fused_targets.end(),
                                                    [](const Target& t) { return t.confidence > 0.7; }) << std::endl;
    std::cout << "Hızlı hedefler (>5 m/s): " << std::count_if(fused_targets.begin(), fused_targets.end(),
                                           [](const Target& t) { return t.velocity > 5.0; }) << std::endl;
}

int main() {
    try {
        demonstrateEnhancedSystem();
    } catch (const std::exception& e) {
        std::cerr << "❌ Hata: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}