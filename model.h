#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "SimpleMath.h"
typedef DirectX::XMINT2 Vector2i; // simple math supports only XMFLOAT. probably interchangeable

class Model {
private:
	std::vector<DirectX::SimpleMath::Vector3> verts_;
	std::vector<std::vector<int>> faces_;
public:
	Model(const char* filename);
	~Model();
	int nverts();
	int nfaces();
	DirectX::SimpleMath::Vector3 vert(int i);
	std::vector<int> face(int idx);
};

#endif //__MODEL_H__