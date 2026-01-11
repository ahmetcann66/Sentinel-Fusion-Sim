#!/bin/bash

# Build script for Target Detection System

echo "🚀 Savunma Sanayi Hedef Tespit Sistemi - Build Script"
echo "=================================================="

# Check if build directory exists
if [ ! -d "build" ]; then
    echo "📁 Build dizini oluşturuluyor..."
    mkdir build
fi

cd build

echo "🔧 CMake configuration..."
cmake ..

if [ $? -eq 0 ]; then
    echo "✅ CMake başarılı"
else
    echo "❌ CMake hatası"
    exit 1
fi

echo "🔨 Derleme başlıyor..."
make -j$(nproc)

if [ $? -eq 0 ]; then
    echo "✅ Derleme başarılı!"
    echo "🎯 Çalıştırmak için: ./target_detection"
else
    echo "❌ Derleme hatası"
    exit 1
fi

echo "📋 Build tamamlandı!"