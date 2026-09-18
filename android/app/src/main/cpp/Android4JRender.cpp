#include "4J_Render.h"

#include <android/log.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <vector>

namespace
{
constexpr char kTag[] = "MinecraftLCE/4JRender";
constexpr int kVertexStride = 32;

struct MatrixStack
{
    std::vector<std::array<float, 16>> values;
};

MatrixStack g_modelView;
MatrixStack g_projection;
MatrixStack g_texture;
int g_matrixMode = GL_MODELVIEW;

int g_width = 1;
int g_height = 1;
bool g_suspended = false;
bool g_initialised = false;

GLuint g_program = 0;
GLuint g_vbo = 0;
GLuint g_vao = 0;
GLuint g_whiteTexture = 0;
GLuint g_boundTexture = 0;

GLint g_uMvp = -1;
GLint g_uColour = -1;
GLint g_uTexture = -1;
GLint g_uAlphaEnabled = -1;
GLint g_uAlphaRef = -1;
GLint g_uAlphaFunc = -1;

float g_colour[4] = {1.0f, 1.0f, 1.0f, 1.0f};
bool g_alphaTestEnabled = false;
float g_alphaRef = 0.0f;
GLenum g_alphaFunc = GL_ALWAYS;
int g_textureLevels = 1;
int g_nextCommandBuffer = 1;

void identity(float *m)
{
    std::memset(m, 0, sizeof(float) * 16);
    m[0] = 1.0f;
    m[5] = 1.0f;
    m[10] = 1.0f;
    m[15] = 1.0f;
}

std::array<float, 16> identityMatrix()
{
    std::array<float, 16> out{};
    identity(out.data());
    return out;
}

void multiply(float *out, const float *a, const float *b)
{
    float result[16];
    for (int column = 0; column < 4; ++column)
    {
        for (int row = 0; row < 4; ++row)
        {
            result[column * 4 + row] =
                a[0 * 4 + row] * b[column * 4 + 0] +
                a[1 * 4 + row] * b[column * 4 + 1] +
                a[2 * 4 + row] * b[column * 4 + 2] +
                a[3 * 4 + row] * b[column * 4 + 3];
        }
    }
    std::memcpy(out, result, sizeof(result));
}

MatrixStack &currentStack()
{
    switch (g_matrixMode)
    {
    case GL_PROJECTION:
        return g_projection;
    case GL_TEXTURE:
        return g_texture;
    case GL_MODELVIEW:
    default:
        return g_modelView;
    }
}

std::array<float, 16> &currentMatrix()
{
    MatrixStack &stack = currentStack();
    if (stack.values.empty())
        stack.values.push_back(identityMatrix());
    return stack.values.back();
}

void rightMultiplyCurrent(const float *rhs)
{
    auto &current = currentMatrix();
    float result[16];
    multiply(result, current.data(), rhs);
    std::memcpy(current.data(), result, sizeof(result));
}

GLuint compileShader(GLenum type, const char *source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint ok = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[1024] = {};
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        __android_log_print(ANDROID_LOG_ERROR, kTag, "shader compile failed: %s", log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

bool createProgram()
{
    static constexpr const char *kVertexShader = R"GLSL(#version 300 es
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_uv;
layout(location = 2) in vec4 a_colour;
uniform mat4 u_mvp;
uniform vec4 u_colour;
out vec2 v_uv;
out vec4 v_colour;
void main() {
    gl_Position = u_mvp * vec4(a_position, 1.0);
    v_uv = a_uv;
    v_colour = a_colour * u_colour;
}
)GLSL";

    static constexpr const char *kFragmentShader = R"GLSL(#version 300 es
precision mediump float;
in vec2 v_uv;
in vec4 v_colour;
uniform sampler2D u_texture;
uniform int u_alphaEnabled;
uniform float u_alphaRef;
uniform int u_alphaFunc;
out vec4 fragColor;
void main() {
    vec4 colour = texture(u_texture, v_uv) * v_colour;
    if (u_alphaEnabled != 0) {
        bool pass = true;
        if (u_alphaFunc == 1) pass = colour.a > u_alphaRef;
        else if (u_alphaFunc == 2) pass = colour.a >= u_alphaRef;
        else if (u_alphaFunc == 3) pass = colour.a == u_alphaRef;
        else if (u_alphaFunc == 4) pass = colour.a < u_alphaRef;
        else if (u_alphaFunc == 5) pass = colour.a <= u_alphaRef;
        if (!pass) discard;
    }
    fragColor = colour;
}
)GLSL";

    GLuint vertex = compileShader(GL_VERTEX_SHADER, kVertexShader);
    GLuint fragment = compileShader(GL_FRAGMENT_SHADER, kFragmentShader);
    if (!vertex || !fragment)
    {
        if (vertex) glDeleteShader(vertex);
        if (fragment) glDeleteShader(fragment);
        return false;
    }

    g_program = glCreateProgram();
    glAttachShader(g_program, vertex);
    glAttachShader(g_program, fragment);
    glLinkProgram(g_program);
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    GLint ok = GL_FALSE;
    glGetProgramiv(g_program, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        char log[1024] = {};
        glGetProgramInfoLog(g_program, sizeof(log), nullptr, log);
        __android_log_print(ANDROID_LOG_ERROR, kTag, "program link failed: %s", log);
        glDeleteProgram(g_program);
        g_program = 0;
        return false;
    }

    g_uMvp = glGetUniformLocation(g_program, "u_mvp");
    g_uColour = glGetUniformLocation(g_program, "u_colour");
    g_uTexture = glGetUniformLocation(g_program, "u_texture");
    g_uAlphaEnabled = glGetUniformLocation(g_program, "u_alphaEnabled");
    g_uAlphaRef = glGetUniformLocation(g_program, "u_alphaRef");
    g_uAlphaFunc = glGetUniformLocation(g_program, "u_alphaFunc");

    glGenVertexArrays(1, &g_vao);
    glGenBuffers(1, &g_vbo);

    glGenTextures(1, &g_whiteTexture);
    glBindTexture(GL_TEXTURE_2D, g_whiteTexture);
    const uint32_t white = 0xffffffffu;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &white);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    g_boundTexture = g_whiteTexture;
    return true;
}

