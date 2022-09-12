#pragma once

#include <cmath>
#include <cstdlib>
#include <limits>
#include <memory>
#include <random>

// Usings

using std::shared_ptr;
using std::make_shared;
using std::sqrt;

// Constants

const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// Utility Functions

inline double degrees_to_radians(double degrees) {
	return degrees * pi / 180;
}

inline double random_double(double min, double max) {
	// Returns a random double in the range [min,max]
	static thread_local std::mt19937 generator;
	std::uniform_real_distribution<double> distribution(min, max);
	return distribution(generator);
}

inline double random_double() {
	// Returns a random double in the range [0,1]
	return random_double(0, 1);
}

inline double clamp(double x, double min, double max) {
	if (x < min) return min;
	if (x > max) return max;
	return x;
}

// Common Includes

#include "ray.h"
#include "vec3.h"