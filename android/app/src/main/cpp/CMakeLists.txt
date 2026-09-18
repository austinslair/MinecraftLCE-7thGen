cmake_minimum_required(VERSION 3.22.1)
project(minecraft_android)

set(REPO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../../../..)

add_library(minecraft_android SHARED
    native_bridge.cpp
    android_input.cpp
    AndroidStubs.cpp
    ${REPO_ROOT}/Minecraft.World/AABB.cpp
    ${REPO_ROOT}/Minecraft.World/Vec3.cpp
)

target_compile_features(minecraft_android PRIVATE cxx_std_17)

target_compile_options(minecraft_android PRIVATE
    -Wno-delete-non-virtual-dtor
    -Wno-mismatched-new-delete
)

target_include_directories(minecraft_android PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${REPO_ROOT}/Minecraft.World
    ${REPO_ROOT}/Minecraft.World/x64headers
    ${REPO_ROOT}/Minecraft.Client
)

find_library(log-lib log)
find_library(gles-lib GLESv3)

target_link_libraries(minecraft_android
    ${log-lib}
    ${gles-lib}
)
