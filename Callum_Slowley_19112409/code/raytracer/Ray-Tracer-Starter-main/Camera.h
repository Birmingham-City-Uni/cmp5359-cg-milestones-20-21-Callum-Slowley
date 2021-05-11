#pragma once

#include "common.h"

class camera {
public:
	camera(double vfov, double aspect_ratio) {
		auto theta = degrees_to_radians(vfov);
		auto h = tan(theta / 2);
		auto viewport_height = 2.0*h;
		auto viewport_width = aspect_ratio * viewport_height;
		auto focal_length = 1.0;

		origin = Point3f(0, 0, 0);
		horizonal = Vec3f(viewport_width, 0.0, 0.0);
		vertical = Vec3f(0.0, viewport_height, 0.0);
		lower_left_corner = origin - horizonal / 2 - vertical / 2 - Vec3f(0, 0, focal_length);
	}
	Ray get_ray(double u, double v) const {
		return Ray(origin, lower_left_corner + u * horizonal + v * vertical - origin);
	}
private:
	Point3f origin;
	Point3f horizonal;
	Vec3f lower_left_corner;
	Vec3f vertical;
};