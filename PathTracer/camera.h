#pragma once

#include "PathTracer.h"

class camera {
	public:
		camera(vec3 position, vec3 lookat, vec3 up, float vfov, float aspect_ratio, float aperture, float focus_distance) {
			float theta = degrees_to_radians(vfov);
			float h = tan(theta / 2.f);
			float viewport_height = 2.f * h;
			float viewport_width = viewport_height * aspect_ratio;

			w = normalize(position - lookat);
			u = normalize(cross(up, w));
			v = cross(w, u);

			origin = position;
			horizontal = focus_distance * viewport_width * u;
			vertical = focus_distance * viewport_height * v;
			lower_left_corner = origin - horizontal / 2.f - vertical / 2.f - focus_distance * w;

			lens_radius = aperture / 2;
		}

		ray get_ray(float s, float t) const {
			vec3 rand = lens_radius * random_vec3_in_unit_disk();
			vec3 offset = u * rand.x + v * rand.y;
			
			return ray(origin + offset, (lower_left_corner + s * horizontal + t * vertical) - (origin + offset));
		}

	private:
		vec3 origin;
		vec3 lower_left_corner;
		vec3 horizontal;
		vec3 vertical;
		vec3 u, v, w;
		float lens_radius;
};