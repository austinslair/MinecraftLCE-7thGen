#include "AndroidTypes.h"

#include <algorithm>
#include <chrono>
#include <memory>

#include "System.h"

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
