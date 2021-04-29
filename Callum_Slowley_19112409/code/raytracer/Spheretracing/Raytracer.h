#pragma once
#include <math.h> 
#include <fstream>
#include <cmath>

struct Vec3 {
	double x, y, z;
	Vec3(double x, double y, double z) :x(x), y(y), z(z) {}
	Vec3 operator + (const Vec3& v) const {
		return Vec3(x + v.x, y + v.y, z + v.z);
	}
	Vec3 operator - (const Vec3& v) const {
		return Vec3(x - v.x, y - v.y, z - v.z);
	}
	Vec3 operator * (double d)const {
		return Vec3(x * d, y * d, z * d);
	}
	Vec3 operator / (double d) const {
		return Vec3(x / d, y / d, z / d);
	}
	Vec3 normalize() const {
		double mg = sqrt(x * x + y * y + z * z);
		return Vec3(x / mg, y / mg, z / mg);
	}
};

//dot product of the 2 vectors
inline double dot(const Vec3& a, const Vec3& b) {
	return (a.x * b.x + a.y * b.y + a.z * b.z);
}

//the ray P(t)=)+tD
struct ray{
	Vec3 o, d;
	ray(const Vec3& o, const Vec3& d) : o(o), d(d) {}
};

//creating the spehere and working out the points of intersection
struct Sphere {
	Vec3 c;
	double r;
	Sphere(const Vec3& c, double r) :c(c), r(r) {}
	Vec3 getNormal(const Vec3& pi) const {
		return(pi - c) / r;
	}
	//now we solve for t  using t^2*d.d+2*t*(o-p).d+(o-p).(o-p)-R^2=0
	bool intersect(const ray& ray, double& t)const {
		const Vec3 o = ray.o;
		const Vec3 d = ray.d;
		const Vec3 oc = o - c;
		const double b = 2 * dot(oc, d);
		const double c = dot(oc, oc) - r * r;
		double disc = b * b - 4 * c; // a is left out as we are using a normalised ray so a would be 1
		//ray misses the sphere
		if (disc < 1e-4) {
			return false;
		}
		//ray hits sphere
		disc = sqrt(disc);
		//2 values of t due to using the quad equation
		const double t0 = -b - disc;
		const double t1 = -b + disc;
		//as we have 2 intersections we are grabbing the shortes(Closest on to the camera)
		t = (t0 < t1) ? t0 : t1;
		return true;
	}
};

//creating the plane and working out points of intersection 
struct plane {
	Vec3 c;
	double w;
	double h;
	plane(const Vec3& c, double w, double h) :c(c), w(w),h(h) {}
	Vec3 getNormal(const Vec3& pi) const {
		return(pi - c); // dont need to devide to normize it 
	}

	bool intersect(const ray& ray, double& t, const Vec3& pi)const {
		Vec3 N = getNormal(pi);
		Vec3 n;
		Vec3 Po;
		const Vec3 o = ray.o;
		const Vec3 d = ray.d;
		const Vec3 a = o - c;
	
		t = (dot((Po - a), N)) / dot(n, N);
	}
};
