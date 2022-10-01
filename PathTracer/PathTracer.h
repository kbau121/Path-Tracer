#pragma once

#include <cmath>
#include <cstdlib>
#include <limits>
#include <memory>
#include <random>

#include "glm/glm.hpp"
#include "glm/gtx/norm.hpp"

// Usings

using std::shared_ptr;
using std::make_shared;
using std::sqrt;
using glm::vec3;

// Constants

const float infinity = std::numeric_limits<float>::infinity();
const float pi = 3.1415926535897932385f;

// Utility Functions

inline float degrees_to_radians(float degrees) {
	return degrees * pi / 180;
}

inline float random_float(float min, float max) {
	static thread_local std::mt19937 generator;
	std::uniform_real_distribution<float> distribution(min, max);
	return distribution(generator);
}

inline float random_float() {
	return random_float(0.f, 1.f);
}

inline vec3 random_vec3() {
	return vec3(random_float(), random_float(), random_float());
}

inline vec3 random_vec3(float min, float max) {
	return vec3(random_float(min, max), random_float(min, max), random_float(min, max));
}

inline vec3 random_vec3_in_unit_disk() {
	while (true) {
		vec3 p = vec3(random_float(-1.f, 1.f), random_float(-1.f, 1.f), 0.f);
		if (glm::length2(p) <= 1) {
			return p;
		}
	}
}

inline vec3 random_vec3_in_unit_sphere() {
	while (true) {
		auto p = random_vec3(-1, 1);

		if (length2(p) <= 1.0) {
			return p;
		}
	}
}

inline float clamp(float x, float min, float max) {
	if (x < min) return min;
	if (x > max) return max;
	return x;
}

inline bool is_near_zero(vec3 u) {
	const float s = 1e-8f;
	return (fabs(u.x) < s) && (fabs(u.y) < s) && (fabs(u.z) < s);
}

// Common Includes

#include "ray.h"