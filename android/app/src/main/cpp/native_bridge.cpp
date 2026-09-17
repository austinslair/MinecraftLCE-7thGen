#include <jni.h>
#include <android/log.h>
#include <GLES3/gl3.h>

#include "android_input.h"

namespace {
constexpr char kTag[] = "MinecraftLCE";
int g_width = 1;
int g_height = 1;
}

extern "C" JNIEXPORT void JNICALL
Java_com_austinslair_minecraftlce_NativeBridge_touch(
    JNIEnv*, jclass, jint action, jint pointer_id, jfloat x, jfloat y) {
    android_input::push_touch(action, pointer_id, x, y);
    __android_log_print(ANDROID_LOG_DEBUG, kTag,
        "touch action=%d pointer=%d x=%.1f y=%.1f", action, pointer_id, x, y);
}

extern "C" JNIEXPORT void JNICALL
Java_com_austinslair_minecraftlce_NativeBridge_surfaceCreated(JNIEnv*, jclass) {
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.04f, 0.04f, 0.04f, 1.0f);
}

extern "C" JNIEXPORT void JNICALL
Java_com_austinslair_minecraftlce_NativeBridge_surfaceChanged(
    JNIEnv*, jclass, jint width, jint height) {
    g_width = width > 0 ? width : 1;
    g_height = height > 0 ? height : 1;
    glViewport(0, 0, g_width, g_height);
}

extern "C" JNIEXPORT void JNICALL
Java_com_austinslair_minecraftlce_NativeBridge_renderFrame(JNIEnv*, jclass) {
    // Temporary renderer hook. The LCE renderer will replace this clear pass
    // once its platform-independent renderer is wired into the Android target.
    glViewport(0, 0, g_width, g_height);
    glClear(GL_COLOR_BUFFER_BIT);
}
