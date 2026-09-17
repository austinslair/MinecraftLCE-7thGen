#pragma once

namespace android_input {

struct TouchEvent {
    int action = 0;
    int pointer_id = 0;
    float x = 0.0f;
    float y = 0.0f;
};

void push_touch(int action, int pointer_id, float x, float y);
TouchEvent last_touch();

} // namespace android_input
