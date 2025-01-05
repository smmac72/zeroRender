#include "tgaimage.h"
#include <iostream>

#include <d3d12.h>
#include "SimpleMath.h"
typedef DirectX::XMINT2 Vector2i; // simple math supports only XMFLOAT. probably interchangeable

TGAColor white = TGAColor(255, 255, 255, 255);
TGAColor red   = TGAColor(255, 0,   0,   255);

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
	TGAImage image(100, 100, TGAImage::RGB);
	line(Vector2i(13, 20), Vector2i(80, 40), image, white);
	line(Vector2i(20, 13), Vector2i(40, 80), image, red);
	line(Vector2i(80, 40), Vector2i(13, 20), image, red);
	image.flip_vertically();
	image.write_tga_file("output.tga");
	return 0;
}