void ensureMatrices()
{
    if (g_modelView.values.empty()) g_modelView.values.push_back(identityMatrix());
    if (g_projection.values.empty()) g_projection.values.push_back(identityMatrix());
    if (g_texture.values.empty()) g_texture.values.push_back(identityMatrix());
}

int alphaFunctionCode(GLenum func)
{
    switch (func)
    {
    case GL_GREATER: return 1;
    case GL_GEQUAL: return 2;
    case GL_EQUAL: return 3;
    case GL_LESS: return 4;
    case GL_LEQUAL: return 5;
    default: return 0;
    }
}

void uploadCommonState()
{
    float mvp[16];
    multiply(mvp, g_projection.values.back().data(), g_modelView.values.back().data());

    glUseProgram(g_program);
    glUniformMatrix4fv(g_uMvp, 1, GL_FALSE, mvp);
    glUniform4fv(g_uColour, 1, g_colour);
    glUniform1i(g_uTexture, 0);
    glUniform1i(g_uAlphaEnabled, g_alphaTestEnabled ? 1 : 0);
    glUniform1f(g_uAlphaRef, g_alphaRef);
    glUniform1i(g_uAlphaFunc, alphaFunctionCode(g_alphaFunc));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_boundTexture ? g_boundTexture : g_whiteTexture);
}

void configureStandardVertexLayout()
{
    glBindVertexArray(g_vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, kVertexStride, reinterpret_cast<void *>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, kVertexStride, reinterpret_cast<void *>(12));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, kVertexStride, reinterpret_cast<void *>(20));
}

