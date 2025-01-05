#include "tgaimage.h"

#include <d3d12.h>
#include "SimpleMath.h"
using namespace DirectX::SimpleMath;

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);

int main(int argc, char** argv)
{
	TGAImage image(1920, 1080, TGAImage::RGB);
	image.set(960, 540, white);
	image.write_tga_file("output.tga");
	return 0;
}

