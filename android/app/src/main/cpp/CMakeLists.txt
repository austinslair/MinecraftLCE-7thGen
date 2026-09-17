cmake_minimum_required(VERSION 3.22.1)
project(minecraft_android)

add_library(minecraft_android SHARED native_bridge.cpp)

target_compile_features(minecraft_android PRIVATE cxx_std_17)
find_library(log-lib log)
target_link_libraries(minecraft_android ${log-lib})
