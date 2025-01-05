#include "tgaimage.h"
#include "model.h"
#include <iostream>

#include <d3d12.h>
#include "SimpleMath.h"
typedef DirectX::SimpleMath::Vector3 Vector3f;
typedef DirectX::XMINT2 Vector2i; // simple math supports only XMFLOAT. probably interchangeable
typedef DirectX::XMINT3 Vector3i;
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
Vector3i FloatToInt(Vector3f fVector)
{
	return Vector3i((int)fVector.x, (int)fVector.y, (int)fVector.z);
}
Vector3f IntToFloat(Vector3i iVector)
{
	return Vector3f(iVector.x, iVector.y, iVector.z);
}
Vector3f operator+(Vector3i a, Vector3f b)
{
	return Vector3f(a.x + b.x, a.y + b.y, a.z + b.z);
}

Model* model = NULL;
int* zBuffer = NULL;

TGAColor white = TGAColor(255, 255, 255, 255);
TGAColor red = TGAColor(255, 0, 0, 255);
TGAColor green = TGAColor(0, 255, 0, 255);
TGAColor blue = TGAColor(0, 0, 255, 255);

const int width = 800;
const int height = 800;
const int depth = 255;

void leftRightSort(Vector2i &start, Vector2i &end)
{
	if (start.x > end.x)
		std::swap(start, end);
}
bool isLineSteep(Vector2i &start, Vector2i &end)
{
	if (std::abs(start.x - end.x) < std::abs(start.y - end.y))
	{
		std::swap(start.x, start.y);
		std::swap(end.x, end.y);
		return true;
	}
	return false;
}
void line(Vector2i start, Vector2i end, TGAImage& image, TGAColor &color)
{
	bool isSteep = isLineSteep(start, end);
	leftRightSort(start, end);

	int dx = end.x - start.x;
	int dy = end.y - start.y;
	int derror = 2 * std::abs(dy);
	int error = 0;
	int y = start.y;
	for (int x = start.x; x <= end.x; x++)
	{
		if (isSteep)
			image.set(y, x, color);
		else
			image.set(x, y, color);
		error += derror;
		if (error > dx)
		{
			y += end.y > start.y ? 1 : -1;
			error -= 2 * dx;
		}
	}
}

void rasterize(Vector3f &A, Vector3f &B, TGAImage& image, TGAColor& color, int* zBuffer)
{
	if (A.x > B.x)
		std::swap(A, B);
	for (int x = A.x; x <= B.x; x++)
	{
		float phi = A.x == B.x ? 1.0 : (x - A.x) / (float)(B.x - A.x);
		Vector3i P = FloatToInt(A + (B - A) * phi);
		int idx = P.x + P.y * width;
		if (zBuffer[idx] < P.z)
		{
			zBuffer[idx] = P.z;
			image.set(P.x, P.y, color);
		}
	}
}

void triangle(Vector3i &t0, Vector3i &t1, Vector3i &t2, TGAImage& image, TGAColor color, int *zBuffer)
{
	if (t0.y == t1.y && t0.y == t2.y)
		return;
	if (t0.y > t1.y) std::swap(t0, t1);
	if (t0.y > t2.y) std::swap(t0, t2);
	if (t1.y > t2.y) std::swap(t1, t2);

	int totalHeight = t2.y - t0.y;
	for (int yOffset = 0; yOffset < totalHeight; yOffset++)
	{
		bool isSecondHalf = yOffset > t1.y - t0.y || t1.y == t0.y;
		int segmentHeight = isSecondHalf ? t2.y - t1.y : t1.y - t0.y;

		float alpha = (float)yOffset / totalHeight;
		float beta = (float)(yOffset - (isSecondHalf ? t1.y - t0.y : 0.0)) / segmentHeight;
		Vector3f A = t0 + IntToFloat(t2 - t0) * alpha;
		Vector3f B = isSecondHalf ? (t1 + IntToFloat(t2 - t1) * beta) : (t0 + IntToFloat(t1 - t0) * beta);
		
		rasterize(A, B, image, color, zBuffer);
	}
}

void dumpZBuffer(TGAImage &image, int *zBuffer)
{
	image.write_tga_file("output.tga");
	TGAImage zbimage(width, height, TGAImage::GRAYSCALE);
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
			zbimage.set(i, j, TGAColor(zBuffer[i + j * width], 1));
	}
	zbimage.flip_vertically();
	zbimage.write_tga_file("zbuffer.tga");
}
int main(int argc, char** argv)
{
	if (argc == 2)
		model = new Model(argv[1]);
	else
		model = new Model("testmodel.obj");

	zBuffer = new int[width * height];
	for (int i = 0; i < width * height; i++) {
		zBuffer[i] = INT_MIN;
	}

	TGAImage image(width, height, TGAImage::RGB);
	Vector3f lightDir(0, 0, -1);
	for (int i = 0; i < model->nfaces(); i++)
	{
		std::vector<int> face = model->face(i);
		Vector3i screenCoords[3];
		Vector3f worldCoords[3];
		for (int j = 0; j < 3; j++)
		{
			worldCoords[j] = model->vert(face[j]);
			screenCoords[j] = Vector3i((int)((worldCoords[j].x + 1.0) * width / 2.0),
									   (int)((worldCoords[j].y + 1.0) * height / 2.0),
									   (int)((worldCoords[j].z + 1.0) * depth / 2.0));
		}
		Vector3f normal = (worldCoords[2] - worldCoords[0]).Cross((worldCoords[1] - worldCoords[0]));
		normal.Normalize();
		float lightIntensity = normal.Dot(lightDir);
		if (lightIntensity > 0)
			triangle(screenCoords[0], screenCoords[1], screenCoords[2], image, TGAColor(255 * lightIntensity, 255 * lightIntensity, 255 * lightIntensity, 255), zBuffer);
	}

	image.flip_vertically();

	dumpZBuffer(image, zBuffer);
	delete model;
	delete[] zBuffer;
	return 0;
}

