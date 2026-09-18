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

#ifndef AUTO_VAR
#define AUTO_VAR(_var, _val) auto _var = _val
#endif

using namespace std;

void MemSect(int sect);
