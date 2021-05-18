#pragma once
#include "hittable.h"
#include "geometry.h"
#include "model.h"


class triangle :public hittable {
public:
	triangle() {}
	triangle(Point3f vert0, Point3f vert1, Point3f vert2, Vec3f vertNormal0, Vec3f vertNormal1, Vec3f vertNormal2, Vec2f UVx, Vec2f UVy, shared_ptr<material> m) :
		v0(vert0), v1(vert1), v2(vert2),v0n(vertNormal0), v1n(vertNormal1),  v2n(vertNormal2), uvx(UVx),uvy(UVy),mat_ptr(m) {
		normal = (v1 - v0).crossProduct(v2 - v0);
	};

	triangle(Point3f vert0, Point3f vert1, Point3f vert2, Vec3f vn, shared_ptr<material> m) :v0(vert0), v1(vert1), v2(vert2), normal(vn), mat_ptr(m) {};

	virtual bool hit(const Ray& r, double t_min, double t_max, hit_record& rec) const override;

	virtual bool bounding_box(aabb& output_box) const override;

public:
	Point3f v0, v1, v2;
	Vec3f normal,v0n,v1n,v2n;
	Vec2f uvx, uvy;
	shared_ptr<material> mat_ptr;
};

bool triangle::hit(const Ray& r, double t_min, double t_max, hit_record& rec) const {
	float thit, t, u, v;

	Vec3<float> v0v1 = v1 - v0;

	Vec3<float> v0v2 = v2 - v0;
	
	Vec3<float> pvec = r.direction().crossProduct(v0v2);

	float det = pvec.dotProduct(v0v1);
	float kEpsilon = 0.00001;

	if (det < kEpsilon)return false;
	
	float invDet = 1 / det;

	Vec3<float> tvec = r.origin() - v0;
	u = tvec.dotProduct(pvec) * invDet;

	if (u < 0 || u>1)return false;

	Vec3<float>gvec = tvec.crossProduct(v0v1);
	v = r.direction().dotProduct(gvec) * invDet;
	if (v < 0 || u + v>1) return false;

	t = v0v2.dotProduct(gvec) * invDet;

	if (t < 0) return false;

	rec.p = r.at(t);
	rec.t = t;
	//needed to load texture.
	rec.u = uvx.x;
	rec.v = uvy.y;
	 //fix normal calacutaions
	rec.normal = this->v1n * u + this->v2n * v + this->v0n * (1.0f - u - v);

	rec.mat_ptr = mat_ptr;
	
	return true;
}

inline bool triangle::bounding_box(aabb& output_box)const {
	float min[3];
	float max[3];
	for (int i = 0.; i < 3; i++) { // for each dimension calculating the min and max values of vertices in the triangle
		min[i] = std::min(v0[i], std::min(v1[i], v2[i]));
		max[i] = std::max(v0[i], std::max(v1[i], v2[i]));
	}
	output_box = aabb(Vec3f(min[0], min[1], min[2]), Vec3f(max[0], max[1], max[2]));
	return true;
}