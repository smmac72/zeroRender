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
	// counts barycentric coordinates of P in the ABC triangle
	Vector3f barycentric(Vector2f A, Vector2f B, Vector2f C, Vector2f P);
	// draws triangle with a specified shader
	void triangle(Vector4f *pts, IShader& shader, TGAImage& image, TGAImage& zbuffer, bool bHasTextures);
};
class RasterLinesweep
{
public:
	// must have basic values dropped in
	RasterLinesweep(Model *model, int width, int height);
	// draws triangle
	void rasterize(Vector3f& A, Vector3f& B, Vector2f& uvA, Vector2f& uvB, float intensityA, float intensityB, TGAImage& image, int* zBuffer, bool bHasTextures);
	// splits triangle in halves and sends them to be drawn
	void triangle(Vector3i& t0, Vector3i& t1, Vector3i& t2, Vector2f uv0, Vector2f uv1, Vector2f uv2, TGAImage& image, float* intensity, int* zBuffer, bool bHasTextures);
private:
	Model *model;
	int width;
	int height;
};

// find camera->center look
Matrix lookat(Vector3f eye, Vector3f center, Vector3f up);
// get viewport matrix
Matrix viewport(int x, int y, int w, int h, int depth = 255);