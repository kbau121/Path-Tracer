#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "PathTracer.h"

#include "camera.h"
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

	const double aspect_ratio = 16.0 / 9.0;
	const int image_width = 1024;
	const int image_height = static_cast<int>(image_width / aspect_ratio);
	const int image_channels = 3;
	const int image_data_stride = image_width * image_channels;
	const int samples_per_pixel = 100;

	// Camera Settings

	camera cam;

	// World Setup

	hittable_list world;
	world.add(make_shared<sphere>(point3(0, 0, -1), 0.5));
	world.add(make_shared<sphere>(point3(0, -100.5, -1), 100));

	// Render

	unsigned char * data = new unsigned char[image_width * image_height * image_channels];

	uint32_t ind = 0;
	for (int h = image_height - 1; h >= 0; --h) {
		printf("%f%%\n", 100 * float(image_height - (h + 1)) / image_height);

		for (int w = 0; w < image_width; ++w) {
			color pixel_color(0, 0, 0);
			for (int s = 0; s < samples_per_pixel; ++s) {
				auto u = (w + random_double()) / (image_width - 1);
				auto v = (h + random_double()) / (image_height - 1);

				ray r = cam.get_ray(u, v);
				pixel_color += ray_color(r, world);
			}

			/*
			data[ind++] = unsigned char (255 * pixel_color.x());
			data[ind++] = unsigned char (255 * pixel_color.y());
			data[ind++] = unsigned char (255 * pixel_color.z());
			*/

			write_color(data[ind], pixel_color, samples_per_pixel);
			ind += image_channels;
		}
	}

	printf("%f%%\n", 100.f);
	printf("\nDone\n");

	// Save Output

	stbi_write_png("output.png", image_width, image_height, image_channels, data, image_data_stride);

	delete[] data;

	return EXIT_SUCCESS;
}