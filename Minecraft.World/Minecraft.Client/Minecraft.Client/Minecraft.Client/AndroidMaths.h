#pragma once

#include <cmath>
#include <cstring>

struct XMVECTOR {
    union {
        float m128_f32[4];
        struct { float x, y, z, w; };
    };
};

struct XMMATRIX {
    union {
        float r[4][4];
        float m[16];
        XMVECTOR row[4];
    };
};

inline XMVECTOR XMVectorSet(float x, float y, float z, float w) {
    XMVECTOR v;
    v.m128_f32[0] = x; v.m128_f32[1] = y; v.m128_f32[2] = z; v.m128_f32[3] = w;
    return v;
}

inline XMVECTOR XMVectorZero() {
    return XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
}

inline XMVECTOR XMVector3Normalize(XMVECTOR v) {
    float len = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    if (len > 1e-5f) {
        float inv = 1.0f / len;
        return XMVectorSet(v.x * inv, v.y * inv, v.z * inv, 0.0f);
    }
    return v;
}

inline XMVECTOR XMVector3Cross(XMVECTOR a, XMVECTOR b) {
    return XMVectorSet(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x,
        0.0f
    );
}

inline XMMATRIX XMMatrixIdentity() {
    XMMATRIX m;
    std::memset(&m, 0, sizeof(XMMATRIX));
    m.r[0][0] = 1.0f; m.r[1][1] = 1.0f; m.r[2][2] = 1.0f; m.r[3][3] = 1.0f;
    return m;
}
