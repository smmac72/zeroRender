#ifndef __DXVECTORS_H__
#define __DXVECTORS_H__
#include <d3d12.h>
#include "SimpleMath.h"
#include "tgaimage.h"
#include <algorithm>
typedef DirectX::SimpleMath::Vector4 Vector4f;
typedef DirectX::SimpleMath::Vector3 Vector3f;
typedef DirectX::SimpleMath::Vector2 Vector2f;
typedef DirectX::SimpleMath::Matrix Matrix;
// Integer Vector2
typedef DirectX::XMINT2 Vector2i;
// Integer Vector3
typedef DirectX::XMINT3 Vector3i;

// defines Vector2i + Vector2i
Vector2i operator+(Vector2i a, Vector2i b);
// defines Vector2i - Vector2i
Vector2i operator-(Vector2i a, Vector2i b);
// defines Vector2i * float
Vector2i operator*(Vector2i vec, float alpha);
// defines Vector3i + Vector3i
Vector3i operator+(Vector3i a, Vector3i b);
// defines Vector3i - Vector3i
Vector3i operator-(Vector3i a, Vector3i b);
// defines Vector3i * float
Vector3i operator*(Vector3i vec, float alpha); 
// defines Vector3i + Vector3f
Vector3f operator+(Vector3i a, Vector3f b);
// defines Matrix * Vector3
Vector3f operator*(Matrix mat, Vector3f vec); 
// defines Matrix * Vector4
Vector4f operator*(Matrix mat, Vector4f vec); 
// defines TGAColor * float
TGAColor operator*(TGAColor color, float alpha);
Vector4f embed1(Vector3f vec);
// extends Vector3 to Vector4(Vec3.x, Vec3.y, Vec3.z, 1.0f)
Vector4f embed(Vector3f vec);
// extends Vector2 to Vector3(Vec2.x, Vec2.y, 1.0f)
Vector3f embed(Vector2f vec);
// returns first matrix column. replaces Up() for Vector3f
Vector4f GetFirstColumn(Matrix m);
// returns matrix row
Vector4f GetMatrixRow(Matrix m, int row);
// for some fucking reason this is needed (c) copilot comment autocompletion. tbh no idea why it worked, vec4 in barycentric requires w=1
Vector4f MatrixToVec4(Matrix m);
// defines Vector3f -> Vector3i
Vector3i FloatToInt(Vector3f fVector);
// defines Vector2f -> Vector2i
Vector2i FloatToInt(Vector2f fVector);
// defines Vector3i -> Vector3f
Vector3f IntToFloat(Vector3i iVector);
// defines Vector2i -> Vector2f
Vector2f IntToFloat(Vector2i iVector);
// creates 4x4 matrix with first column being the embedded Vector3
Matrix VecToMatrix(Vector3f vec);
// creates 4x4 matrix with first column being the Vector4
Matrix VecToMatrix(Vector4f vec);
// tbh no idea why this works, i've skipped my linear algebra classes
// defines Matrix->Vector3f
Vector3f MatrixToVec(Matrix m);
// cuts Vector4 to Vector2(Vec4.x, Vec4.y)
Vector2f proj(Vector4f vec);
#endif //__DXVECTORS_H__