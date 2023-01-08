#pragma once

#include "PathTracer.h"
#include "hittable.h"

struct hit_record;

class material {
	public:
		virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& r_out) const = 0;
};

class lambertian : public material {
	public:
		lambertian(const vec3& a) : albedo(a) {}

		virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& r_out) const override {
			vec3 scatter_direction = rec.normal + normalize(random_vec3());

			// Catch degenerate scatter directions
			if (is_near_zero(scatter_direction)) {
				scatter_direction = rec.normal;
			}

			r_out = ray(rec.p, scatter_direction);
			attenuation = albedo;
			return true;
		}

	public:
		vec3 albedo;
};

class metal : public material {
	public:
		metal(const vec3& a, float f) : albedo(a), roughness(f < 1 ? f : 1) {}

		virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& r_out) const override {
			vec3 reflected = reflect(normalize(r_in.direction()), rec.normal);

			r_out = ray(rec.p, reflected + roughness * random_vec3_in_unit_sphere());
			attenuation = albedo;
			return dot(r_out.direction(), rec.normal) > 0;
		}

	public:
		vec3 albedo;
		float roughness;
};

class dielectric : public material {
	public:
		dielectric(float index_of_refraction) : ior(index_of_refraction) {}

		virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& r_out) const override {
			attenuation = vec3(1.0, 1.0, 1.0);
			float ior_ratio = rec.front_face ? (1.f / ior) : ior;

			vec3 unit_direction = normalize(r_in.direction());
			float cos_theta = fmin(dot(-unit_direction, rec.normal), 1.f);
			float sin_theta = sqrt(1.f - cos_theta * cos_theta);

			bool total_internal_reflection = ior_ratio * sin_theta > 1.0;
			vec3 direction;

			if (total_internal_reflection || reflectance(cos_theta, ior_ratio) > random_float()) {
				direction = reflect(unit_direction, rec.normal);
			}
			else {
				direction = refract(unit_direction, rec.normal, ior_ratio);
			}

			//
			//direction = refract(unit_direction, rec.normal, 1.f);
			//

			r_out = ray(rec.p, direction);
			return true;
		}

	public:
		float ior;

	private:
		static float reflectance(float cosine, float ior_ratio) {
			// Schlick's approximation
			float r0 = (1.f - ior_ratio) / (1.f + ior_ratio);
			r0 = r0 * r0;
			return r0 + (1.f - r0) * pow((1.f - cosine), 5.f);
		}
};

class normal : public material {
	public:
		normal() {}

		virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& r_out) const override {
			vec3 scatter_direction = rec.normal + normalize(random_vec3());

			// Catch degenerate scatter directions
			if (is_near_zero(scatter_direction)) {
				scatter_direction = rec.normal;
			}

			r_out = ray(rec.p, scatter_direction);
			
			if (rec.front_face) {
				attenuation = (standard_unit_vector + rec.normal) / 2.f;
			}
			else {
				attenuation = (standard_unit_vector - rec.normal) / 2.f;
			}

			return true;
		}

	private:
		const vec3 standard_unit_vector = normalize(vec3(1, 1, 1));
};