#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string>
#include <memory>

typedef uint8_t   BYTE;
typedef uint16_t  WORD;
typedef uint32_t  DWORD;
typedef uint32_t  UINT;
typedef int32_t   BOOL;
typedef int32_t   LONG;
typedef int64_t   LONGLONG;
typedef uint64_t  ULONGLONG;
typedef int64_t   __int64;
typedef uint64_t  __uint64;
typedef float     FLOAT;
typedef void*     HANDLE;
typedef void*     LPVOID;
typedef const void* LPCVOID;
typedef const char*    LPCSTR;
typedef const wchar_t* LPCWSTR;
typedef int32_t   HRESULT;

typedef uint64_t  XUID;
typedef uint64_t  PlayerUID;
typedef uint64_t  SessionID;
typedef uint64_t  GameSessionUID;

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
using std::shared_ptr;
using std::weak_ptr;
using std::make_shared;

class Entity;
