#include "tgaimage.h"
#include "model.h"
#include <iostream>

#include <d3d12.h>
#include "SimpleMath.h"
typedef DirectX::XMINT2 Vector2i; // simple math supports only XMFLOAT. probably interchangeable

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
int main(int argc, char** argv)
{
	TGAImage image(width, height, TGAImage::RGB);
	Model* model = new Model("testmodel.obj");
	for (int i = 0; i < model->nfaces(); i++)
	{
		std::vector<int> face = model->face(i);
		for (int j = 0; j < 3; j++) {
			DirectX::SimpleMath::Vector3 v0 = model->vert(face[j]);
			DirectX::SimpleMath::Vector3 v1 = model->vert(face[(j + 1) % 3]);
			int x0 = (v0.x + 1.) * width / 2.;
			int y0 = (v0.y + 1.) * height / 2.;
			int x1 = (v1.x + 1.) * width / 2.;
			int y1 = (v1.y + 1.) * height / 2.;
			line(Vector2i(x0, y0), Vector2i(x1, y1), image, white);
		}
	}
	image.flip_vertically();
	image.write_tga_file("output.tga");
	return 0;
}

