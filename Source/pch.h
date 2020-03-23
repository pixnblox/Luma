#pragma once

// STL headers.
#define _USE_MATH_DEFINES
#include <algorithm>
#include <cassert>
#include <cfloat>
#include <chrono>
#include <cmath>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <vector>
using namespace std;

// Float version of pi, to avoid casts where it is needed.
const float M_PI_F = static_cast<float>(M_PI);
