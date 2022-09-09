#pragma once

#include "PathTracer.h"
#include "hittable.h"

struct hit_record;

class material {
	public:
		virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& r_out) const = 0;
};

class lambertian : public material {
	public:
		lambertian(const color& a) : albedo(a) {}

		virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& r_out) const override {
			vec3 scatter_direction = rec.normal + random_unit_vector();

			// Catch degenerate scatter directions
			if (scatter_direction.near_zero()) {
				scatter_direction = rec.normal;
			}

			r_out = ray(rec.p, scatter_direction);
			attenuation = albedo;
			return true;
		}

	public:
		color albedo;
};

class metal : public material {
	public:
		metal(const color& a, double f) : albedo(a), roughness(f < 1 ? f : 1) {}

		virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& r_out) const override {
			vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);

			r_out = ray(rec.p, reflected + roughness * random_in_unit_sphere());
			attenuation = albedo;
			return dot(r_out.direction(), rec.normal) > 0;
		}

	public:
		color albedo;
		double roughness;
};