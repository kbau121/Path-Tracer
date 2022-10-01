#pragma once

#include "hittable.h"
#include "glm/glm.hpp"
#include "glm/gtx/norm.hpp"

class sphere : public hittable {
	public:
		sphere() {}
		sphere(glm::vec3 cen, float r, shared_ptr<material> m) : center(cen), radius(r), mat_ptr(m) {}

		virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const override;

	public:
		glm::vec3 center;
		float radius;
		shared_ptr<material> mat_ptr;
};

bool sphere::hit(const ray& r, float t_min, float t_max, hit_record& rec) const {
	glm::vec3 co = r.origin() - center;

	float a = glm::length2(r.direction());
	float half_b = dot(co, r.direction());
	float c = glm::length2(co) - radius * radius;

	float discriminant = half_b * half_b - a * c;
	if (discriminant < 0) return false;
	float sqrtd = sqrt(discriminant);

	// Find the nearest root within the range [t_min, t_max]
	float root = (-half_b - sqrtd) / a;
	float dist = root * r.direction().length();
	if (dist < t_min || t_max < dist) {
		root = (-half_b + sqrtd) / a;
		dist = root * r.direction().length();
		if (dist < t_min || t_max < dist) {
			return false;
		}
	}

	rec.t = dist;
	rec.p = r.at(root);
	glm::vec3 outward_normal = (rec.p - center) / radius;
	rec.set_face_normal(r, outward_normal);
	rec.mat_ptr = mat_ptr;

	return true;
}