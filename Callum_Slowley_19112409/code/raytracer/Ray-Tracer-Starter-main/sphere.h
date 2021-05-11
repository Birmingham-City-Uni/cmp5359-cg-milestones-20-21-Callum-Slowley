#pragma once


#include "hittable.h"
#include "geometry.h"

class sphere : public hittable {
public:
	sphere() {}
	sphere(Point3f cen, double r) : centre(cen), radius(r) {};

	virtual bool hit(
		const Ray& r, double t_min, double t_max, hit_record& rec) const override;

public:
	Point3f centre;
	double radius;
};

bool sphere::hit(const Ray& r, double t_min, double t_max, hit_record& rec) const {
	Vec3f oc = r.origin() - centre;
	auto a = r.direction().norm();
	auto half_b = oc.dotProduct(r.direction());
	auto c = oc.norm() - radius * radius;
	auto discriminant = half_b * half_b - a * c;
	if (discriminant < 0) return false;
	auto sqrtd = sqrt(discriminant);

	// Find the nearest root that lies in the acceptable range.
	auto root = (-half_b - sqrtd) / a;
	if (root < t_min || t_max < root) {
		root = (-half_b + sqrtd) / a;
		if (root < t_min || t_max < root)
			return false;
	}

	// Update hit record data accordingly
	rec.t = root;
	rec.p = r.at(rec.t);
	Vec3f outward_normal = (rec.p - centre) / radius;
	rec.set_face_normal(r, outward_normal);

	return true;
}