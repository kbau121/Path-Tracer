#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "PathTracer.h"

#include "color.h"
#include "hittable_list.h"
#include "stb_image_write.h"
#include "sphere.h"

#include <iostream>

color ray_color(const ray& r, const hittable_list& world) {
	hit_record rec;
	if (world.hit(r, 0, infinity, rec)) {
		return 0.5 * (rec.normal + color(1, 1, 1));
	}

	vec3 unit_direction = unit_vector(r.direction());
	auto t = 0.5 * (unit_direction.y() + 1.0);
	return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}

int main() {
	// Image Settings

	const double ASPECT_RATIO = 16.0 / 9.0;
	const int IMAGE_WIDTH = 256;
	const int IMAGE_HEIGHT = static_cast<int>(IMAGE_WIDTH / ASPECT_RATIO);
	const int IMAGE_CHANNELS = 3;
	const int IMAGE_DATA_STRIDE = IMAGE_WIDTH * IMAGE_CHANNELS;

	// Camera Settings

	double viewport_height = 2.0;
	double viewport_width = ASPECT_RATIO * viewport_height;
	double focal_length = 1.0;

	auto origin = point3(0, 0, 0);
	auto horizontal = vec3(viewport_width, 0, 0);
	auto vertical = vec3(0, viewport_height, 0);
	auto lower_left_corner = origin - (horizontal / 2) - (vertical / 2) - vec3(0, 0, focal_length);

	// World Setup

	hittable_list world;
	world.add(make_shared<sphere>(point3(0, 0, -1), 0.5));
	world.add(make_shared<sphere>(point3(0, -100.5, -1), 100));

	// Render

	unsigned char * data = new unsigned char[IMAGE_WIDTH * IMAGE_HEIGHT * IMAGE_CHANNELS];

	uint32_t ind = 0;
	for (int h = IMAGE_HEIGHT - 1; h >= 0; --h) {
		printf("%f%%\n", 100 * float(IMAGE_HEIGHT - (h + 1)) / IMAGE_HEIGHT);

		for (int w = 0; w < IMAGE_WIDTH; ++w) {
			auto u = w / (double(IMAGE_WIDTH) - 1);
			auto v = h / (double(IMAGE_HEIGHT) - 1);

			ray r(origin, lower_left_corner + (u * horizontal) + (v * vertical) - origin);
			color pixel_color = ray_color(r, world);

			data[ind++] = unsigned char (255 * pixel_color.x());
			data[ind++] = unsigned char (255 * pixel_color.y());
			data[ind++] = unsigned char (255 * pixel_color.z());
		}
	}

	printf("%f%%\n", 100.f);
	printf("\nDone\n");

	// Save Output

	stbi_write_png("output.png", IMAGE_WIDTH, IMAGE_HEIGHT, IMAGE_CHANNELS, data, IMAGE_DATA_STRIDE);

	delete[] data;

	return EXIT_SUCCESS;
}