#pragma once
#include "Ray.h"



struct hit_record {
	Point3f p;
	Vec3f normal;
	double t;
	bool front_face;



	inline void set_face_normal(const Ray& r, const Vec3f& outward_normal) {
		front_face = (r.direction().dotProduct(outward_normal)) < 0;
		normal = front_face ? outward_normal : -outward_normal;
	}
};



class hittable {
public:
	// pure virtual, must be overriden in derived classes
	virtual bool hit(const Ray& r, double t_min, double t_max, hit_record& rec) const = 0;
};