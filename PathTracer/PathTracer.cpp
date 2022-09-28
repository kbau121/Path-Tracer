#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "PathTracer.h"

#include "camera.h"
#include "color.h"
#include "hittable_list.h"
#include "material.h"
#include "stb_image_write.h"
#include "sphere.h"
#include "thread_pool.h"
#include "triangle.h"

#include <chrono>
#include <iostream>

using std::thread;

color ray_color(const ray& r, const hittable_list& world, int depth) {
	hit_record rec;

	if (depth <= 0) {
		return color(0, 0, 0);
	}

	if (world.hit(r, 0.001, infinity, rec)) {
		ray r_out;
		color attenuation;

		if (rec.mat_ptr->scatter(r, rec, attenuation, r_out)) {
			return attenuation * ray_color(r_out, world, depth - 1);
		}

		return color(0, 0, 0);
	}

	vec3 unit_direction = unit_vector(r.direction());
	double t = 0.5 * (unit_direction.y() + 1.0);
	return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}

hittable_list sample_scene() {
	hittable_list world;
	
	auto material_ground = make_shared<lambertian>(color(0.8, 0.8, 0.0));
	auto material_left = make_shared<dielectric>(1.5);
	auto material_center = make_shared<lambertian>(color(0.1, 0.2, 0.5));
	auto material_right = make_shared<metal>(color(0.8, 0.6, 0.2), 0.0);

	world.add(make_shared<sphere>(point3(0.0, -100.5, -1.0), 100.0, material_ground));
	world.add(make_shared<sphere>(point3(-1.0, 0.0, -1.0), 0.5, material_left));
	world.add(make_shared<sphere>(point3(-1.0, 0.0, -1.0), -0.45, material_left));
	world.add(make_shared<sphere>(point3(0.0, 0.0, -1.0), 0.5, material_center));
	world.add(make_shared<sphere>(point3(1.0, 0.0, -1.0), 0.5, material_right));

	return world;
}

hittable_list test_scene() {
	auto material_ground = make_shared<lambertian>(color(0.5, 0.5, 0.5));
	auto material_metal = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
	auto material_lambertian = make_shared<lambertian>(color(0.1, 0.2, 0.5));

	hittable_list world;

	world.add(make_shared<sphere>(point3(0.0, -1000.0, 0.0), 1000, material_ground));

	world.add(make_shared<triangle>(
		point3(0.0, 0.0, -1.0),
		point3(1.0, 0.0, -1.0),
		point3(0.0, 1.0, -1.0),
		material_metal
		));
	
	world.add(make_shared<sphere>(
		point3(0.0, 0.5, 0.0),
		0.5,
		material_lambertian
		));

	return world;
}

hittable_list random_spheres_scene() {
	hittable_list world;

	auto ground_mat = make_shared<lambertian>(color(0.5, 0.5, 0.5));
	world.add(make_shared<sphere>(point3(0.0, -1000.0, 0.0), 1000, ground_mat));

	for (int x = -11; x < 11; ++x) {
		for (int y = -11; y < 11; ++y) {
			double choose_mat = random_double();
			point3 center(x + 0.9 * random_double(), 0.2, y + 0.9 * random_double());

			if ((center - point3(4, 0.2, 0)).length() > 0.9) {
				shared_ptr<material> next_mat;

				if (choose_mat < 0.8) {
					// lambertian
					color albedo = color::random() * color::random();
					next_mat = make_shared<lambertian>(albedo);
				}
				else if (choose_mat < 0.95) {
					// metal
					color albedo = color::random(0.5, 1);
					double roughness = random_double(0, 0.5);
					next_mat = make_shared<metal>(albedo, roughness);
				}
				else {
					// glass
					next_mat = make_shared<dielectric>(1.5);
				}

				world.add(make_shared<sphere>(center, 0.2, next_mat));
			}
		}
	}

	auto dielectric_mat = make_shared<dielectric>(1.5);
	world.add(make_shared<sphere>(point3(0.0, 1.0, 0.0), 1.0, dielectric_mat));

	auto lambertian_mat = make_shared<lambertian>(color(0.4, 0.2, 0.1));
	world.add(make_shared<sphere>(point3(-4.0, 1.0, 0.0), 1.0, lambertian_mat));

	auto metal_mat = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
	world.add(make_shared<sphere>(point3(4.0, 1.0, 0.0), 1.0, metal_mat));

	return world;
}