std::vector<uint8_t> expandQuads(const void *data, int count)
{
    const auto *bytes = static_cast<const uint8_t *>(data);
    const int quadCount = count / 4;
    std::vector<uint8_t> triangles(static_cast<size_t>(quadCount) * 6u * kVertexStride);
    static constexpr int indices[6] = {0, 1, 2, 0, 2, 3};

    uint8_t *dst = triangles.data();
    for (int q = 0; q < quadCount; ++q)
    {
        const uint8_t *quad = bytes + static_cast<size_t>(q) * 4u * kVertexStride;
        for (int index : indices)
        {
            std::memcpy(dst, quad + static_cast<size_t>(index) * kVertexStride, kVertexStride);
            dst += kVertexStride;
        }
    }
    return triangles;
}
} // namespace

C4JRender RenderManager;

void C4JRender::Tick() {}
void C4JRender::UpdateGamma(unsigned short) {}

void C4JRender::MatrixMode(int type)
{
    g_matrixMode = type;
    ensureMatrices();
}

void C4JRender::MatrixSetIdentity()
{
    identity(currentMatrix().data());
}

void C4JRender::MatrixTranslate(float x, float y, float z)
{
    float m[16];
    identity(m);
    m[12] = x;
    m[13] = y;
    m[14] = z;
    rightMultiplyCurrent(m);
}

void C4JRender::MatrixRotate(float angle, float x, float y, float z)
{
    const float length = std::sqrt(x * x + y * y + z * z);
    if (length <= 0.000001f) return;
    x /= length;
    y /= length;
    z /= length;

    const float radians = angle * 3.14159265358979323846f / 180.0f;
    const float c = std::cos(radians);
    const float s = std::sin(radians);
    const float t = 1.0f - c;

    float m[16] = {
        t*x*x + c,     t*x*y + s*z,   t*x*z - s*y,   0.0f,
        t*x*y - s*z,   t*y*y + c,     t*y*z + s*x,   0.0f,
        t*x*z + s*y,   t*y*z - s*x,   t*z*z + c,     0.0f,
        0.0f,           0.0f,           0.0f,           1.0f
    };
    rightMultiplyCurrent(m);
}

void C4JRender::MatrixScale(float x, float y, float z)
{
    float m[16];
    identity(m);
    m[0] = x;
    m[5] = y;
    m[10] = z;
    rightMultiplyCurrent(m);
}

void C4JRender::MatrixPerspective(float fovy, float aspect, float zNear, float zFar)
{
    if (aspect == 0.0f || zNear == zFar) return;
    const float radians = fovy * 3.14159265358979323846f / 360.0f;
    const float f = 1.0f / std::tan(radians);
    float m[16] = {};
    m[0] = f / aspect;
    m[5] = f;
    m[10] = (zFar + zNear) / (zNear - zFar);
    m[11] = -1.0f;
    m[14] = (2.0f * zFar * zNear) / (zNear - zFar);
    rightMultiplyCurrent(m);
}

void C4JRender::MatrixOrthogonal(float left, float right, float bottom, float top, float zNear, float zFar)
{
    if (right == left || top == bottom || zFar == zNear) return;
    float m[16];
    identity(m);
    m[0] = 2.0f / (right - left);
    m[5] = 2.0f / (top - bottom);
    m[10] = -2.0f / (zFar - zNear);
    m[12] = -(right + left) / (right - left);
    m[13] = -(top + bottom) / (top - bottom);
    m[14] = -(zFar + zNear) / (zFar - zNear);
    rightMultiplyCurrent(m);
}

void C4JRender::MatrixPop()
{
    auto &stack = currentStack().values;
    if (stack.size() > 1) stack.pop_back();
}

void C4JRender::MatrixPush()
{
    auto &stack = currentStack().values;
    if (stack.empty()) stack.push_back(identityMatrix());
    stack.push_back(stack.back());
}

