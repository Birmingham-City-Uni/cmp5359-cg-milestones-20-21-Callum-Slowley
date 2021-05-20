#pragma once

#include "geometry.h"
#include <chrono>

class VertexShader {
public:
	VertexShader() {
		t_start = std::chrono::high_resolution_clock::now();
	}
	void processVertex(Vec3f in, Vec3f* out) {
		auto passedTime = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - t_start).count();
		out->x = in.x + sinf(in.y*4+passedTime)/4;
		out->y = in.y;
		out->z = in.z;

	}
public:
	std::chrono::steady_clock::time_point t_start;
};