#include <jni.h>
#include <android/log.h>
#include <GLES3/gl3.h>
#include <cmath>
#include <cstring>
#include <vector>
#include "android_input.h"

namespace {
constexpr char kTag[] = "MinecraftLCE";

int g_width = 1;
int g_height = 1;

GLuint g_program = 0;
GLint g_uMvp = -1;
GLint g_uTexture = -1;

GLuint g_vao = 0;
GLuint g_vbo = 0;
GLuint g_texture = 0;

float g_rotX = 25.0f;
float g_rotY = -45.0f;
float g_lastTouchX = 0.0f;
float g_lastTouchY = 0.0f;
bool g_isDragging = false;
float g_animTime = 0.0f;

struct Vertex {
    float x, y, z;
    float u, v;
    float light;
};

// Shaders
const char* kVertexShader = 
    "#version 300 es\n"
    "layout(location = 0) in vec3 a_position;\n"
    "layout(location = 1) in vec2 a_texCoord;\n"
    "layout(location = 2) in float a_light;\n"
    "uniform mat4 u_mvp;\n"
    "out vec2 v_texCoord;\n"
    "out float v_light;\n"
    "void main() {\n"
    "    gl_Position = u_mvp * vec4(a_position, 1.0);\n"
    "    v_texCoord = a_texCoord;\n"
    "    v_light = a_light;\n"
    "}\n";

const char* kFragmentShader = 
    "#version 300 es\n"
    "precision mediump float;\n"
    "in vec2 v_texCoord;\n"
    "in float v_light;\n"
    "uniform sampler2D u_texture;\n"
    "out vec4 fragColor;\n"
    "void main() {\n"
    "    vec4 tex = texture(u_texture, v_texCoord);\n"
    "    fragColor = vec4(tex.rgb * v_light, tex.a);\n"
    "}\n";

GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        char info[512];
        glGetShaderInfoLog(shader, sizeof(info), nullptr, info);
        __android_log_print(ANDROID_LOG_ERROR, kTag, "Shader compile error: %s", info);
    }
    return shader;
}

void matrixIdentity(float* m) {
    std::memset(m, 0, 16 * sizeof(float));
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

void matrixMultiply(float* out, const float* a, const float* b) {
    float res[16];
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            res[j * 4 + i] = 0.0f;
            for (int k = 0; k < 4; ++k) {
                res[j * 4 + i] += a[k * 4 + i] * b[j * 4 + k];
            }
        }
    }
    std::memcpy(out, res, 16 * sizeof(float));
}

void matrixPerspective(float* m, float fovRad, float aspect, float nearZ, float farZ) {
    float f = 1.0f / std::tan(fovRad / 2.0f);
    std::memset(m, 0, 16 * sizeof(float));
    m[0] = f / aspect;
    m[5] = f;
    m[10] = (farZ + nearZ) / (nearZ - farZ);
    m[11] = -1.0f;
    m[14] = (2.0f * farZ * nearZ) / (nearZ - farZ);
}

void matrixRotateX(float* m, float angleDeg) {
    float rad = angleDeg * 3.14159265f / 180.0f;
    float c = std::cos(rad);
    float s = std::sin(rad);
    matrixIdentity(m);
    m[5] = c;
    m[6] = s;
    m[9] = -s;
    m[10] = c;
}

void matrixRotateY(float* m, float angleDeg) {
    float rad = angleDeg * 3.14159265f / 180.0f;
    float c = std::cos(rad);
    float s = std::sin(rad);
    matrixIdentity(m);
    m[0] = c;
    m[2] = -s;
    m[8] = s;
    m[10] = c;
}

void matrixTranslate(float* m, float x, float y, float z) {
    matrixIdentity(m);
    m[12] = x;
    m[13] = y;
    m[14] = z;
}

