#pragma once

#include "glm/glm.hpp"

#include <iostream>

int* get_RGB(glm::vec3 pixel_color, int samples_per_pixel) {
	int* RGB = new int[3];

	float r = pixel_color.r;
	float g = pixel_color.g;
	float b = pixel_color.b;

	// Scale based on the number of samples
	float scale = 1.f / samples_per_pixel;
	r = sqrtf(scale * r);
	g = sqrtf(scale * g);
	b = sqrtf(scale * b);

	RGB[0] = unsigned char(256 * clamp(r, 0.f, 0.999f));
	RGB[1] = unsigned char(256 * clamp(g, 0.f, 0.999f));
	RGB[2] = unsigned char(256 * clamp(b, 0.f, 0.999f));

	return RGB;
}

void write_color(std::ostream& out, glm::vec3 pixel_color, int samples_per_pixel) {
	int* RGB = get_RGB(pixel_color, samples_per_pixel);

	out << RGB[0] << ' '
		<< RGB[1] << ' '
		<< RGB[2] << '\n';
}

void write_color(unsigned char* data, glm::vec3 pixel_color, int samples_per_pixel) {
	int* RGB = get_RGB(pixel_color, samples_per_pixel);

	data[0] = RGB[0];
	data[1] = RGB[1];
	data[2] = RGB[2];
}