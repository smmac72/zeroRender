#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "tgaimage.h"
#include "dxvectors.h"

class Model {
private:
	std::vector<Vector3f> verts_;
	std::vector<std::vector<Vector3i>> faces_; // vertex/uv/normal
	std::vector<Vector3f> norms_;
	std::vector<Vector2f> uv_;
	TGAImage diffusemap_;
	void load_texture(std::string filename, const char* suffix, TGAImage& img);
public:
	Model(const char* filename);
	~Model();
	int nverts();
	int nfaces();
	Vector3f vert(int i);
	Vector2f uv(int iface, int nvert);
	TGAColor diffuse(Vector2i uv);
	std::vector<int> face(int idx);
};

#endif //__MODEL_H__