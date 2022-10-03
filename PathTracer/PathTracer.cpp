#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "PathTracer.h"

#include "camera.h"
#include "color.h"
#include "hittable_list.h"
#include "material.h"
#include "obj_reader.h"
#include "stb_image_write.h"
#include "sphere.h"
#include "thread_pool.h"
#include "triangle.h"

#include <chrono>
#include <iostream>

#include <embree3/rtcore.h>
#include "glm/glm.hpp"

using std::thread;

vec3 ray_color(const ray& r, RTCScene* scene, int depth) {
	hit_record rec;

	if (depth <= 0) {
		return vec3(0.f);
	}

	RTCRayHit rayhit;
	rayhit.ray.org_x = r.orig.x;
	rayhit.ray.org_y = r.orig.y;
	rayhit.ray.org_z = r.orig.z;
	rayhit.ray.dir_x = r.dir.x;
	rayhit.ray.dir_y = r.dir.y;
	rayhit.ray.dir_z = r.dir.z;
	rayhit.ray.tnear = 0.001f;
	rayhit.ray.tfar = infinity;
	rayhit.ray.mask = -1;
	rayhit.ray.flags = 0;
	rayhit.hit.geomID = -1;
	rayhit.hit.instID[0] = -1;

	RTCIntersectContext context;
	rtcInitIntersectContext(&context);
	rtcIntersect1(*scene, &context, &rayhit);

	if (rayhit.hit.geomID != -1) {
		ray r_out;
		vec3 attenuation;

		rec.t = rayhit.ray.tfar;
		rec.p = vec3(r.at(rayhit.ray.tfar));
		rec.set_face_normal(r, normalize(vec3(rayhit.hit.Ng_x, rayhit.hit.Ng_y, rayhit.hit.Ng_z)));
		rec.mat_ptr = make_shared<normal>();

		if (rec.mat_ptr->scatter(r, rec, attenuation, r_out)) {
			//return attenuation * ray_color(r_out, scene, depth - 1);
			return attenuation;
		}

		return vec3(0.f);
	}

	vec3 unit_direction = normalize(r.direction());
	float t = 0.5f * (unit_direction.y + 1.f);
	return (1.f - t) * vec3(1.f, 1.f, 1.f) + t * vec3(0.5f, 0.7f, 1.f);
}

hittable_list sample_scene() {
	hittable_list world;
	
	auto material_ground = make_shared<lambertian>(vec3(0.8f, 0.8f, 0.f));
	auto material_left = make_shared<dielectric>(1.5f);
	auto material_center = make_shared<lambertian>(vec3(0.1f, 0.2f, 0.5f));
	auto material_right = make_shared<metal>(vec3(0.8f, 0.6f, 0.2f), 0.f);

	world.add(make_shared<sphere>(vec3(0.f, -100.5f, -1.f), 100.f, material_ground));
	world.add(make_shared<sphere>(vec3(-1.f, 0.f, -1.f), 0.5f, material_left));
	world.add(make_shared<sphere>(vec3(-1.f, 0.f, -1.f), -0.45f, material_left));
	world.add(make_shared<sphere>(vec3(0.f, 0.f, -1.f), 0.5f, material_center));
	world.add(make_shared<sphere>(vec3(1.f, 0.f, -1.f), 0.5f, material_right));

	return world;
}