void C4JRender::MatrixMult(float *mat)
{
    if (mat) rightMultiplyCurrent(mat);
}

const float *C4JRender::MatrixGet(int type)
{
    ensureMatrices();
    if (type == GL_PROJECTION || type == GL_PROJECTION_MATRIX)
        return g_projection.values.back().data();
    if (type == GL_TEXTURE)
        return g_texture.values.back().data();
    return g_modelView.values.back().data();
}

void C4JRender::Set_matrixDirty() {}

void C4JRender::Initialise(int width, int height)
{
    g_width = std::max(1, width);
    g_height = std::max(1, height);
    g_modelView.values.assign(1, identityMatrix());
    g_projection.values.assign(1, identityMatrix());
    g_texture.values.assign(1, identityMatrix());
    g_matrixMode = GL_MODELVIEW;
    g_initialised = createProgram();
    glViewport(0, 0, g_width, g_height);
}

void C4JRender::InitialiseContext() {}

void C4JRender::Resize(int width, int height)
{
    g_width = std::max(1, width);
    g_height = std::max(1, height);
    glViewport(0, 0, g_width, g_height);
}

void C4JRender::StartFrame() {}
void C4JRender::DoScreenGrabOnNextPresent() {}
void C4JRender::Present() {}

void C4JRender::Clear(int flags, void *)
{
    GLbitfield mask = 0;
    if (flags & CLEAR_COLOUR_FLAG) mask |= GL_COLOR_BUFFER_BIT;
    if (flags & CLEAR_DEPTH_FLAG) mask |= GL_DEPTH_BUFFER_BIT;
    glClear(mask);
}

void C4JRender::SetClearColour(const float colourRGBA[4])
{
    if (colourRGBA) glClearColor(colourRGBA[0], colourRGBA[1], colourRGBA[2], colourRGBA[3]);
}

bool C4JRender::IsWidescreen()
{
    return static_cast<float>(g_width) / static_cast<float>(std::max(1, g_height)) > 1.5f;
}

bool C4JRender::IsHiDef() { return g_width >= 1280 || g_height >= 720; }
void C4JRender::CaptureThumbnail(ImageFileBuffer *) {}
void C4JRender::CaptureScreen(ImageFileBuffer *, XSOCIAL_PREVIEWIMAGE *) {}
void C4JRender::BeginConditionalSurvey(int) {}
void C4JRender::EndConditionalSurvey() {}
void C4JRender::BeginConditionalRendering(int) {}
void C4JRender::EndConditionalRendering() {}

void C4JRender::DrawVertices(ePrimitiveType primitiveType, int count, void *dataIn,
                             eVertexType vType, ePixelShaderType)
{
    if (!g_initialised || !dataIn || count <= 0) return;
    if (vType == VERTEX_TYPE_COMPRESSED)
    {
        // Compact console vertices need a dedicated unpacking shader. Keep the
        // path explicit instead of interpreting their packed layout incorrectly.
        return;
    }

    ensureMatrices();
    uploadCommonState();
    configureStandardVertexLayout();

    GLenum drawMode = static_cast<GLenum>(primitiveType);
    const void *uploadData = dataIn;
    int drawCount = count;
    std::vector<uint8_t> quadTriangles;

    if (primitiveType == PRIMITIVE_TYPE_QUAD_LIST)
    {
        quadTriangles = expandQuads(dataIn, count);
        uploadData = quadTriangles.data();
        drawCount = (count / 4) * 6;
        drawMode = GL_TRIANGLES;
    }

    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(drawCount) * kVertexStride,
                 uploadData, GL_STREAM_DRAW);
    glDrawArrays(drawMode, 0, drawCount);
    glBindVertexArray(0);
}

void C4JRender::DrawVertexBuffer(ePrimitiveType primitiveType, int count, void *buffer,
                                 eVertexType vType, ePixelShaderType psType)
{
    DrawVertices(primitiveType, count, buffer, vType, psType);
}

