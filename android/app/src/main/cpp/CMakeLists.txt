cmake_minimum_required(VERSION 3.22.1)
project(minecraft_android)

get_filename_component(REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../.." ABSOLUTE)

# Original client sources use platform-heavy shared headers. Remap only the
# incompatible headers to Android compatibility equivalents so the original
# translation units can remain untouched while the port is brought up.
set(ANDROID_CLIENT_COMPAT_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/client_compat/stdafx.h")
set(ANDROID_SYSTEM_COMPAT_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/client_compat/System.h")
set(ANDROID_CLIENT_VFS_OVERLAY "${CMAKE_CURRENT_BINARY_DIR}/android-client-vfs.yaml")
file(GENERATE OUTPUT "${ANDROID_CLIENT_VFS_OVERLAY}" CONTENT
"{\n  \"version\": 0,\n  \"roots\": [\n    {\n      \"type\": \"file\",\n      \"name\": \"${REPO_ROOT}/Minecraft.Client/stdafx.h\",\n      \"external-contents\": \"${ANDROID_CLIENT_COMPAT_HEADER}\"\n    },\n    {\n      \"type\": \"file\",\n      \"name\": \"${REPO_ROOT}/Minecraft.World/System.h\",\n      \"external-contents\": \"${ANDROID_SYSTEM_COMPAT_HEADER}\"\n    }\n  ]\n}\n")

add_library(minecraft_android SHARED
    native_bridge.cpp
    android_input.cpp
    AndroidStubs.cpp
    AndroidSystem.cpp

    # Untouched Minecraft.Client sources. Grow this list incrementally as
    # Android platform dependencies are implemented.
    ${REPO_ROOT}/Minecraft.Client/AllowAllCuller.cpp
    ${REPO_ROOT}/Minecraft.Client/Timer.cpp

    ${REPO_ROOT}/Minecraft.World/AABB.cpp
    ${REPO_ROOT}/Minecraft.World/Vec3.cpp
    ${REPO_ROOT}/Minecraft.World/IntCache.cpp
    ${REPO_ROOT}/Minecraft.World/Mth.cpp
    ${REPO_ROOT}/Minecraft.World/Random.cpp
    ${REPO_ROOT}/Minecraft.World/ImprovedNoise.cpp
    ${REPO_ROOT}/Minecraft.World/PerlinNoise.cpp
    ${REPO_ROOT}/Minecraft.World/SimplexNoise.cpp
    ${REPO_ROOT}/Minecraft.World/Facing.cpp
)

target_compile_features(minecraft_android PRIVATE cxx_std_17)
target_compile_options(minecraft_android PRIVATE
    -Wno-delete-non-virtual-dtor
    -Wno-mismatched-new-delete
    -fms-extensions
    -ivfsoverlay
    ${ANDROID_CLIENT_VFS_OVERLAY}
)

target_include_directories(minecraft_android PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/client_compat
    ${REPO_ROOT}/Minecraft.World
    ${REPO_ROOT}/Minecraft.World/x64headers
    ${REPO_ROOT}/Minecraft.Client
)

find_library(log-lib log)
find_library(gles-lib GLESv3)
find_library(android-lib android)

target_link_libraries(minecraft_android
    ${log-lib}
    ${gles-lib}
    ${android-lib}
)
