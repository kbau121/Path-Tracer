#pragma once

#include "hittable.h"
#include "glm/glm.hpp"

using glm::vec3;

class triangle : public hittable {
	public:
		triangle() {}
		triangle(vec3 p0, vec3 p1, vec3 p2, shared_ptr<material> m) : p{ p0, p1, p2 }, mat_ptr(m) {
			vec3 e1 = p[1] - p[0];
			vec3 e2 = p[2] - p[0];

			vec3 _n = normalize(cross(e1, e2));
			n[0] = _n;
			n[1] = _n;
			n[2] = _n;
		}

		triangle(vec3 p0, vec3 p1, vec3 p2, vec3 n0, vec3 n1, vec3 n2, shared_ptr<material>m) : p{ p0, p1, p2 }, n{ normalize(n0), normalize(n1), normalize(n2) }, mat_ptr(m) {}

		virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const override;

	public:
		vec3 p[3];
		vec3 n[3];
		shared_ptr<material> mat_ptr;
};

bool triangle::hit(const ray& r, float t_min, float t_max, hit_record& rec) const {
	vec3 d = normalize(r.direction());

	vec3 e1 = p[1] - p[0];
	vec3 e2 = p[2] - p[0];
	vec3 T = r.origin() - p[0];
	vec3 P = cross(d, e2);
	vec3 Q = cross(T, e1);
	float Pe1 = dot(P, e1);

	if (Pe1 == 0) {
		return false;
	}

	vec3 out = vec3(dot(Q, e2), dot(P, T), dot(Q, d)) / Pe1;

	if (out.y < 0 || out.z < 0 || out.y + out.z > 1) {
		return false;
	}

	if (out.x < t_min || t_max < out.x) {
		return false;
	}

	rec.t = out.x;
	rec.p = r.at(rec.t / r.direction().length());

	vec3 outward_normal =
		n[0] * (1 - out.y - out.z) +
		n[1] * out.y +
		n[2] * out.z;

	rec.set_face_normal(r, normalize(outward_normal));
	rec.mat_ptr = mat_ptr;

	return true;
}