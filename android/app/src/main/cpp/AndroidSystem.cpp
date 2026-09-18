#include "client_compat/System.h"

#include <cassert>
#include <chrono>
#include <cstring>

// Include the original container definition only after System has been
// declared. This avoids the original circular include where NBT headers call
// System::arraycopy while System.h is still being parsed.
#include "ArrayWithLength.h"

namespace
{
template <typename T>
void arraycopyImpl(arrayWithLength<T> src, unsigned int srcPos,
                   arrayWithLength<T>* dst, unsigned int dstPos,
                   unsigned int length)
{
    if (length == 0)
        return;

    assert(dst != nullptr);
    assert(src.data != nullptr);
    assert(dst->data != nullptr);
    assert(srcPos <= src.length && length <= src.length - srcPos);
    assert(dstPos <= dst->length && length <= dst->length - dstPos);

    std::memmove(dst->data + dstPos, src.data + srcPos, sizeof(T) * length);
}
}

void System::arraycopy(arrayWithLength<byte> src, unsigned int srcPos,
                       arrayWithLength<byte>* dst, unsigned int dstPos,
                       unsigned int length)
{
    arraycopyImpl(src, srcPos, dst, dstPos, length);
}

void System::arraycopy(arrayWithLength<Node*> src, unsigned int srcPos,
                       arrayWithLength<Node*>* dst, unsigned int dstPos,
                       unsigned int length)
{
    arraycopyImpl(src, srcPos, dst, dstPos, length);
}

void System::arraycopy(arrayWithLength<Biome*> src, unsigned int srcPos,
                       arrayWithLength<Biome*>* dst, unsigned int dstPos,
                       unsigned int length)
{
    arraycopyImpl(src, srcPos, dst, dstPos, length);
}

void System::arraycopy(arrayWithLength<int> src, unsigned int srcPos,
                       arrayWithLength<int>* dst, unsigned int dstPos,
                       unsigned int length)
{
    arraycopyImpl(src, srcPos, dst, dstPos, length);
}

__int64 System::nanoTime()
{
    using namespace std::chrono;
    return static_cast<__int64>(duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count());
}

__int64 System::currentTimeMillis()
{
    using namespace std::chrono;
    return static_cast<__int64>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
}

__int64 System::currentRealTimeMillis()
{
    return currentTimeMillis();
}
