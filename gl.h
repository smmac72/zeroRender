#ifndef __GL_H__
#define __GL_H__
#include "tgaimage.h"
#include "dxvectors.h"

Model* model = NULL;
int* zBuffer = NULL;

TGAColor white = TGAColor(255, 255, 255, 255);
TGAColor red = TGAColor(255, 0, 0, 255);
TGAColor green = TGAColor(0, 255, 0, 255);
TGAColor blue = TGAColor(0, 0, 255, 255);

const int width = 800;
const int height = 800;
const int depth = 255;

extern Matrix ModelView;
extern Matrix Projection;

Matrix viewport(int x, int y, int w, int h);
void projection(float coeff = 0.f); // coeff = -1/c
Matrix lookat(Vector3f eye, Vector3f center, Vector3f up);

struct IShader {
    virtual ~IShader();
    virtual Vector4f vertex(int iface, int nthvert) = 0;
    virtual bool fragment(Vector3f bar, TGAColor& color) = 0;
};

void triangle(Vector3i& t0, Vector3i& t1, Vector3i& t2, Vector2f uv0, Vector2f uv1, Vector2f uv2, TGAImage& image, float* intensity, int* zBuffer);
void rasterize(Vector3f& A, Vector3f& B, Vector2f& uvA, Vector2f& uvB, float intensityA, float intensityB, TGAImage& image, int* zBuffer);

#endif //__GL_H__