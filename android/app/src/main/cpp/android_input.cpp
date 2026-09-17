#include "android_input.h"

#include <mutex>

namespace {
std::mutex g_mutex;
android_input::TouchEvent g_last_touch;
}

namespace android_input {

void push_touch(int action, int pointer_id, float x, float y) {
    std::lock_guard<std::mutex> lock(g_mutex);
    g_last_touch = {action, pointer_id, x, y};
}

TouchEvent last_touch() {
    std::lock_guard<std::mutex> lock(g_mutex);
    return g_last_touch;
}

} // namespace android_input
