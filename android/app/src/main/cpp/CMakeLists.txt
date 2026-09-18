cmake_minimum_required(VERSION 3.22.1)
project(minecraft_android)

get_filename_component(REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../.." ABSOLUTE)

# Minecraft.Client sources include their platform PCH with a quoted include,
# which normally resolves to Minecraft.Client/stdafx.h. During the Android
# port we remap that one file to a small Android compatibility PCH so the
# original client translation units can be compiled without editing them.
set(ANDROID_CLIENT_COMPAT_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/client_compat/stdafx.h")
set(ANDROID_CLIENT_VFS_OVERLAY "${CMAKE_CURRENT_BINARY_DIR}/android-client-vfs.yaml")
file(GENERATE OUTPUT "${ANDROID_CLIENT_VFS_OVERLAY}" CONTENT
"{\n  \"version\": 0,\n  \"roots\": [\n    {\n      \"type\": \"file\",\n      \"name\": \"${REPO_ROOT}/Minecraft.Client/stdafx.h\",\n      \"external-contents\": \"${ANDROID_CLIENT_COMPAT_HEADER}\"\n    }\n  ]\n}\n")

add_library(minecraft_android SHARED
    native_bridge.cpp
    android_input.cpp
    AndroidStubs.cpp

    # First untouched Minecraft.Client compile probe. More client sources will
    # be added incrementally as their platform dependencies are implemented.
    ${REPO_ROOT}/Minecraft.Client/AllowAllCuller.cpp

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
