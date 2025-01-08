#include "model.h"
#include "gl.h"


Vector3f operator+(Vector3i a, Vector3f b)
{
	return Vector3f(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vector3i operator-(Vector3i a, Vector3i b)
{
	return Vector3i(a.x - b.x, a.y - b.y, a.z - b.z);
}
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
Vector3i operator*(Vector3i vec, float alpha)
{
	return Vector3i((int)(vec.x * alpha), (int)(vec.y * alpha), (int)(vec.z * alpha));
}
void rasterize(Vector3f& A, Vector3f& B, Vector2f& uvA, Vector2f& uvB, float intensityA, float intensityB, TGAImage& image, int* zBuffer)
{
	if (A.x > B.x) { std::swap(A, B); std::swap(uvA, uvB); std::swap(intensityA, intensityB); }

	for (float x = A.x; x <= B.x; x++)
	{
		float phi = A.x == B.x ? 1.0f : (x - A.x) / (float)(B.x - A.x);
		Vector3i P = FloatToInt(A + (B - A) * phi);
		Vector2i uvP = FloatToInt(uvA + (uvB - uvA) * phi);
		float intensityP = intensityA + (intensityB - intensityA) * phi;
		int idx = P.x + P.y * width;
		if (P.x >= width || P.y >= height || P.x < 0 || P.y < 0) continue;
		if (zBuffer[idx] < P.z)
		{
			zBuffer[idx] = P.z;
			TGAColor color = model->diffuse(uvP);
			image.set(P.x, P.y, color * intensityP);
		}
	}
}

void triangle(Vector3i& t0, Vector3i& t1, Vector3i& t2, Vector2f uv0, Vector2f uv1, Vector2f uv2, TGAImage& image, float* intensity, int* zBuffer)
{
	if (t0.y == t1.y && t0.y == t2.y)
		return;
	if (t0.y > t1.y) { std::swap(t0, t1); std::swap(uv0, uv1); std::swap(intensity[0], intensity[1]); }
	if (t0.y > t2.y) { std::swap(t0, t2); std::swap(uv0, uv2); std::swap(intensity[0], intensity[2]); }
	if (t1.y > t2.y) { std::swap(t1, t2); std::swap(uv1, uv2); std::swap(intensity[1], intensity[2]); }

	int totalHeight = t2.y - t0.y;
	for (int yOffset = 0; yOffset < totalHeight; yOffset++)
	{
		bool isSecondHalf = yOffset > t1.y - t0.y || t1.y == t0.y;
		int segmentHeight = isSecondHalf ? t2.y - t1.y : t1.y - t0.y;

		float alpha = (float)yOffset / totalHeight;
		float beta = (float)(yOffset - (isSecondHalf ? t1.y - t0.y : 0.0)) / segmentHeight;
		Vector3f A = t0 + IntToFloat(t2 - t0) * alpha;
		Vector3f B = isSecondHalf ? (t1 + IntToFloat(t2 - t1) * beta) : (t0 + IntToFloat(t1 - t0) * beta);
		Vector2f uvA = uv0 + (uv2 - uv0) * alpha;
		Vector2f uvB = isSecondHalf ? (uv1 + (uv2 - uv1) * beta) : (uv0 + (uv1 - uv0) * beta);
		float intensityA = intensity[0] + (intensity[2] - intensity[0]) * alpha;
		float intensityB = isSecondHalf ? (intensity[1] + (intensity[2] - intensity[1]) * beta) : (intensity[0] + (intensity[1] - intensity[0]) * beta);
		rasterize(A, B, uvA, uvB, intensityA, intensityB, image, zBuffer);
	}
}


Matrix lookat(Vector3f eye, Vector3f center, Vector3f up)
{
	Vector3f z = (eye - center);
	z.Normalize();
	Vector3f x = up.Cross(z);
	x.Normalize();
	Vector3f y = z.Cross(x);
	y.Normalize();
	Matrix Minv(DirectX::XMFLOAT4(x.x, x.y, x.z, 0.0f),
		DirectX::XMFLOAT4(y.x, y.y, y.z, 0.0f),
		DirectX::XMFLOAT4(z.x, z.y, z.z, 0.0f),
		DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	Matrix Tr(DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, -center.x),
		DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, -center.y),
		DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, -center.z),
		DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	Minv *= Tr;
	return Minv;
}

Matrix viewport(int x, int y, int w, int h)
{
	Matrix out(DirectX::XMFLOAT4(w / 2.0f, 0.0f, 0.0f, x + w / 2.0f),
		DirectX::XMFLOAT4(0.0f, h / 2.0f, 0.0f, y + h / 2.0f),
		DirectX::XMFLOAT4(0.0f, 0.0f, depth / 2.0f, depth / 2.0f),
		DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	return out;
}