hittable_list test_scene() {
	auto material_ground = make_shared<lambertian>(vec3(0.5f, 0.5f, 0.5f));
	auto material_metal = make_shared<metal>(vec3(0.7f, 0.6f, 0.5f), 0.f);
	auto material_lambertian = make_shared<lambertian>(vec3(0.1f, 0.2f, 0.5f));
	auto material_normal = make_shared<normal>();

	hittable_list world;

	world.add(make_shared<sphere>(vec3(0.f, -1000.f, 0.f), 1000.f, material_ground));

	vec3 p0 = vec3(0.f, 0.f, -1.f);
	vec3 p1 = vec3(2.f, 0.f, -2.f);
	vec3 p2 = vec3(0.f, 2.f, -2.f);
	vec3 p3 = vec3(2.f, 2.f, -1.f);

	world.add(make_shared<triangle>(
		p0,
		p1,
		p2,
		cross(p1 - p0, p2 - p0),
		vec3(0.f, 0.f, 1.f),
		vec3(0.f, 0.f, 1.f),
		material_normal
		));

	world.add(make_shared<triangle>(
		p3,
		p1,
		p2,
		-cross(p1 - p3, p2 - p3),
		vec3(0.f, 0.f, 1.f),
		vec3(0.f, 0.f, 1.f),
		material_normal
		));
	
	world.add(make_shared<sphere>(
		vec3(0.f, 0.5f, 0.5f),
		0.5f,
		material_lambertian
		));

	return world;
}

hittable_list random_spheres_scene() {
	hittable_list world;

	auto ground_mat = make_shared<lambertian>(vec3(0.5f, 0.5f, 0.5f));
	world.add(make_shared<sphere>(vec3(0.f, -1000.f, 0.f), 1000.f, ground_mat));

	for (int x = -11; x < 11; ++x) {
		for (int y = -11; y < 11; ++y) {
			float choose_mat = random_float();
			vec3 center(x + 0.9f * random_float(), 0.2f, y + 0.9f * random_float());

			if ((center - vec3(4.f, 0.2f, 0.f)).length() > 0.9f) {
				shared_ptr<material> next_mat;

				if (choose_mat < 0.8f) {
					// lambertian
					vec3 albedo = random_vec3() * random_vec3();
					next_mat = make_shared<lambertian>(albedo);
				}
				else if (choose_mat < 0.95f) {
					// metal
					vec3 albedo = random_vec3(0.5f, 1.f);
					float roughness = random_float(0.f, 0.5f);
					next_mat = make_shared<metal>(albedo, roughness);
				}
				else {
					// glass
					next_mat = make_shared<dielectric>(1.5f);
				}

				world.add(make_shared<sphere>(center, 0.2f, next_mat));
			}
		}
	}

	auto dielectric_mat = make_shared<dielectric>(1.5f);
	world.add(make_shared<sphere>(vec3(0.f, 1.f, 0.f), 1.f, dielectric_mat));

	auto lambertian_mat = make_shared<lambertian>(vec3(0.4f, 0.2f, 0.1f));
	world.add(make_shared<sphere>(vec3(-4.f, 1.f, 0.f), 1.f, lambertian_mat));

	auto metal_mat = make_shared<metal>(vec3(0.7f, 0.6f, 0.5f), 0.f);
	world.add(make_shared<sphere>(vec3(4.f, 1.f, 0.f), 1.f, metal_mat));

	return world;
}

void sample_pixel
(
	int w, int h,
	const int image_width, const int image_height,
	const int samples_per_pixel, const int max_depth, const int image_channels,
	camera cam,
	RTCScene* scene,
	unsigned char* data
)
{
	vec3 pixel_color(0.f, 0.f, 0.f);
	for (int s = 0; s < samples_per_pixel; ++s) {
		float u = (w + random_float()) / (image_width - 1);
		float v = (h + random_float()) / (image_height - 1);

		ray r = cam.get_ray(u, v);
		pixel_color += ray_color(r, scene, max_depth);
	}

	const int ind = ((image_height - h - 1) * image_width + w) * image_channels;
	write_color(&data[ind], pixel_color, samples_per_pixel);
}

void sample_rect
(
	int x_s, int y_s, const int rect_width, const int rect_height,
	const int image_width, const int image_height,
	const int samples_per_pixel, const int max_depth, const int image_channels,
	camera cam,
	RTCScene* scene,
	unsigned char* data
)
{
	int y_max = std::min(y_s + rect_height, image_height);
	int x_max = std::min(x_s + rect_width, image_width);

	for (int y = y_s; y < y_max; ++y) {
		for (int x = x_s; x < x_max; ++x) {
			sample_pixel(x, y, image_width, image_height, samples_per_pixel, max_depth, image_channels, cam, scene, data);
		}
	}
}

