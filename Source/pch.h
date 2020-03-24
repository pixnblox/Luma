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

// Make certain names from the std namespace accessible.
using std::make_shared;
using std::shared_ptr;
using std::string;
using std::vector;

// Math constants.
const float PI = static_cast<float>(M_PI);
const float INF = std::numeric_limits<float>::infinity();