void C4JRender::CBuffLockStaticCreations() {}
int C4JRender::CBuffCreate(int) { return g_nextCommandBuffer++; }
void C4JRender::CBuffDelete(int, int) {}
void C4JRender::CBuffStart(int, bool) {}
void C4JRender::CBuffClear(int) {}
int C4JRender::CBuffSize(int) { return 0; }
void C4JRender::CBuffEnd() {}
bool C4JRender::CBuffCall(int, bool) { return false; }
void C4JRender::CBuffTick() {}
void C4JRender::CBuffDeferredModeStart() {}
void C4JRender::CBuffDeferredModeEnd() {}

int C4JRender::TextureCreate()
{
    GLuint texture = 0;
    glGenTextures(1, &texture);
    return static_cast<int>(texture);
}

void C4JRender::TextureFree(int idx)
{
    GLuint texture = static_cast<GLuint>(idx);
    if (texture)
    {
        if (g_boundTexture == texture) g_boundTexture = g_whiteTexture;
        glDeleteTextures(1, &texture);
    }
}

void C4JRender::TextureBind(int idx)
{
    g_boundTexture = idx > 0 ? static_cast<GLuint>(idx) : g_whiteTexture;
    glBindTexture(GL_TEXTURE_2D, g_boundTexture);
}

void C4JRender::TextureBindVertex(int idx) { TextureBind(idx); }
void C4JRender::TextureSetTextureLevels(int levels) { g_textureLevels = std::max(1, levels); }
int C4JRender::TextureGetTextureLevels() { return g_textureLevels; }

