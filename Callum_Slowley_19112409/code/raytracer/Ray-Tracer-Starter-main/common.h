#pragma once

#include <cmath>
#include <limits>
#include <memory>
#include <cstdlib>



//using
using std::shared_ptr;
using std::make_shared;
using std::sqrt;

//constrants
const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

//Utility Function
inline double degrees_to_radians(double degrees) {
	return degrees * pi / 180;
}

// added clamp to common for the textures
inline double clamp(double x, double min, double max) {
	if (x < min) { return min; }
	if (x > max) { return max; }
	return x;
}

inline double random_double() {
	//returns a radom real in [0,1]
	return rand() / (RAND_MAX + 1.0);
}


inline double random_double(double min, double max) {
	//return a random real in [min,man]
	return min + (max - min) * random_double();
}

inline int random_int(int min, int max) {
	return static_cast<int>(random_double(min, max + 1));
}
//common headers
#include "Ray.h"
#include "geometry.h"