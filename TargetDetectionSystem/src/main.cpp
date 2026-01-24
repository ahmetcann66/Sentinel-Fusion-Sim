#include <iostream>
#include <vector>
#include <random>
#include "TargetDetector.h"

// Rastgele radar verisi üreten yardımcı fonksiyon
std::vector<std::vector<double>> generateMockData(int count) {
    std::vector<std::vector<double>> data;
    std::mt19937 gen(42); // Sabit seed (her seferinde aynı sayıları üretir)
    std::uniform_real_distribution<> pos_dist(0.0, 2000.0); // 0-2000m arası mesafe
    std::uniform_real_distribution<> vel_dist(-300.0, 300.0); // Hız

    for(int i = 0; i < count; ++i) {
        data.push_back({
            pos_dist(gen), // x
            pos_dist(gen), // y
            pos_dist(gen), // z
            vel_dist(gen)  // velocity
        });
    }
    return data;
}

int main() {
    std::cout << "--- Radar System Initializing ---\n";

    // 1. Radar verisi oluştur (10 tane sinyal)
    auto raw_data = generateMockData(10);
    
    // 2. Dedektörü başlat (Eşik değeri: 0.3)
    TargetDetector detector(0.3);

    // 3. Hedefleri tespit et
    std::cout << "Processing " << raw_data.size() << " signals...\n";
    auto targets = detector.detectRadarTargets(raw_data);

    // 4. Sonuçları ekrana bas
    detector.printTargets();

    std::cout << "\nTotal Targets Detected: " << detector.getTargetCount() << "\n";

    return 0;
}