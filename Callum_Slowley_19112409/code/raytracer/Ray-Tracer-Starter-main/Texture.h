/* code in this file is from Peter Shirley https://raytracing.github.io/books/RayTracingTheNextWeek.html#solidtextures/thefirsttextureclass:constanttexture */
#pragma once
#include "common.h"
#include <iostream>
#include <math.h>
#include "stb-master/stb_image.h"

class Texture {
public:
	virtual Colour colour_Value(double u, double v, const Point3f& p) const = 0;
};

class solid_colour :public Texture {
public:
	solid_colour() {}
	solid_colour(Colour c) :colour_value(c) {}

	solid_colour(double red, double green, double blue) : solid_colour(Colour(red, green, blue)) {}

	virtual Colour colour_Value(double u, double v, const Vec3f& p) const override {
		return colour_value;
	}
private:
	Colour colour_value;
};

class image_texture :public Texture {
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

	~image_texture() {
		delete data;
	}

	virtual Colour colour_Value(double u, double v, const Vec3f& p) const override {
		//if no texture is loaded cyan is used to debug
		if (data == nullptr) { return Colour(0, 1, 1); }

		//clamp text coords
		u = clamp(u, 0.0, 1.0);
		v = 1.0 - clamp(v, 0.0, 1.0); //flipped v to image cords

		auto i = static_cast<int>(u * width);
		auto j = static_cast<int>(v * height);

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