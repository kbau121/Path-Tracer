#pragma once

#include "vec3.h"

#include <iostream>

int* get_RGB(color pixel_color, int samples_per_pixel) {
	int* RGB = new int[3];

	double r = pixel_color.x();
	double g = pixel_color.y();
	double b = pixel_color.z();

	// Scale based on the number of samples
	double scale = 1.0 / samples_per_pixel;
	r = sqrt(scale * r);
	g = sqrt(scale * g);
	b = sqrt(scale * b);

	RGB[0] = unsigned char(256 * clamp(r, 0.0, 0.999));
	RGB[1] = unsigned char(256 * clamp(g, 0.0, 0.999));
	RGB[2] = unsigned char(256 * clamp(b, 0.0, 0.999));

	return RGB;
}

void write_color(std::ostream& out, color pixel_color, int samples_per_pixel) {
	int* RGB = get_RGB(pixel_color, samples_per_pixel);

	out << RGB[0] << ' '
		<< RGB[1] << ' '
		<< RGB[2] << '\n';
}

void write_color(unsigned char* data, color pixel_color, int samples_per_pixel) {
	int* RGB = get_RGB(pixel_color, samples_per_pixel);

	data[0] = RGB[0];
	data[1] = RGB[1];
	data[2] = RGB[2];
}