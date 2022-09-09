#pragma once

#include "hittable.h"
#include "vec3.h"

class sphere : public hittable {
	public:
		sphere() {}
		sphere(point3 cen, double r, shared_ptr<material> m) : center(cen), radius(r), mat_ptr(m) {}

		virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;

	public:
		point3 center;
		double radius;
		shared_ptr<material> mat_ptr;
};

bool sphere::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
	vec3 co = r.origin() - center;

	double a = r.direction().length_squared();
	double half_b = dot(co, r.direction());
	double c = co.length_squared() - radius * radius;

	double discriminant = half_b * half_b - a * c;
	if (discriminant < 0) return false;
	double sqrtd = sqrt(discriminant);

	// Find the nearest root within the range [t_min, t_max]
	double root = (-half_b - sqrtd) / a;
	if (root < t_min || t_max < root) {
		root = (-half_b + sqrtd) / a;
		if (root < t_min || t_max < root) {
			return false;
		}
	}

	rec.t = root;
	rec.p = r.at(rec.t);
	vec3 outward_normal = (rec.p - center) / radius;
	rec.set_face_normal(r, outward_normal);
	rec.mat_ptr = mat_ptr;

	return true;
}