#define NOMINMAX
#include "shader.h"
#include "dxvectors.h"
#include "model.h"

struct IShader
{
	virtual DirectX::SimpleMath::Vector4 vertex(int iface, int nthvert) = 0;
	virtual bool fragment(Vector3f bar, TGAColor& color) = 0;
};
class RasterBarycentric
{
public:
	Vector3f barycentric(Vector2f A, Vector2f B, Vector2f C, Vector2f P);
	void triangle(DirectX::SimpleMath::Vector4* pts, IShader& shader, TGAImage& image, TGAImage& zbuffer);
};
class RasterLinesweep
{
public:
	RasterLinesweep(Model *model, int width, int height);
	void rasterize(Vector3f& A, Vector3f& B, Vector2f& uvA, Vector2f& uvB, float intensityA, float intensityB, TGAImage& image, int* zBuffer);
	void triangle(Vector3i& t0, Vector3i& t1, Vector3i& t2, Vector2f uv0, Vector2f uv1, Vector2f uv2, TGAImage& image, float* intensity, int* zBuffer);
private:
	Model *model;
	int width;
	int height;
};

Matrix lookat(Vector3f eye, Vector3f center, Vector3f up);

Matrix viewport(int x, int y, int w, int h, int depth = 255);