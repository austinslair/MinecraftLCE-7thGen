#pragma once

// Android-only compatibility PCH used while progressively bringing the
// original Minecraft.Client translation units into the NDK build.
// The original client sources stay unchanged; CMake remaps their stdafx.h
// include to this file with a Clang VFS overlay.

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <math.h>
#include <assert.h>

#include <algorithm>
#include <deque>
#include <exception>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "AndroidTypes.h"
#include "AndroidMaths.h"
#include "AndroidStubs.h"
#include "sal.h"

// Shared declarations that the original client/world precompiled headers
// provide before entity headers. Keep using the original LCE definitions
// instead of duplicating enums or class metadata in the Android layer.
#include "Class.h"

// Many original Minecraft.Client headers rely on the precompiled header to
// provide the shared array typedefs (floatArray, byteArray, etc.). The Android
// System.h remap breaks the original include cycle, so this can be included
// safely here without editing those client headers.
#include "ArrayWithLength.h"

// Biome.h expects the original PCH to supply eMinecraftColour.
#include "Common/App_enums.h"

#ifndef AUTO_VAR
#define AUTO_VAR(_var, _val) auto _var = _val
#endif

using namespace std;

// The original x64 compatibility layer implements XLockFreeStack as a
// critical-section-protected vector. Preserve that API/behavior on Android so
// untouched client code such as LevelRenderer can compile and keep its LIFO
// semantics. The values are intentionally T* because the original code also
// uses small integer values encoded as pointers.
template <typename T>
class XLockFreeStack
{
public:
    XLockFreeStack()
    {
        InitializeCriticalSection(&m_cs);
    }

    ~XLockFreeStack()
    {
        DeleteCriticalSection(&m_cs);
    }

    void Initialize() {}

    void Push(T* data)
    {
        EnterCriticalSection(&m_cs);
        m_stack.push_back(data);
        LeaveCriticalSection(&m_cs);
    }

    T* Pop()
    {
        EnterCriticalSection(&m_cs);
        if (m_stack.empty())
        {
            LeaveCriticalSection(&m_cs);
            return nullptr;
        }

        T* value = m_stack.back();
        m_stack.pop_back();
        LeaveCriticalSection(&m_cs);
        return value;
    }

private:
    std::vector<T*> m_stack;
    CRITICAL_SECTION m_cs;
};

void MemSect(int sect);
