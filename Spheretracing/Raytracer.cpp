#include <fstream>
#include <cmath>
#include"Raytracer.h"

void clamp255(Vec3& col) {
	col.x = (col.x > 255) ? 255 : (col.x < 0) ? 0 : col.x;
	col.y = (col.y > 255) ? 255 : (col.y < 0) ? 0 : col.y;
	col.z = (col.z > 255) ? 255 : (col.z < 0) ? 0 : col.z;
}

int main() {
	//size of the image
	const int H = 500;
	const int W = 500;

	//by adding more colours we can change what our sphere looks like
	const Vec3 white(255, 255, 255);
	const Vec3 black(0, 0, 0);
	const Vec3 red(255, 0, 0);
	const Vec3 green(0, 255, 0);
	const Vec3 blue(0, 0, 255);
	
	//creating the sphere thats being rendered

	double Sr = 100; // radius of the sphere, this controls its size

	float Sx, Sy, Sz; //these are the spheres x y and z.

	Sx = 0.75; //x and y are related to the screen size so the sphere doesnt appear off screen

	Sy = 0.25; 

	Sz = 50;

	//creating my sphere's pos relitive to the image size
	Vec3 Spherepos (W * Sx, H *Sy, Sz);

	const Sphere sphere(Spherepos, Sr);

	//creating the light
	double Lr = 100; //lights radius

	float Lx, Ly, Lz; // the lights x, y and z

	Lx = 0;

	Ly = 0; // where positive y goes down

	Lz = 50;

	Vec3 Lightpos(Lx, Ly, Lz);

	const Sphere light(Lightpos, Lr);

	std::ofstream out("out.ppm");
	out << "P3\n" << W << ' ' << H << ' ' << "255\n";

	double t;
	Vec3 pix_col(black);

	for (int y = 0; y < H; y++) {
		for (int x = 0; x < W; x++) {
			pix_col = black;
			
			const ray ray(Vec3(x, y, 0), Vec3(0, 0, 1));
			if (sphere.intersect(ray, t)) {
				const Vec3 pi = ray.o + ray.d * t;
				const Vec3 L = light.c - pi;
				const Vec3 N = sphere.getNormal(pi);
				const double dt = dot(L.normalize(), N.normalize());

				//change the first color here to change the color of the sphere, seccond color is the color of the light bouncing off
				pix_col = (green + white * dt) * 0.5;

				clamp255(pix_col);
			}
			out << (int)pix_col.x << ' '
				<< (int)pix_col.y << ' '
				<< (int)pix_col.z << '\n';
		}
	}
}