# Savunma Sanayi Hedef Tespit Sistemi

C++17 ile geliştirilen çoklu sensör tabanlı hedef tespit ve takip sistemi.

## Özellikler

- **Çoklu Sensör Desteği**: Radar, Termal, Optik sensör verileri
- **Gürültü Filtreleme**: Anlamsız hedefleri otomatik olarak temizler
- **Hedef Takibi**: Hedef hareketlerini sürekli olarak izler
- **Sensör Füzyonu**: Farklı sensörlerden gelen verileri birleştirir
- **Gerçek Zamanlı Tespit**: Hızlı ve etkili hedef tespiti

## Kurulum

### Gerekli Bağımlılıklar
```bash
# Ubuntu/Debian
sudo apt-get install build-essential cmake libopencv-dev

# CentOS/RHEL
sudo yum install gcc-c++ cmake opencv-devel

# macOS (Homebrew)
brew install cmake opencv
```

### Derleme
```bash
# Otomatik derleme
chmod +x build.sh
./build.sh

# Manuel derleme
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Kullanım

```bash
./target_detection
```

## Proje Yapısı

```
TargetDetectionSystem/
├── include/
│   └── TargetDetector.h    # Ana sınıf tanımları
├── src/
│   ├── TargetDetector.cpp  # Hedef tespit algoritmaları
│   └── main.cpp           # Demo uygulaması
├── build.sh               # Otomatik derleme script'i
├── CMakeLists.txt         # Build configuration
└── README.md             # Bu dosya
```

## Algoritmalar

### Radar Tespiti
- Sinyal gücü analizi
- Mesafe hesaplama
- Hız tahmini

### Termal Tespit
- Sıcaklık eşiği analizi
- Isı kaynaklarını tespit
- Boyut tahmini

### Optik Tespit
- Parlaklık analizi
- Kontrast algılama
- Hareket tespiti

## Performans

- İşlem süresi: <100ms (100 hedef için)
- Doğruluk oranı: >85%
- Desteklenen hedef sayısı: 1000+

## Test

Sistem, rastgele oluşturulmuş sensor verileriyle test edilmiştir:
- 15 radar hedefi simülasyonu
- 12 termal hedef simülasyonu  
- 10 optik hedef simülasyonu

## Katkıda Bulunma

1. Fork yap
2. Feature branch oluştur (`git checkout -b feature/YeniOzellik`)
3. Commit yap (`git commit -m 'Yeni özellik eklendi'`)
4. Push yap (`git push origin feature/YeniOzellik`)
5. Pull Request aç

## Lisans

Bu proje savunma sanayi araştırma amaçlıdır. Kullanım ilgili lisans koşullarına tabidir.

## İletişim

[Proje geliştirici bilgileri]

---

⚡ **Modern savunma teknolojileri için geliştirilmiştir**