void buildMinecraftTextureAtlas() {
    constexpr int W = 48; // 3 tiles of 16x16: 0=Top, 1=Side, 2=Bottom
    constexpr int H = 16;
    unsigned char pixels[W * H * 4];

    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            int tile = x / 16;
            int tx = x % 16;
            int ty = y;
            int idx = (y * W + x) * 4;

            int noise = ((tx * 37 + ty * 19 + tile * 53) % 23) - 11;

            if (tile == 0) {
                // Top: Minecraft grass green
                int r = 106 + noise;
                int g = 175 + noise * 2;
                int b = 64 + noise;
                pixels[idx + 0] = (unsigned char)std::clamp(r, 0, 255);
                pixels[idx + 1] = (unsigned char)std::clamp(g, 0, 255);
                pixels[idx + 2] = (unsigned char)std::clamp(b, 0, 255);
                pixels[idx + 3] = 255;
            } else if (tile == 1) {
                // Side: Grass top trim with dirt below
                int drop = 3 + ((tx * 7) % 3);
                if (ty < drop) {
                    int r = 106 + noise;
                    int g = 175 + noise * 2;
                    int b = 64 + noise;
                    pixels[idx + 0] = (unsigned char)std::clamp(r, 0, 255);
                    pixels[idx + 1] = (unsigned char)std::clamp(g, 0, 255);
                    pixels[idx + 2] = (unsigned char)std::clamp(b, 0, 255);
                    pixels[idx + 3] = 255;
                } else {
                    int r = 134 + noise;
                    int g = 96 + noise;
                    int b = 67 + noise;
                    pixels[idx + 0] = (unsigned char)std::clamp(r, 0, 255);
                    pixels[idx + 1] = (unsigned char)std::clamp(g, 0, 255);
                    pixels[idx + 2] = (unsigned char)std::clamp(b, 0, 255);
                    pixels[idx + 3] = 255;
                }
            } else {
                // Bottom: Pure Minecraft dirt
                int r = 134 + noise;
                int g = 96 + noise;
                int b = 67 + noise;
                pixels[idx + 0] = (unsigned char)std::clamp(r, 0, 255);
                pixels[idx + 1] = (unsigned char)std::clamp(g, 0, 255);
                pixels[idx + 2] = (unsigned char)std::clamp(b, 0, 255);
                pixels[idx + 3] = 255;
            }
        }
    }

    glGenTextures(1, &g_texture);
    glBindTexture(GL_TEXTURE_2D, g_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, W, H, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}

void initMesh() {
    // Atlas UV coordinates
    // Tile 0: [0/3, 1/3] = [0.0, 0.3333]
    // Tile 1: [1/3, 2/3] = [0.3333, 0.6666]
    // Tile 2: [2/3, 3/3] = [0.6666, 1.0]
    constexpr float u0_top = 0.0f, u1_top = 0.3333f;
    constexpr float u0_side = 0.3333f, u1_side = 0.6666f;
    constexpr float u0_bot = 0.6666f, u1_bot = 1.0f;

    const std::vector<Vertex> vertices = {
        // Top Face (Y = +0.5, light = 1.0)
        {-0.5f,  0.5f, -0.5f,  u0_top, 0.0f, 1.0f},
        {-0.5f,  0.5f,  0.5f,  u0_top, 1.0f, 1.0f},
        { 0.5f,  0.5f,  0.5f,  u1_top, 1.0f, 1.0f},
        {-0.5f,  0.5f, -0.5f,  u0_top, 0.0f, 1.0f},
        { 0.5f,  0.5f,  0.5f,  u1_top, 1.0f, 1.0f},
        { 0.5f,  0.5f, -0.5f,  u1_top, 0.0f, 1.0f},

        // Bottom Face (Y = -0.5, light = 0.5)
        {-0.5f, -0.5f, -0.5f,  u0_bot, 0.0f, 0.5f},
        { 0.5f, -0.5f,  0.5f,  u1_bot, 1.0f, 0.5f},
        {-0.5f, -0.5f,  0.5f,  u0_bot, 1.0f, 0.5f},
        {-0.5f, -0.5f, -0.5f,  u0_bot, 0.0f, 0.5f},
        { 0.5f, -0.5f, -0.5f,  u1_bot, 0.0f, 0.5f},
        { 0.5f, -0.5f,  0.5f,  u1_bot, 1.0f, 0.5f},

        // Front Face (Z = +0.5, light = 0.8)
        {-0.5f, -0.5f,  0.5f,  u0_side, 1.0f, 0.8f},
        { 0.5f, -0.5f,  0.5f,  u1_side, 1.0f, 0.8f},
        { 0.5f,  0.5f,  0.5f,  u1_side, 0.0f, 0.8f},
        {-0.5f, -0.5f,  0.5f,  u0_side, 1.0f, 0.8f},
        { 0.5f,  0.5f,  0.5f,  u1_side, 0.0f, 0.8f},
        {-0.5f,  0.5f,  0.5f,  u0_side, 0.0f, 0.8f},

        // Back Face (Z = -0.5, light = 0.8)
        {-0.5f, -0.5f, -0.5f,  u1_side, 1.0f, 0.8f},
        { 0.5f,  0.5f, -0.5f,  u0_side, 0.0f, 0.8f},
        { 0.5f, -0.5f, -0.5f,  u0_side, 1.0f, 0.8f},
        {-0.5f, -0.5f, -0.5f,  u1_side, 1.0f, 0.8f},
        {-0.5f,  0.5f, -0.5f,  u1_side, 0.0f, 0.8f},
        { 0.5f,  0.5f, -0.5f,  u0_side, 0.0f, 0.8f},

        // Left Face (X = -0.5, light = 0.65)
        {-0.5f, -0.5f, -0.5f,  u0_side, 1.0f, 0.65f},
        {-0.5f,  0.5f,  0.5f,  u1_side, 0.0f, 0.65f},
        {-0.5f,  0.5f, -0.5f,  u0_side, 0.0f, 0.65f},
        {-0.5f, -0.5f, -0.5f,  u0_side, 1.0f, 0.65f},
        {-0.5f, -0.5f,  0.5f,  u1_side, 1.0f, 0.65f},
        {-0.5f,  0.5f,  0.5f,  u1_side, 0.0f, 0.65f},

        // Right Face (X = +0.5, light = 0.65)
        { 0.5f, -0.5f, -0.5f,  u1_side, 1.0f, 0.65f},
        { 0.5f,  0.5f, -0.5f,  u1_side, 0.0f, 0.65f},
        { 0.5f,  0.5f,  0.5f,  u0_side, 0.0f, 0.65f},
        { 0.5f, -0.5f, -0.5f,  u1_side, 1.0f, 0.65f},
        { 0.5f,  0.5f,  0.5f,  u0_side, 0.0f, 0.65f},
        { 0.5f, -0.5f,  0.5f,  u0_side, 1.0f, 0.65f},
    };

    glGenVertexArrays(1, &g_vao);
    glBindVertexArray(g_vao);

    glGenBuffers(1, &g_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));

    // TexCoord attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, u));

    // Light attribute
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, light));

    glBindVertexArray(0);
}
} // namespace

