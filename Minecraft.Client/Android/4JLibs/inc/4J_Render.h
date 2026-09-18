#pragma once

#include <GLES3/gl3.h>
#include <cstdint>
#include <cstdlib>

#include "AndroidTypes.h"

// Android implementation of the 4J rendering interface used by the original
// client. Keep this API close to the console/Windows versions so game code can
// remain platform-agnostic.

class ImageFileBuffer
{
public:
    enum EImageType
    {
        e_typePNG,
        e_typeJPG
    };

    EImageType m_type = e_typePNG;
    void *m_pBuffer = nullptr;
    int m_bufferSize = 0;

    int GetType() const { return m_type; }
    void *GetBufferPointer() { return m_pBuffer; }
    int GetBufferSize() const { return m_bufferSize; }
    void Release()
    {
        free(m_pBuffer);
        m_pBuffer = nullptr;
        m_bufferSize = 0;
    }
    bool Allocated() const { return m_pBuffer != nullptr; }
};

struct D3DXIMAGE_INFO
{
    int Width;
    int Height;
};

struct XSOCIAL_PREVIEWIMAGE
{
    BYTE *pBytes;
    DWORD Pitch;
    DWORD Width;
    DWORD Height;
};
using PXSOCIAL_PREVIEWIMAGE = XSOCIAL_PREVIEWIMAGE *;

class C4JRender
{
public:
    enum eVertexType
    {
        VERTEX_TYPE_PF3_TF2_CB4_NB4_XW1,
        VERTEX_TYPE_COMPRESSED,
        VERTEX_TYPE_PF3_TF2_CB4_NB4_XW1_LIT,
        VERTEX_TYPE_PF3_TF2_CB4_NB4_XW1_TEXGEN,
        VERTEX_TYPE_COUNT
    };

    enum ePixelShaderType
    {
        PIXEL_SHADER_TYPE_STANDARD,
        PIXEL_SHADER_TYPE_PROJECTION,
        PIXEL_SHADER_TYPE_FORCELOD,
        PIXEL_SHADER_COUNT
    };

    // Values intentionally match the OpenGL primitive constants. Original
    // code frequently stores GL_* values and later casts them to this enum.
    enum ePrimitiveType
    {
        PRIMITIVE_TYPE_LINE_LIST = GL_LINES,
        PRIMITIVE_TYPE_LINE_STRIP = GL_LINE_STRIP,
        PRIMITIVE_TYPE_TRIANGLE_LIST = GL_TRIANGLES,
        PRIMITIVE_TYPE_TRIANGLE_STRIP = GL_TRIANGLE_STRIP,
        PRIMITIVE_TYPE_TRIANGLE_FAN = GL_TRIANGLE_FAN,
        PRIMITIVE_TYPE_QUAD_LIST = 0x0007,
        PRIMITIVE_TYPE_COUNT = 0x0008
    };

    enum eViewportType
    {
        VIEWPORT_TYPE_FULLSCREEN,
        VIEWPORT_TYPE_SPLIT_TOP,
        VIEWPORT_TYPE_SPLIT_BOTTOM,
        VIEWPORT_TYPE_SPLIT_LEFT,
        VIEWPORT_TYPE_SPLIT_RIGHT,
        VIEWPORT_TYPE_QUADRANT_TOP_LEFT,
        VIEWPORT_TYPE_QUADRANT_TOP_RIGHT,
        VIEWPORT_TYPE_QUADRANT_BOTTOM_LEFT,
        VIEWPORT_TYPE_QUADRANT_BOTTOM_RIGHT
    };

    enum eTextureFormat
    {
        TEXTURE_FORMAT_RxGyBzAw,
        MAX_TEXTURE_FORMATS
    };

    void Tick();
    void UpdateGamma(unsigned short usGamma);

    void MatrixMode(int type);
    void MatrixSetIdentity();
    void MatrixTranslate(float x, float y, float z);
    void MatrixRotate(float angle, float x, float y, float z);
    void MatrixScale(float x, float y, float z);
    void MatrixPerspective(float fovy, float aspect, float zNear, float zFar);
    void MatrixOrthogonal(float left, float right, float bottom, float top, float zNear, float zFar);
    void MatrixPop();
    void MatrixPush();
    void MatrixMult(float *mat);
    const float *MatrixGet(int type);
    void Set_matrixDirty();

    // Android owns the EGL surface/context. The backend only needs the current
    // surface dimensions, supplied by the JNI bridge.
    void Initialise(int width, int height);
    void InitialiseContext();
    void Resize(int width, int height);
    void StartFrame();
    void DoScreenGrabOnNextPresent();
    void Present();
    void Clear(int flags, void *pRect = nullptr);
    void SetClearColour(const float colourRGBA[4]);
    bool IsWidescreen();
    bool IsHiDef();
    void CaptureThumbnail(ImageFileBuffer *pngOut);
    void CaptureScreen(ImageFileBuffer *jpgOut, XSOCIAL_PREVIEWIMAGE *previewOut);
    void BeginConditionalSurvey(int identifier);
    void EndConditionalSurvey();
    void BeginConditionalRendering(int identifier);
    void EndConditionalRendering();

    void DrawVertices(ePrimitiveType primitiveType, int count, void *dataIn,
                      eVertexType vType, ePixelShaderType psType);
    void DrawVertexBuffer(ePrimitiveType primitiveType, int count, void *buffer,
                          eVertexType vType, ePixelShaderType psType);

    void CBuffLockStaticCreations();
    int CBuffCreate(int count);
    void CBuffDelete(int first, int count);
    void CBuffStart(int index, bool full = false);
    void CBuffClear(int index);
    int CBuffSize(int index);
    void CBuffEnd();
    bool CBuffCall(int index, bool full = true);
    void CBuffTick();
    void CBuffDeferredModeStart();
    void CBuffDeferredModeEnd();

