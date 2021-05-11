#pragma once
#include "geometry.h"
#include "SDL.h" 
#include <fstream>
#include <chrono>

class Ray {
public:
    Ray() {};
    Ray(const Point3f& origin, const Vec3f& direction) : o(origin), d(direction) {}

    Point3f origin() const { return o; }
    Vec3f direction() const { return d; }

    Point3f at(double t) const {
        return o + t * d;
    }
public:
    Point3f o;
    Vec3f d;
};