extern "C" JNIEXPORT void JNICALL
Java_com_austinslair_minecraftlce_NativeBridge_touch(
    JNIEnv*, jclass, jint action, jint pointer_id, jfloat x, jfloat y) {
    android_input::push_touch(action, pointer_id, x, y);

    if (action == 0) { // ACTION_DOWN
        g_lastTouchX = x;
        g_lastTouchY = y;
        g_isDragging = true;
    } else if (action == 2 && g_isDragging) { // ACTION_MOVE
        float dx = x - g_lastTouchX;
        float dy = y - g_lastTouchY;
        g_rotY += dx * 0.4f;
        g_rotX += dy * 0.4f;
        if (g_rotX > 85.0f) g_rotX = 85.0f;
        if (g_rotX < -85.0f) g_rotX = -85.0f;
        g_lastTouchX = x;
        g_lastTouchY = y;
    } else if (action == 1) { // ACTION_UP
        g_isDragging = false;
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_austinslair_minecraftlce_NativeBridge_surfaceCreated(JNIEnv*, jclass) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Classic Minecraft Sky Fog Blue
    glClearColor(0.48f, 0.65f, 1.0f, 1.0f);

    GLuint vs = compileShader(GL_VERTEX_SHADER, kVertexShader);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, kFragmentShader);

    g_program = glCreateProgram();
    glAttachShader(g_program, vs);
    glAttachShader(g_program, fs);
    glLinkProgram(g_program);

    g_uMvp = glGetUniformLocation(g_program, "u_mvp");
    g_uTexture = glGetUniformLocation(g_program, "u_texture");

    buildMinecraftTextureAtlas();
    initMesh();
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
    glViewport(0, 0, g_width, g_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (g_program == 0 || g_vao == 0) return;

    if (!g_isDragging) {
        g_rotY += 0.5f; // Gentle automatic spin when idle
    }
    g_animTime += 0.02f;
    float bob = std::sin(g_animTime) * 0.08f;

    // MVP Matrices
    float aspect = (float)g_width / (float)g_height;
    float proj[16], view[16], model[16], rotX[16], rotY[16], trans[16], temp[16], mvp[16];

    matrixPerspective(proj, 45.0f * 3.14159265f / 180.0f, aspect, 0.1f, 100.0f);
    matrixTranslate(view, 0.0f, -bob, -2.8f);

    matrixRotateX(rotX, g_rotX);
    matrixRotateY(rotY, g_rotY);
    matrixMultiply(model, rotX, rotY);

    matrixMultiply(temp, view, model);
    matrixMultiply(mvp, proj, temp);

    glUseProgram(g_program);
    glUniformMatrix4fv(g_uMvp, 1, GL_FALSE, mvp);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_texture);
    glUniform1i(g_uTexture, 0);

    glBindVertexArray(g_vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}
