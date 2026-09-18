cmake_minimum_required(VERSION 3.22.1)
project(minecraft_android)

add_library(minecraft_android SHARED
    native_bridge.cpp
    android_input.cpp
)

target_compile_features(minecraft_android PRIVATE cxx_std_17)

target_include_directories(minecraft_android PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

find_library(log-lib log)
find_library(gles-lib GLESv3)

target_link_libraries(minecraft_android
    ${log-lib}
    ${gles-lib}
)
