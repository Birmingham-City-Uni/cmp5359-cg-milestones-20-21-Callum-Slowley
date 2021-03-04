#include <vector>
#include <cmath>
#include <iostream>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const int width = 1080;
const int height = 1080;

//global model used for shading of triangle meshs
Model* model = nullptr;

void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color) {
	bool steep = false;
	if (std::abs(p0.x - p1.x) < std::abs(p0.y - p1.y)) {
		std::swap(p0.x, p0.y);
		std::swap(p1.x, p1.y);
		steep = true;
	}
	if (p0.x > p1.x) {
		std::swap(p0, p1);
	}

	for (int x = p0.x; x <= p1.x; x++) {
		float t = (x - p0.x) / (float)(p1.x - p0.x);
		int y = p0.y*(1. - t) + p1.y*t;
		if (steep) {
			image.set(y, x, color);
		}
		else {
			image.set(x, y, color);
		}
	}
}

//Go over the Barycentric coords next time

void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) {
	if (t0.y == t1.y && t0.y == t2.y) return;
	//Drwaing the triangle between A and B. A being the red line and B being the green
	if (t0.y > t1.y) { std::swap(t0, t1); }
	if (t0.y > t2.y) { std::swap(t0, t2); }
	if (t1.y > t2.y) { std::swap(t1, t2); }
	float total_height = t2.y - t0.y;
	//cutting triangle at y axis (Appears dotted for now as we have a constant gradiant)
	//lower green line
	for (int y = t0.y; y <= t1.y; y++) {
		 float segment_height = t1.y - t0.y + 1;
		 float alpha = (y - t0.y) / total_height;
		 float beta = (y - t0.y) / segment_height;
		 Vec2i A = t0 + (t2 - t0) * alpha;
		 Vec2i B = t0 + (t1 - t0) * beta;
		 //A has to be on the left
		 if (A.x > B.x) std::swap(A, B);

		 ////replaced by nested for 
		 //image.set(A.x, y, red);
		 //image.set(B.x, y, green);

		 //nested loop used for each pixel for x values
		 for (int j = A.x; j <= B.x; j++) {
			 image.set(j, y, color);
		 }
	}
	//Upper green line
	for (int y = t1.y; y <= t2.y; y++) {
		float segment_height = t2.y - t1.y + 1;
		float alpha = (y - t0.y) / total_height;
		float beta = (y - t1.y) / segment_height;
		Vec2i A = t0 + (t2 - t0) * alpha;
		Vec2i B = t1 + (t2 - t1) * beta;
		//A has to be on the left
		if (A.x > B.x) std::swap(A, B);

		////replaced by nested for 
		//image.set(A.x, y, red);
		//image.set(B.x, y, green);

		//nested loop used for each pixel for x values
		for (int j = A.x; j <= B.x; j++) {
			image.set(j, y, color);
		}
	}
	////No longer needed as we are drawing the line by partisoning the lines so we can draw the triangle fill
	//line(t0, t1, image, green);
	//line(t1, t2, image, green);
	//line(t2, t0, image, red);
}

int main(int argc, char** argv) {
	TGAImage image(width, height, TGAImage::RGB);

	Vec2i t0[3] = { Vec2i(650, 1000),   Vec2i(530, 1066),  Vec2i(968, 901) };
	Vec2i t1[3] = { Vec2i(90, 680),  Vec2i(42, 750),   Vec2i(895, 627) };
	Vec2i t2[3] = { Vec2i(360, 380), Vec2i(1050, 160), Vec2i(1079, 256) };

	////not used for meshes
	//triangle(t0[0], t0[1], t0[2], image, red);
	//triangle(t1[0], t1[1], t1[2], image, white);
	//triangle(t2[0], t2[1], t2[2], image, green);

	if (2 == argc) {
		model = new Model(argv[1]);
	}
	else {
		model = new Model("obj/cc.obj");
	}

	////used to get random colors.
	//for (int i = 0; i < model->nfaces(); i++) {
	//	std::vector<int> face = model->face(i);
	//	Vec2i screen_coords[3];
	//	for (int j = 0; j < 3; j++) {
	//		Vec3f world_coords = model->vert(face[j]);
	//		screen_coords[j] = Vec2i((world_coords.x + 1) * width / 2, (world_coords.y + 1.0f) * height / 2.0f);
	//	}
	//	triangle(screen_coords[0], screen_coords[1], screen_coords[2], image, TGAColor(rand() % 255, rand() % 255, rand() % 255, 255));
	//}

	//shaded
	Vec3f light_dir(0, 0, -1);
	for (int i = 0; i < model->nfaces(); i++) {
		std::vector<int> face = model->face(i);
		Vec2i screen_coords[3];
		Vec3f wordl_cords[3];
		for (int j = 0; j < 3; j++) {
			Vec3f v = model->vert(face[j]);
			screen_coords[j]= Vec2i((v.x + 1) * width / 2, (v.y + 1.0f) * height / 2.0f);
			wordl_cords[j] = v;
		}
		Vec3f n = (wordl_cords[2] - wordl_cords[0])^(wordl_cords[1] - wordl_cords[0]);
		n.normalize();
		float intensity = n * light_dir;
		if (intensity > 0) {
			triangle(screen_coords[0], screen_coords[1], screen_coords[2], image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
		}
	}

	image.flip_vertically(); // Origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	return 0;
}