    int TextureCreate();
    void TextureFree(int idx);
    void TextureBind(int idx);
    void TextureBindVertex(int idx);
    void TextureSetTextureLevels(int levels);
    int TextureGetTextureLevels();
    void TextureData(int width, int height, void *data, int level,
                     eTextureFormat format = TEXTURE_FORMAT_RxGyBzAw);
    void TextureDataUpdate(int xoffset, int yoffset, int width, int height,
                           void *data, int level);
    void TextureSetParam(int param, int value);
    void TextureDynamicUpdateStart();
    void TextureDynamicUpdateEnd();
    HRESULT LoadTextureData(const char *filename, D3DXIMAGE_INFO *srcInfo, int **dataOut);
    HRESULT LoadTextureData(BYTE *data, DWORD bytes, D3DXIMAGE_INFO *srcInfo, int **dataOut);
    HRESULT SaveTextureData(const char *filename, D3DXIMAGE_INFO *srcInfo, int *dataOut);
    HRESULT SaveTextureDataToMemory(void *output, int outputCapacity, int *outputLength,
                                    int width, int height, int *dataIn);
    void TextureGetStats();
    GLuint TextureGetTexture(int idx);

    void StateSetColour(float r, float g, float b, float a);
    void StateSetDepthMask(bool enable);
    void StateSetBlendEnable(bool enable);
    void StateSetBlendFunc(int src, int dst);
    void StateSetBlendFactor(unsigned int colour);
    void StateSetAlphaFunc(int func, float param);
    void StateSetDepthFunc(int func);
    void StateSetFaceCull(bool enable);
    void StateSetFaceCullCW(bool enable);
    void StateSetLineWidth(float width);
    void StateSetWriteEnable(bool red, bool green, bool blue, bool alpha);
    void StateSetDepthTestEnable(bool enable);
    void StateSetAlphaTestEnable(bool enable);
    void StateSetDepthSlopeAndBias(float slope, float bias);
    void StateSetFogEnable(bool enable);
    void StateSetFogMode(int mode);
    void StateSetFogNearDistance(float dist);
    void StateSetFogFarDistance(float dist);
    void StateSetFogDensity(float density);
    void StateSetFogColour(float red, float green, float blue);
    void StateSetLightingEnable(bool enable);
    void StateSetVertexTextureUV(float u, float v);
    void StateSetLightColour(int light, float red, float green, float blue);
    void StateSetLightAmbientColour(float red, float green, float blue);
    void StateSetLightDirection(int light, float x, float y, float z);
    void StateSetLightEnable(int light, bool enable);
    void StateSetViewport(eViewportType viewportType);
    void StateSetEnableViewportClipPlanes(bool enable);
    void StateSetTexGenCol(int col, float x, float y, float z, float w, bool eyeSpace);
    void StateSetStencil(int function, uint8_t stencilRef, uint8_t stencilFuncMask,
                         uint8_t stencilWriteMask);
    void StateSetForceLOD(int lod);

    void BeginEvent(LPCWSTR eventName);
    void EndEvent();
    void Suspend();
    bool Suspended();
    void Resume();
};

// Fixed-function constants that GLES 3 intentionally omits but the original
// renderer uses as abstract state identifiers.
#ifndef GL_MODELVIEW
#define GL_MODELVIEW 0x1700
#endif
#ifndef GL_PROJECTION
#define GL_PROJECTION 0x1701
#endif
#ifndef GL_TEXTURE
#define GL_TEXTURE 0x1702
#endif
#ifndef GL_MODELVIEW_MATRIX
#define GL_MODELVIEW_MATRIX 0x0BA6
#endif
#ifndef GL_PROJECTION_MATRIX
#define GL_PROJECTION_MATRIX 0x0BA7
#endif
#ifndef GL_ALPHA_TEST
#define GL_ALPHA_TEST 0x0BC0
#endif
#ifndef GL_LIGHTING
#define GL_LIGHTING 0x0B50
#endif
#ifndef GL_LIGHT0
#define GL_LIGHT0 0x4000
#endif
#ifndef GL_LIGHT1
#define GL_LIGHT1 0x4001
#endif
#ifndef GL_FOG
#define GL_FOG 0x0B60
#endif
#ifndef GL_QUADS
#define GL_QUADS 0x0007
#endif
#ifndef GL_SMOOTH
#define GL_SMOOTH 0x1D01
#endif
#ifndef GL_CLAMP
#define GL_CLAMP 0x2900
#endif

#ifndef GL_FOG_START
#define GL_FOG_START 0x0B63
#endif
#ifndef GL_FOG_END
#define GL_FOG_END 0x0B64
#endif
#ifndef GL_FOG_MODE
#define GL_FOG_MODE 0x0B65
#endif
#ifndef GL_FOG_DENSITY
#define GL_FOG_DENSITY 0x0B62
#endif
#ifndef GL_FOG_COLOR
#define GL_FOG_COLOR 0x0B66
#endif

#ifndef GL_POSITION
#define GL_POSITION 0x1203
#endif
#ifndef GL_AMBIENT
#define GL_AMBIENT 0x1200
#endif
#ifndef GL_DIFFUSE
#define GL_DIFFUSE 0x1201
#endif
#ifndef GL_SPECULAR
#define GL_SPECULAR 0x1202
#endif
#ifndef GL_LIGHT_MODEL_AMBIENT
#define GL_LIGHT_MODEL_AMBIENT 0x0B53
#endif

constexpr int CLEAR_DEPTH_FLAG = 1;
constexpr int CLEAR_COLOUR_FLAG = 2;

extern C4JRender RenderManager;
