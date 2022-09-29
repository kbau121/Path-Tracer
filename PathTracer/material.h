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
			double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
			double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

			bool total_internal_reflection = ior_ratio * sin_theta > 1.0;
			vec3 direction;

			if (total_internal_reflection || reflectance(cos_theta, ior_ratio) > random_double()) {
				direction = reflect(unit_direction, rec.normal);
			}
			else {
				direction = refract(unit_direction, rec.normal, ior_ratio);
			}

			r_out = ray(rec.p, direction);
			return true;
		}

	public:
		double ior;

	private:
		static double reflectance(double cosine, double ior_ratio) {
			// Schlick's approximation
			double r0 = (1.0 - ior_ratio) / (1.0 + ior_ratio);
			r0 = r0 * r0;
			return r0 + (1 - r0) * pow((1.0 - cosine), 5.0);
		}
};

class normal : public material {
	public:
		normal() {}

		virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& r_out) const override {
			vec3 scatter_direction = rec.normal + random_unit_vector();

			// Catch degenerate scatter directions
			if (scatter_direction.near_zero()) {
				scatter_direction = rec.normal;
			}

			r_out = ray(rec.p, scatter_direction);
			
			if (rec.front_face) {
				attenuation = (standard_unit_vector + rec.normal) / 2;
			}
			else {
				attenuation = (standard_unit_vector - rec.normal) / 2;
			}

			return true;
		}

	private:
		const vec3 standard_unit_vector = unit_vector(vec3(1, 1, 1));
};