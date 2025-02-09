#include "dxvectors.h"

Vector2i operator+(Vector2i a, Vector2i b)
{
	return Vector2i(a.x + b.x, a.y + b.y);
}
Vector2i operator-(Vector2i a, Vector2i b)
{
	return Vector2i(a.x - b.x, a.y - b.y);
}
Vector2i operator*(Vector2i vec, float alpha)
{
	return Vector2i((int)(vec.x * alpha), (int)(vec.y * alpha));
}
Vector3i operator+(Vector3i a, Vector3i b)
{
	return Vector3i(a.x + b.x, a.y + b.y, a.z + b.z);
}
Vector3i operator-(Vector3i a, Vector3i b)
{
	return Vector3i(a.x - b.x, a.y - b.y, a.z - b.z);
}
Vector3i operator*(Vector3i vec, float alpha)
{
	return Vector3i((int)(vec.x * alpha), (int)(vec.y * alpha), (int)(vec.z * alpha));
}
Vector3f operator+(Vector3i a, Vector3f b)
{
	return Vector3f(a.x + b.x, a.y + b.y, a.z + b.z);
}
Vector3f operator*(Matrix mat, Vector3f vec)
{
	return Vector3f(mat(0, 0) * vec.x + mat(0, 1) * vec.y + mat(0, 2) * vec.z + mat(0, 3),
		mat(1, 0) * vec.x + mat(1, 1) * vec.y + mat(1, 2) * vec.z + mat(1, 3),
		mat(2, 0) * vec.x + mat(2, 1) * vec.y + mat(2, 2) * vec.z + mat(2, 3));
}
Vector4f operator*(Matrix mat, Vector4f vec)
{
	return Vector4f(mat(0, 0) * vec.x + mat(0, 1) * vec.y + mat(0, 2) * vec.z + mat(0, 3) * vec.w,
		mat(1, 0) * vec.x + mat(1, 1) * vec.y + mat(1, 2) * vec.z + mat(1, 3) * vec.w,
		mat(2, 0) * vec.x + mat(2, 1) * vec.y + mat(2, 2) * vec.z + mat(2, 3) * vec.w,
		mat(3, 0) * vec.x + mat(3, 1) * vec.y + mat(3, 2) * vec.z + mat(3, 3) * vec.w);
}
TGAColor operator*(TGAColor color, float alpha)
{
	return TGAColor((unsigned char)std::clamp(color.r * alpha, 0.0f, 255.0f),
		(unsigned char)std::clamp(color.g * alpha, 0.0f, 255.0f),
		(unsigned char)std::clamp(color.b * alpha, 0.0f, 255.0f), color.a);
}
Vector4f embed1(Vector3f vec)
{
	return DirectX::SimpleMath::Vector4(vec.x, vec.y, vec.z, 0.0f);
}
Vector4f embed(Vector3f vec)
{
	return DirectX::SimpleMath::Vector4(vec.x, vec.y, vec.z, 1.0f);
}
Vector3f embed(Vector2f vec)
{
	return Vector3f(vec.x, vec.y, 1.0f);
}

Vector4f GetFirstColumn(Matrix m) { return Vector4f(m(0, 0), m(1, 0), m(2, 0), m(3, 0)); }
Vector4f GetMatrixRow(Matrix m, int row) { return Vector4f(m(row, 0), m(row, 1), m(row, 2), m(row, 3)); }

Vector3i FloatToInt(Vector3f fVector)
{
	return Vector3i((int)fVector.x, (int)fVector.y, (int)fVector.z);
}
Vector2i FloatToInt(Vector2f fVector)
{
	return Vector2i((int)fVector.x, (int)fVector.y);
}

Vector3f IntToFloat(Vector3i iVector)
{
	return Vector3f((float)iVector.x, (float)iVector.y, (float)iVector.z);
}
Vector2f IntToFloat(Vector2i iVector)
{
	return Vector2f((float)iVector.x, (float)iVector.y);
}

Matrix VecToMatrix(Vector3f vec)
{
	return Matrix(DirectX::SimpleMath::Vector4(vec.x, 0, 0, 0),
		DirectX::SimpleMath::Vector4(vec.y, 0, 0, 0),
		DirectX::SimpleMath::Vector4(vec.z, 0, 0, 0),
		DirectX::SimpleMath::Vector4(1, 0, 0, 0));
}
Matrix VecToMatrix(Vector4f vec)
{
	return Matrix(DirectX::SimpleMath::Vector4(vec.x, 0, 0, 0),
		DirectX::SimpleMath::Vector4(vec.y, 0, 0, 0),
		DirectX::SimpleMath::Vector4(vec.z, 0, 0, 0),
		DirectX::SimpleMath::Vector4(vec.w, 0, 0, 0));
}
Vector3f MatrixToVec(Matrix m)
{
	return Vector3f(m(0, 0) / m(3, 0), m(1, 0) / m(3, 0), m(2, 0) / m(3, 0));
}
Vector4f MatrixToVec4(Matrix m)
{
	return Vector4f(m(0, 0) / m(3, 0), m(1, 0) / m(3, 0), m(2, 0) / m(3, 0), 1.0f);
}
Vector2f proj(Vector4f vec)
{
	return Vector2f(vec.x, vec.y);
}