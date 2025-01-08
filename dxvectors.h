#ifndef __DXVECTORS_H__
#define __DXVECTORS_H__
#include <d3d12.h>
#include "SimpleMath.h"
#include "tgaimage.h"
#include <algorithm>
typedef DirectX::SimpleMath::Vector3 Vector3f;
typedef DirectX::SimpleMath::Vector2 Vector2f;
typedef DirectX::XMINT2 Vector2i; // simplemath supports only XMFLOAT. probably interchangeable
typedef DirectX::XMINT3 Vector3i;
typedef DirectX::SimpleMath::Matrix Matrix;

Vector2i operator+(Vector2i a, Vector2i b);
Vector2i operator-(Vector2i a, Vector2i b);
Vector2i operator*(Vector2i vec, float alpha);
Vector3i operator+(Vector3i a, Vector3i b);
Vector3i operator-(Vector3i a, Vector3i b);
Vector3i operator*(Vector3i vec, float alpha); Vector3f operator+(Vector3i a, Vector3f b);
TGAColor operator*(TGAColor color, float alpha);
DirectX::SimpleMath::Vector4 embed(Vector3f vec);
DirectX::SimpleMath::Vector3 embed(Vector2f vec);
DirectX::SimpleMath::Vector4 GetFirstColumn(Matrix m);

Vector3i FloatToInt(Vector3f fVector);
Vector2i FloatToInt(Vector2f fVector);
Vector3f IntToFloat(Vector3i iVector);
Vector2f IntToFloat(Vector2i iVector);
Matrix VecToMatrix(Vector3f vec);
Matrix VecToMatrix(DirectX::SimpleMath::Vector4 vec);
Vector3f MatrixToVec(Matrix m);
Vector2f proj(DirectX::SimpleMath::Vector4 vec);
#endif //__DXVECTORS_H__