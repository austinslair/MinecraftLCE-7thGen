#include "android_assets.h"

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>

namespace {
constexpr char kTag[] = "MinecraftLCE";
AAssetManager* g_assetManager = nullptr;
}

namespace android_assets {

bool initialize(JNIEnv* env, jobject assetManager) {
    if (env == nullptr || assetManager == nullptr) {
        __android_log_print(ANDROID_LOG_ERROR, kTag, "AssetManager initialization failed: null JNI argument");
        g_assetManager = nullptr;
        return false;
    }

    g_assetManager = AAssetManager_fromJava(env, assetManager);
    if (g_assetManager == nullptr) {
        __android_log_print(ANDROID_LOG_ERROR, kTag, "AssetManager initialization failed: native handle unavailable");
        return false;
    }

    __android_log_print(ANDROID_LOG_INFO, kTag, "Native AssetManager initialized");
    return true;
}

AAssetManager* manager() {
    return g_assetManager;
}

bool is_initialized() {
    return g_assetManager != nullptr;
}

} // namespace android_assets

extern "C" JNIEXPORT void JNICALL
Java_com_austinslair_minecraftlce_NativeBridge_initPlatform(
    JNIEnv* env,
    jclass,
    jobject assetManager) {
    android_assets::initialize(env, assetManager);
}