void sample_pixel
(
	int w, int h,
	const int image_width, const int image_height,
	const int samples_per_pixel, const int max_depth, const int image_channels,
	camera cam, hittable_list world,
	unsigned char* data
)
{
	color pixel_color(0, 0, 0);
	for (int s = 0; s < samples_per_pixel; ++s) {
		double u = (w + random_double()) / (image_width - 1);
		double v = (h + random_double()) / (image_height - 1);

		ray r = cam.get_ray(u, v);
		pixel_color += ray_color(r, world, max_depth);
	}

	const int ind = ((image_height - h - 1) * image_width + w) * image_channels;
	write_color(&data[ind], pixel_color, samples_per_pixel);
}

void sample_rect
(
	int x_s, int y_s, const int rect_width, const int rect_height,
	const int image_width, const int image_height,
	const int samples_per_pixel, const int max_depth, const int image_channels,
	camera cam, hittable_list world,
	unsigned char* data
)
{
	int y_max = std::min(y_s + rect_height, image_height);
	int x_max = std::min(x_s + rect_width, image_width);

	for (int y = y_s; y < y_max; ++y) {
		for (int x = x_s; x < x_max; ++x) {
			sample_pixel(x, y, image_width, image_height, samples_per_pixel, max_depth, image_channels, cam, world, data);
		}
	}
}

int main() {
	// Image Settings

	const double aspect_ratio = 3.0 / 2.0;
	const int image_width = 256;
	const int image_height = static_cast<int>(image_width / aspect_ratio);
	const int image_channels = 3;
	const int image_data_stride = image_width * image_channels;
	const int samples_per_pixel = 100;
	const int max_depth = 50;

	// Camera Settings

	// Final Render Settings
	/*
	point3 camera_position(13.0, 2.0, 3.0);
	point3 camera_lookat(0.0, 0.0, 0.0);
	vec3 camera_up(0.0, 1.0, 0.0);
	double aperture = 0.1;
	double focus_distance = 10.0;
	*/

	// Sample Settings
	point3 camera_position(3.0, 2.0, 5.0);
	point3 camera_lookat(0.0, 0.0, -1.0);
	vec3 camera_up(0.0, 1.0, 0.0);
	double aperture = 0.1;
	double focus_distance = (camera_position - camera_lookat).length();

	camera cam(camera_position, camera_lookat, camera_up, 20, aspect_ratio, aperture, focus_distance);

	// World Setup

	hittable_list world = test_scene();

	// Render
	
	unsigned char * data = new unsigned char[image_width * image_height * image_channels];
	const int rect_width = 64;
	const int rect_height = 64;

	auto time_s = std::chrono::high_resolution_clock::now();

	// Initialize the thread pool
	thread_pool pool;

	// Queue all jobs in the thread pool
	printf("Starting work...\n");
	pool.Start(1, int(std::ceil(double(image_height) / rect_height) * std::ceil(double(image_width) / rect_width)));

	for (int y = 0; y < image_height; y += rect_height) {
		for (int x = 0; x < image_width; x += rect_width) {
			pool.QueueJob(
				[x, y, rect_width, rect_height, image_width, image_height, samples_per_pixel, max_depth, image_channels, cam, world, data]
				{
					sample_rect(x, y, rect_width, rect_height,
						image_width, image_height, samples_per_pixel, max_depth, image_channels,
						cam, world, data);
				});
		}
	}

	// Wait for all threads to finish
	while (pool.IsBusy());
	pool.Stop();

	auto time_f = std::chrono::high_resolution_clock::now();
	auto duration = (time_f - time_s);
	auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
	auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration - hours);
	auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration - hours - minutes);
	auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration - hours - minutes - seconds);;

	printf("\nElapsed time: %02d:%02d:%02lld:%04lld\n", hours.count(), minutes.count(), seconds.count(), milliseconds.count());

	// Save Output

	stbi_write_png("output.png", image_width, image_height, image_channels, data, image_data_stride);

	delete[] data;

	return EXIT_SUCCESS;
}