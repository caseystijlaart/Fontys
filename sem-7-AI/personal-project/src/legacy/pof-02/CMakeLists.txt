cmake_minimum_required(VERSION 3.16)
project(pof02_demo CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(
    pof02_demo
    src/main.cpp
    src/SoilMoistureSensor.cpp
    src/TempHumiditySensor.cpp
    src/LightSensor.cpp
    src/Classification.cpp
    src/Stress.cpp
    src/SensorHistory.cpp
    src/FeatureEngineering.cpp
    src/MLLayer.cpp
    src/RecommendationEngine.cpp
    src/MonitoringSystem.cpp)

target_include_directories(pof02_demo PRIVATE include)