void C4JRender::TextureData(int width, int height, void *data, int level, eTextureFormat)
{
    glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

void C4JRender::TextureDataUpdate(int xoffset, int yoffset, int width, int height, void *data, int level)
{
    glTexSubImage2D(GL_TEXTURE_2D, level, xoffset, yoffset, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

void C4JRender::TextureSetParam(int param, int value)
{
    glTexParameteri(GL_TEXTURE_2D, static_cast<GLenum>(param), value);
}

void C4JRender::TextureDynamicUpdateStart() {}
void C4JRender::TextureDynamicUpdateEnd() {}
HRESULT C4JRender::LoadTextureData(const char *, D3DXIMAGE_INFO *, int **) { return E_FAIL; }
HRESULT C4JRender::LoadTextureData(BYTE *, DWORD, D3DXIMAGE_INFO *, int **) { return E_FAIL; }
HRESULT C4JRender::SaveTextureData(const char *, D3DXIMAGE_INFO *, int *) { return E_FAIL; }
HRESULT C4JRender::SaveTextureDataToMemory(void *, int, int *, int, int, int *) { return E_FAIL; }
void C4JRender::TextureGetStats() {}
GLuint C4JRender::TextureGetTexture(int idx) { return static_cast<GLuint>(idx); }

void C4JRender::StateSetColour(float r, float g, float b, float a)
{
    g_colour[0] = r;
    g_colour[1] = g;
    g_colour[2] = b;
    g_colour[3] = a;
}

void C4JRender::StateSetDepthMask(bool enable) { glDepthMask(enable ? GL_TRUE : GL_FALSE); }
void C4JRender::StateSetBlendEnable(bool enable) { enable ? glEnable(GL_BLEND) : glDisable(GL_BLEND); }
void C4JRender::StateSetBlendFunc(int src, int dst) { glBlendFunc(static_cast<GLenum>(src), static_cast<GLenum>(dst)); }

void C4JRender::StateSetBlendFactor(unsigned int colour)
{
    const float r = ((colour >> 16) & 0xff) / 255.0f;
    const float g = ((colour >> 8) & 0xff) / 255.0f;
    const float b = (colour & 0xff) / 255.0f;
    const float a = ((colour >> 24) & 0xff) / 255.0f;
    glBlendColor(r, g, b, a);
}

void C4JRender::StateSetAlphaFunc(int func, float param)
{
    g_alphaFunc = static_cast<GLenum>(func);
    g_alphaRef = param;
}

void C4JRender::StateSetDepthFunc(int func) { glDepthFunc(static_cast<GLenum>(func)); }
void C4JRender::StateSetFaceCull(bool enable) { enable ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE); }
void C4JRender::StateSetFaceCullCW(bool enable) { glFrontFace(enable ? GL_CW : GL_CCW); }
void C4JRender::StateSetLineWidth(float width) { glLineWidth(width); }
void C4JRender::StateSetWriteEnable(bool r, bool g, bool b, bool a) { glColorMask(r, g, b, a); }
void C4JRender::StateSetDepthTestEnable(bool enable) { enable ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST); }
void C4JRender::StateSetAlphaTestEnable(bool enable) { g_alphaTestEnabled = enable; }

void C4JRender::StateSetDepthSlopeAndBias(float slope, float bias)
{
    if (slope == 0.0f && bias == 0.0f)
    {
        glDisable(GL_POLYGON_OFFSET_FILL);
    }
    else
    {
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(slope, bias);
    }
}

void C4JRender::StateSetFogEnable(bool) {}
void C4JRender::StateSetFogMode(int) {}
void C4JRender::StateSetFogNearDistance(float) {}
void C4JRender::StateSetFogFarDistance(float) {}
void C4JRender::StateSetFogDensity(float) {}
void C4JRender::StateSetFogColour(float, float, float) {}
void C4JRender::StateSetLightingEnable(bool) {}
void C4JRender::StateSetVertexTextureUV(float, float) {}
void C4JRender::StateSetLightColour(int, float, float, float) {}
void C4JRender::StateSetLightAmbientColour(float, float, float) {}
void C4JRender::StateSetLightDirection(int, float, float, float) {}
void C4JRender::StateSetLightEnable(int, bool) {}

void C4JRender::StateSetViewport(eViewportType viewportType)
{
    int x = 0;
    int y = 0;
    int width = g_width;
    int height = g_height;

    switch (viewportType)
    {
    case VIEWPORT_TYPE_SPLIT_TOP: height /= 2; y = g_height - height; break;
    case VIEWPORT_TYPE_SPLIT_BOTTOM: height /= 2; break;
    case VIEWPORT_TYPE_SPLIT_LEFT: width /= 2; break;
    case VIEWPORT_TYPE_SPLIT_RIGHT: width /= 2; x = g_width - width; break;
    case VIEWPORT_TYPE_QUADRANT_TOP_LEFT: width /= 2; height /= 2; y = g_height - height; break;
    case VIEWPORT_TYPE_QUADRANT_TOP_RIGHT: width /= 2; height /= 2; x = g_width - width; y = g_height - height; break;
    case VIEWPORT_TYPE_QUADRANT_BOTTOM_LEFT: width /= 2; height /= 2; break;
    case VIEWPORT_TYPE_QUADRANT_BOTTOM_RIGHT: width /= 2; height /= 2; x = g_width - width; break;
    case VIEWPORT_TYPE_FULLSCREEN:
    default: break;
    }
    glViewport(x, y, width, height);
}

void C4JRender::StateSetEnableViewportClipPlanes(bool) {}
void C4JRender::StateSetTexGenCol(int, float, float, float, float, bool) {}

void C4JRender::StateSetStencil(int function, uint8_t stencilRef, uint8_t stencilFuncMask, uint8_t stencilWriteMask)
{
    glStencilFunc(static_cast<GLenum>(function), stencilRef, stencilFuncMask);
    glStencilMask(stencilWriteMask);
}

void C4JRender::StateSetForceLOD(int) {}
void C4JRender::BeginEvent(LPCWSTR) {}
void C4JRender::EndEvent() {}
void C4JRender::Suspend() { g_suspended = true; }
bool C4JRender::Suspended() { return g_suspended; }
void C4JRender::Resume() { g_suspended = false; }
