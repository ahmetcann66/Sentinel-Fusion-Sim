# 🎯 Sentinel Fusion Sim

**Real-Time Radar Simulation & Threat Detection System powered by C++20**

![Build Status](https://img.shields.io/badge/build-passing-brightgreen)
![Language](https://img.shields.io/badge/C%2B%2B-20-blue)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-lightgrey)

---

## 📋 Executive Summary

Sentinel Fusion Sim is a high-performance simulation tool designed to replicate radar signal processing and threat assessment in real-time. Built strictly with **Modern C++20**, it demonstrates advanced usage of the Standard Template Library (STL), including Ranges, Concepts, and the Spaceship Operator (`<=>`), to process and classify dynamic aerial targets based on kinematics and signal confidence.

This project serves as a proof-of-concept for defense industry algorithms, focusing on clean architecture, type safety, and efficient data processing.

---

## 🏗️ System Architecture

### Core Components

1.  **Target Detection Engine (`TargetDetector`)**
    * Processes raw radar signals (X, Y, Z, Velocity).
    * Calculates 3D Euclidean distance and confidence levels.
    * Classifies threats using a multi-factor logic matrix.
    * Sorts targets by priority using C++20 comparisons.

2.  **Real-Time Simulation Loop (`main`)**
    * Generates stochastic (random) mock data using `std::mt19937`.
    * Simulates radar sweep cycles with adjustable frequencies.
    * Provides a dynamic CLI Dashboard for visualization.

---

## 🧮 Mathematical Foundations

The system employs standard kinematic models for threat evaluation:

**1. Euclidean Distance:**
$$d = \sqrt{x^2 + y^2 + z^2}$$

**2. Threat Assessment Logic:**
The system categorizes targets based on a combination of **Proximity** and **Velocity**:
* **CRITICAL:** Distance < 500m AND Velocity > 100 m/s
* **HIGH:** Distance < 1000m
* **MEDIUM:** Velocity > 50 m/s
* **LOW:** Passive/Distant targets

---

## 🛠️ Technical Implementation

### Key C++20 Features Used

* **`std::ranges` & `std::views`**: Used for readable and efficient data filtering and sorting.
* **Three-Way Comparison (`operator<=>`)**: Simplifies the logic for sorting targets by "Threat Level" first, then "Confidence".
* **`std::optional`**: Safely handles invalid or noisy signals without using exceptions or null pointers.
* **`std::chrono`**: Precise timing for simulation loops.

### Algorithmic Complexity

* **Sorting**: $O(N \log N)$ - Optimised using standard library introspection sort.
* **Filtering**: $O(N)$ - Single pass noise reduction.
* **Space Complexity**: $O(N)$ - Linear memory usage relative to target count.

---

## 📦 Installation and Build

### Prerequisites
* **Compiler**: GCC 10+, Clang 12+, or MSVC 2019+ (Must support C++20)
* **Build System**: CMake 3.20+

### Build Instructions (Windows/Linux)

```bash
# 1. Clone the repository
git clone [https://github.com/ahmetcann66/Sentinel-Fusion-Sim.git](https://github.com/ahmetcann66/Sentinel-Fusion-Sim.git)
cd Sentinel-Fusion-Sim/TargetDetectionSystem

# 2. Configure the project
cmake -S . -B build

# 3. Build the executable (Debug mode recommended for logs)
cmake --build build --config Debug

# 4. Run the Simulation
./build/Debug/radar_detection.exe

========================================
      SENTINEL RADAR - LIVE FEED        
========================================
Scan Cycle: 42
Raw Signals Received: 15
Active Targets Locked: 12
----------------------------------------
ID: 8  | Threat: CRITICAL | Conf: 0.92 | Vel: 250.0 m/s
ID: 3  | Threat: HIGH     | Conf: 0.85 | Vel: 120.5 m/s
...
[Scanning for new threats...]

🗺️ Roadmap & Future Optimizations
While the current version focuses on architectural correctness, the following performance upgrades are planned:

[ ] Sensor Fusion: Integrate Kalman Filter for predictive tracking.

[ ] Visualization: Connect to Unity/Unreal Engine via DLL for 3D rendering.

[ ] Optimization: Implement SIMD (AVX2) for vector distance calculations.

[ ] Concurrency: Use std::jthread for parallel signal processing.

📄 License
This project is open-source and available under the MIT License.

Developed by Ahmet Can Bozkurt
