CXX = g++
CXXFLAGS = -std=c++20 -O2 -Wall -Wextra -Iinclude
TARGET = radar_detection
SOURCES = src/TargetDetector.cpp src/MonitorInterface.cpp src/main.cpp

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)

.PHONY: all clean