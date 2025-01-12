#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "tgaimage.h"
#include "dxvectors.h"

class Model
{
private:
	// faces. structure: vertex/uv/normal
	std::vector<std::vector<Vector3i>> faces_;
	// vertices
	std::vector<Vector3f> verts_;
	// normals
	std::vector<Vector3f> norms_;
	// uvs
	std::vector<Vector2f> uv_;
	TGAImage diffusemap_;
	TGAImage normalmap_;
	TGAImage specularmap_;
	// loads texture FILENAME_diffuse.tga to img
	void load_texture(std::string filename, const char* suffix, TGAImage& img);
public:
	Model(const char* filename);
	~Model();
	// get amount of vertices
	int nverts();
	// get amount of faces
	int nfaces();
	// get normal to a specific vertex of a face
	Vector3f normal(int iface, int nthvert);
	// get normal to uv
	Vector3f normal(Vector2f uv);
	// get vertex by id
	Vector3f vert(int i);
	// get specific vertex of a face
	Vector3f vert(int iface, int nthvert);
	// get uv of a specific vertex of a face
	Vector2f uv(int iface, int nthvert);
	// get color at the specified uv point
	TGAColor diffuse(Vector2f uv);
	// get diffuse texture
	TGAImage diffusemap();
	// get normal texture
	TGAImage normalmap();
	// get specular texture
	TGAImage specularmap();
	// get specular at the specified uv point
	float specular(Vector2f uv);
	// get face by id
	std::vector<int> face(int idx);
};

#endif //__MODEL_H__