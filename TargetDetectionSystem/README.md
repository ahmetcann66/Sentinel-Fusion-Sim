# Radar Target Detection System

Savunma sanayii için geliştirilen basit ve etkili radar tabanlı hedef tespit sistemi.

## 🎯 Proje Amacı

Bu proje, çoklu sensör karmaşıklığı olmadan sadece radar verileri kullanarak:
- Hedef tespiti ve takibi
- Otomatik tehdit seviyesi belirleme  
- Gürültü filtreleme ve doğrulama
- Gerçek zamanlı hedef önceliklendirme

İşlemlerini gerçekleştiren temiz ve anlaşılır bir sistem sunar.

## 📊 Sistem Şeması

```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   Radar Verisi   │───▶│  Hedef Tespit    │───▶│   Gürültü       │
│   (x,y,z,sig)   │    │  Algoritması     │    │   Filtreleme    │
└─────────────────┘    └──────────────────┘    └─────────────────┘
                                                        │
                                                        ▼
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   Sonuçlar      │◀───│   Tehdit         │◀───│   Doğrulanmış   │
│   (Ekrana)      │    │   Analizi        │    │   Hedefler      │
└─────────────────┘    └──────────────────┘    └─────────────────┘
```

## 🚀 Kurulum Yönergeleri

### 1. Gereksinimler

**Sistem Gereksinimleri:**
- Linux, Windows veya macOS
- C++17 uyumlu derleyici (GCC 7+, Clang 5+, MSVC 2019+)
- CMake 3.20 veya üzeri

**Derleyici Kurulumu:**

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install build-essential cmake
```

**Windows (Visual Studio):**
```bash
# Visual Studio 2019+ kurulumu yeterli
# Veya Build Tools for Visual Studio
```

**macOS (Homebrew):**
```bash
brew install cmake
```

### 2. Proje Kurulumu

**Adım 1 - Projeyi Kopyala:**
```bash
cd ~/Projects
git clone <proje-linki>
cd Sentinel-Fusion-Sim/TargetDetectionSystem
```

**Adım 2 - Derleme:**
```bash
mkdir build && cd build
cmake ..
make
```

**Adım 3 - Çalıştırma:**
```bash
./radar_detection
```

### 3. Hızlı Test

Doğrudan derleme (CMake olmadan):
```bash
# Proje dizininde
g++ -std=c++17 -I./include src/TargetDetector.cpp src/main.cpp -o radar_detection
./radar_detection
```

## 🏗️ Proje Yapısı

```
TargetDetectionSystem/
├── 📁 include/
│   └── 📄 TargetDetector.h     # Ana sınıf tanımları
├── 📁 src/
│   ├── 📄 TargetDetector.cpp   # Tespit algoritmaları
│   └── 📄 main.cpp             # Demo uygulaması
├── 📄 CMakeLists.txt          # Derleme ayarları
└── 📄 README.md               # Bu doküman
```

## 📈 Çalışma Prensibi

1. **Veri Girişi**: Radar sensöründen (x,y,z,signal_strength) verileri
2. **Tespit**: Sinyal gücü > eşik olan noktalar hedef olarak işaretlenir
3. **Doğrulama**: Hız ve konum bilgilerini kontrol et
4. **Tehdit Analizi**: Hedef hızına ve güvenilirliğine göre seviye belirle
5. **Sıralama**: Tehdit seviyesine göre hedefleri önceliklendir
6. **Çıktı**: Tüm hedefleri tablo formatında göster

## 🎮 Kullanım Örneği

**Program çıktısı:**
```
=== Radar Target Detection System ===

Generated 15 radar data points

Detection complete!
Targets detected: 12
Total targets in system: 12

=== Detected Targets ===
 ID |        X |        Y |        Z | Velocity | Confidence |  Threat | Description
----------------------------------------------------------------------
   1 |    45.32 |   -23.45 |    12.89 |     5.23 |       0.89 |    High | Radar detected target
   2 |    12.67 |    34.91 |   -45.23 |     2.15 |       0.67 |  Medium | Radar detected target
   3 |   -67.89 |    23.45 |     8.91 |     1.23 |       0.45 |     Low | Radar detected target
```

## 🔧 Ayarlar

**Varsayılan Parametreler:**
- Gürültü Eşiği: 0.3 (30% sinyal gücü)
- Maksimum Hedef: 500 (pre-allocation)

**Özelleştirme:**
```cpp
TargetDetector detector(0.25);  // Düşük gürültü eşiği
detector.setNoiseThreshold(0.4); // Çalışma zamanı ayarı
```

---

⚡ **Modern savunma teknolojileri için basit ve etkili çözüm**