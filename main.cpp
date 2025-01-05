#include "tgaimage.h"
#include "model.h"
#include <iostream>

#include <d3d12.h>
#include "SimpleMath.h"
typedef DirectX::XMINT2 Vector2i; // simple math supports only XMFLOAT. probably interchangeable
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

Model* model = NULL;

TGAColor white = TGAColor(255, 255, 255, 255);
TGAColor red   = TGAColor(255, 0,   0,   255);

const int width = 250;
const int height = 250;

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
void triangle(Vector2i t0, Vector2i t1, Vector2i t2, TGAImage& image, TGAColor color)
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
		int segmentHeight = isSecondHalf ? t2.y - t1.y + 1 : t1.y - t0.y + 1;

		float alpha = (float)yOffset / totalHeight;
		float beta = (float)(yOffset - (isSecondHalf ? t1.y - t0.y : 0)) / segmentHeight;

		Vector2i A =				t0 + (t2 - t0) * alpha;
		Vector2i B = isSecondHalf ? t1 + (t2 - t1) * beta : t0 + (t1 - t0) * beta;
		if (A.x > B.x)
			std::swap(A, B);
		for (int x = A.x; x <= B.x; x++)
			image.set(x, t0.y + yOffset, color);
	}
}

int main(int argc, char** argv)
{
	TGAImage image(width, height, TGAImage::RGB);

	if (argc == 2)
		model = new Model(argv[1]);
	else
		model = new Model("testmodel.obj");

	DirectX::SimpleMath::Vector3 lightDir(0, 0, -1);
	for (int i = 0; i < model->nfaces(); i++)
	{
		std::vector<int> face = model->face(i);
		Vector2i screenCoords[3];
		DirectX::SimpleMath::Vector3 worldCoords[3];
		for (int j = 0; j < 3; j++)
		{
			worldCoords[j] = model->vert(face[j]);
			screenCoords[j] = Vector2i((int)((worldCoords[j].x + 1.0) * width / 2.0), (int)((worldCoords[j].y + 1.0) * height / 2.0));
		}
		DirectX::SimpleMath::Vector3 normal = (worldCoords[2] - worldCoords[0]).Cross((worldCoords[1] - worldCoords[0]));
		normal.Normalize();
		float lightIntensity = normal.Dot(lightDir);
		if (lightIntensity > 0)
			triangle(screenCoords[0], screenCoords[1], screenCoords[2], image, TGAColor(255 * lightIntensity, 255 * lightIntensity, 255 * lightIntensity, 255));
	}

	image.flip_vertically();
	image.write_tga_file("output.tga");
	return 0;
}

