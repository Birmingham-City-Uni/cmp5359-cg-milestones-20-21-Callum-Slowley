#pragma once
/* code in this file is from Peter Shirley https://raytracing.github.io/books/RayTracingTheNextWeek.html#solidtextures/thefirsttextureclass:constanttexture */
#pragma once
#include "geometry.h"
#include <iostream>
#include <math.h>
#include "stb-master/stb_image.h"

// added clamp to common for the textures
inline double clamp(double x, double min, double max) {
	if (x < min) { return min; }
	if (x > max) { return max; }
	return x;
}


class image_texture  {
public:
	//bytes per pixel
	const static int bpp = 3;

	image_texture() :data(nullptr), width(0), height(0), bytes_per_scanline(0) {}

	image_texture(const char* filename) {
		auto components_per_pixel = bpp;
		data = stbi_load(filename, &width, &height, &components_per_pixel, components_per_pixel);

		if (!data) {
			std::cerr << "ERROR: could not load texture image file: " << filename << ".\n";
		}

		bytes_per_scanline = bpp * width;
	}

	 Colour colour_Value(Vec2f uv) {
		//if no texture is loaded gray is used to debug
		if (data == nullptr) { return Colour(.8, .8, .8); }

		//clamp text coords
		uv.x = clamp(uv.x, 0.0, 1.0);
		uv.y = 1.0 - clamp(uv.y, 0.0, 1.0); //flipped v to image cords

		auto i = static_cast<int>(uv.x * width);
		auto j = static_cast<int>(uv.y * height);

		//clamp interger mapping 
		if (i >= width) { i = width - 1; }
		if (j >= height) { j = height - 1; }

		const auto colour_scale = 1.0 / 255.0;
		auto pixel = data + j * bytes_per_scanline + i * bpp;

		return Colour(colour_scale * pixel[0], colour_scale * pixel[1], colour_scale * pixel[2]);
	}
private:
	unsigned char* data;
	int width, height;
	int bytes_per_scanline;
};