int main() {
	// Image Settings

	const float aspect_ratio = 3.f / 2.f;
	const int image_width = 256;
	const int image_height = static_cast<int>(image_width / aspect_ratio);
	const int image_channels = 3;
	const int image_data_stride = image_width * image_channels;
	const int samples_per_pixel = 128;
	const int max_depth = 64;

	// Camera Settings

	// Final Render Settings
	/*
	point3 camera_position(13.0, 2.0, 3.0);
	point3 camera_lookat(0.0, 0.0, 0.0);
	vec3 camera_up(0.0, 1.0, 0.0);
	float aperture = 0.1f;
	float focus_distance = 10.f;
	*/

	// Sample Settings
	vec3 camera_position(0.0, 0.0, 7.0);
	vec3 camera_lookat(0.0, 0.0, 0.0);
	vec3 camera_up(0.0, 1.0, 0.0);
	float aperture = 0.1f;
	float focus_distance = length(camera_position - camera_lookat);

	camera cam(camera_position, camera_lookat, camera_up, 20, aspect_ratio, aperture, focus_distance);

	// World Setup

	RTCDevice device = rtcNewDevice("");
	RTCScene scene = rtcNewScene(device);
	RTCGeometry geometry = read_obj("C:\\Users\\Chocomann\\Downloads\\monke.obj", device);

	//RTCGeometry geometry = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
	/*
	glm::vec3* verts = (glm::vec3*) rtcSetNewGeometryBuffer(geometry, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, sizeof(glm::vec3), 3);
	glm::uvec3* inds = (glm::uvec3*) rtcSetNewGeometryBuffer(geometry, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, sizeof(glm::uvec3), 3);

	verts[0] = glm::vec3(0, 0, 0);
	verts[1] = glm::vec3(1, 0, 0);
	verts[2] = glm::vec3(0.5, 1, 0);

	inds[0] = glm::uvec3(0, 1, 2);

	rtcCommitGeometry(geometry);
	*/

	rtcAttachGeometry(scene, geometry);
	rtcReleaseGeometry(geometry);
	geometry = nullptr; // geometry, more like, ge-nope-etry
	rtcCommitScene(scene);

	RTCScene* scene_ptr = &scene;

	//context.flags = RTC_INTERSECT_CONTEXT_FLAG_INCOHERENT; // try later

	// Render
	
	unsigned char * data = new unsigned char[image_width * image_height * image_channels];
	const int rect_width = 64;
	const int rect_height = 64;

	auto time_s = std::chrono::high_resolution_clock::now();

	// Initialize the thread pool
	thread_pool pool;

	// Queue all jobs in the thread pool
	printf("Starting work...\n");
	pool.Start(1, int(std::ceilf(float(image_height) / rect_height) * std::ceilf(float(image_width) / rect_width)));

	for (int y = 0; y < image_height; y += rect_height) {
		for (int x = 0; x < image_width; x += rect_width) {
			sample_rect(x, y, rect_width, rect_height, image_width, image_height, samples_per_pixel, max_depth, image_channels, cam, scene_ptr, data);

			/*
			pool.QueueJob(
				[x, y, rect_width, rect_height, image_width, image_height, samples_per_pixel, max_depth, image_channels, cam, _scene, _context, data]
				{
					sample_rect(x, y, rect_width, rect_height,
						image_width, image_height, samples_per_pixel, max_depth, image_channels,
						cam, _scene, _context, data);
				});
				*/
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

	// Cleanup

	rtcReleaseScene(scene);
	rtcReleaseDevice(device);

	delete[] data;

	return EXIT_SUCCESS;
}