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

class dielectric : public material {
	public:
		dielectric(double index_of_refraction) : ior(index_of_refraction) {}

		virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& r_out) const override {
			attenuation = color(1.0, 1.0, 1.0);
			double ior_ratio = rec.front_face ? (1.0 / ior) : ior;

			vec3 unit_direction = unit_vector(r_in.direction());
			vec3 refracted = refract(unit_direction, rec.normal, ior_ratio);

			r_out = ray(rec.p, refracted);
			return true;
		}

	public:
		double ior;
};