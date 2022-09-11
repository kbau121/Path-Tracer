#pragma once

#include "PathTracer.h"

class camera {
	public:
		camera(point3 position, point3 lookat, vec3 up, double vfov, double aspect_ratio, double aperture, double focus_distance) {
			double theta = degrees_to_radians(vfov);
			double h = tan(theta / 2);
			double viewport_height = 2.0 * h;
			double viewport_width = viewport_height * aspect_ratio;

			w = unit_vector(position - lookat);
			u = unit_vector(cross(up, w));
			v = cross(w, u);

			origin = position;
			horizontal = focus_distance * viewport_width * u;
			vertical = focus_distance * viewport_height * v;
			lower_left_corner = origin - horizontal / 2 - vertical / 2 - focus_distance * w;

			lens_radius = aperture / 2;
		}

		ray get_ray(double s, double t) const {
			vec3 rand = lens_radius * random_in_unit_disk();
			vec3 offset = u * rand.x() + v * rand.y();
			
			return ray(origin + offset, (lower_left_corner + s * horizontal + t * vertical) - (origin + offset));
		}

	private:
		point3 origin;
		point3 lower_left_corner;
		vec3 horizontal;
		vec3 vertical;
		vec3 u, v, w;
		double lens_radius;
};