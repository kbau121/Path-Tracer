#pragma once

#include "PathTracer.h"

class camera {
	public:
		camera(point3 position, point3 lookat, vec3 up, double vfov, double aspect_ratio) {
			double theta = degrees_to_radians(vfov);
			double h = tan(theta / 2);
			double viewport_height = 2.0 * h;
			double viewport_width = viewport_height * aspect_ratio;

			vec3 w = unit_vector(position - lookat);
			vec3 u = unit_vector(cross(up, w));
			vec3 v = cross(w, u);

			origin = position;
			horizontal = viewport_width * u;
			vertical = viewport_height * v;
			lower_left_corner = origin - horizontal / 2 - vertical / 2 - w;
		}

		ray get_ray(double u, double v) const {
			return ray(origin, lower_left_corner + u * horizontal + v * vertical - origin);
		}

	private:
		point3 origin;
		point3 lower_left_corner;
		vec3 horizontal;
		vec3 vertical;
};