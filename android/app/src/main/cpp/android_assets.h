#pragma once

#include <jni.h>

struct AAssetManager;

namespace android_assets {

// Captures Android's native asset manager for the legacy engine/platform layer.
// The Java AssetManager is owned by Android; this stores only the native handle.
bool initialize(JNIEnv* env, jobject assetManager);

AAssetManager* manager();
bool is_initialized();

} // namespace android_assets
