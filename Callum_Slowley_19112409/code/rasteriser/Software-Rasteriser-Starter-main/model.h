#pragma once

#include <vector>
#include "Texture.h"
#include "geometry.h"

class Model {
private:
	std::vector<Vec3f> verts_;              // Stores Vec3f for every model vertex world position
	std::vector<std::vector<int> > faces_;  // Stores a vector of vector<int> that represent indices in verts_ for vertices comprising a face
	std::vector<Vec2f> vts_;				// Stores Vec3f for every model vertex texture coordinate
	std::vector<Vec3f> vns_;
	std::vector<std::vector<int> > vnorms_;
	std::vector<std::vector<int> > uvs_;

public:
	Model(const char *filename);
	~Model();

	void material(const char* filename){
		textureMaterial = image_texture(filename);
	}
	int nverts();
	int nfaces();
	Vec3f vert(int i);
	Vec2f vt(int i);
	Vec3f vnorms(int i);
	std::vector<int> face(int idx);
	std::vector<int> vNorms(int idx);
	std::vector<int> uvs(int idx);

	image_texture textureMaterial;
};

