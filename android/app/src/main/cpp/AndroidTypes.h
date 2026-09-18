#pragma once
#define _HAS_STD_BYTE 0
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <cstring>
#include <string>
#include <memory>
#include <iostream>

#define byte unsigned char
typedef void      VOID;
typedef uint8_t   BYTE;
typedef uint16_t  WORD;
typedef uint32_t  DWORD;
typedef uint32_t  UINT;
typedef int32_t   BOOL;
typedef int32_t   LONG;
typedef int64_t   LONGLONG;
typedef uint64_t  ULONGLONG;

// Android builds use Clang with -fms-extensions so the original LCE sources
// can keep Microsoft integer spellings such as __int64. Under that mode
// __int64 is a language keyword and must not be typedef'd again.
#if defined(__clang__)
typedef unsigned __int64 __uint64;
#else
typedef int64_t   __int64;
typedef uint64_t  __uint64;
#endif

typedef float     FLOAT;
typedef void*     HANDLE;
typedef void*     LPVOID;
typedef const void* LPCVOID;
typedef const char*    LPCSTR;
typedef const wchar_t* LPCWSTR;
typedef wchar_t   WCHAR;
typedef int32_t   HRESULT;

typedef union _LARGE_INTEGER {
    struct {
        DWORD LowPart;
        LONG HighPart;
    };
    struct {
        DWORD LowPart;
        LONG HighPart;
    } u;
    LONGLONG QuadPart;
} LARGE_INTEGER;

typedef void* XMEMCOMPRESSION_CONTEXT;
typedef void* XMEMDECOMPRESSION_CONTEXT;

typedef uint64_t  XUID;
typedef uint64_t  PlayerUID;
typedef uint64_t  SessionID;
typedef uint64_t  GameSessionUID;

// XInput exposes four local user slots. Preserve that platform contract so
// original client structures keep their expected layout on Android.
#ifndef XUSER_MAX_COUNT
#define XUSER_MAX_COUNT 4
#endif

#define ZeroMemory(Destination,Length) memset((Destination),0,(Length))
#define CopyMemory(Destination,Source,Length) memcpy((Destination),(Source),(Length))

#define S_OK      ((HRESULT)0L)
#define S_FALSE   ((HRESULT)1L)
#define E_FAIL    ((HRESULT)0x80004005L)
#define FAILED(hr) (((HRESULT)(hr)) < 0)
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#define HRESULT_SUCCEEDED(hr) SUCCEEDED(hr)

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define AUTO_VAR(_var, _val) auto _var = _val

using std::string;
using std::wstring;
using std::ostream;
using std::wostream;
using std::endl;
using std::shared_ptr;
using std::weak_ptr;
using std::make_shared;

class Entity;

#ifndef WINAPI
#define WINAPI
#endif

typedef uintptr_t ULONG_PTR;
typedef uint32_t  ULONG;
typedef intptr_t  LONG_PTR;
typedef int32_t   INT;
