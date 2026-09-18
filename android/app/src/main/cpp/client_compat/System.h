#pragma once

#include "AndroidTypes.h"

template <class T> class arrayWithLength;
class Node;
class Biome;

class System
{
public:
    static void arraycopy(arrayWithLength<byte> src, unsigned int srcPos, arrayWithLength<byte>* dst, unsigned int dstPos, unsigned int length);
    static void arraycopy(arrayWithLength<Node*> src, unsigned int srcPos, arrayWithLength<Node*>* dst, unsigned int dstPos, unsigned int length);
    static void arraycopy(arrayWithLength<Biome*> src, unsigned int srcPos, arrayWithLength<Biome*>* dst, unsigned int dstPos, unsigned int length);
    static void arraycopy(arrayWithLength<int> src, unsigned int srcPos, arrayWithLength<int>* dst, unsigned int dstPos, unsigned int length);

    static __int64 nanoTime();
    static __int64 currentTimeMillis();
    static __int64 currentRealTimeMillis();

    static void ReverseUSHORT(unsigned short* pusVal);
    static void ReverseSHORT(short* psVal);
    static void ReverseULONG(unsigned long* pulVal);
    static void ReverseULONG(unsigned int* pulVal);
    static void ReverseINT(int* piVal);
    static void ReverseULONGLONG(__int64* pullVal);
    static void ReverseWCHARA(WCHAR* pwch, int iLen);
};

#ifndef MAKE_FOURCC
#define MAKE_FOURCC(ch0, ch1, ch2, ch3) \
    ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) | \
    ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24))
#endif
