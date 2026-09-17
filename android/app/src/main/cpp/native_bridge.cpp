#include <jni.h>
#include <android/log.h>

namespace {
constexpr char kTag[] = "MinecraftLCE";
}

extern "C" JNIEXPORT void JNICALL
Java_com_austinslair_minecraftlce_NativeBridge_touch(
    JNIEnv*, jclass, jint action, jint pointer_id, jfloat x, jfloat y) {
    __android_log_print(ANDROID_LOG_DEBUG, kTag,
        "touch action=%d pointer=%d x=%.1f y=%.1f", action, pointer_id, x, y);
}
