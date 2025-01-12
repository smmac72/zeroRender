#include "render.h"
#include <iostream>
Vector3f RasterBarycentric::barycentric(Vector2f A, Vector2f B, Vector2f C, Vector2f P)
{
	// P = (1-u-v)A + uB + vC
	// finding vector perpendicular to u, v by a cross function
	Vector3f s[2] = { Vector3f(C.x - A.x, B.x - A.x, A.x - P.x),
						Vector3f(C.y - A.y, B.y - A.y, A.y - P.y) };

	Vector3f u = s[0].Cross(s[1]);
	if (std::abs(u.z) >= 1.0f) // checking if the triangle is degenerate
		return Vector3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
	return Vector3f(-1, 1, 1); 
}

void RasterBarycentric::triangle(Vector4f *pts, IShader& shader, TGAImage& image, TGAImage& zbuffer, bool bHasTextures)
{
	// finding box limits for our triangle
	Vector2f bboxmin(FLT_MAX, FLT_MAX);
	Vector2f bboxmax(FLT_MIN, FLT_MIN);
	for (int i = 0; i < 3; i++)
	{
		bboxmin.x = std::min(bboxmin.x, pts[i].x);
		bboxmin.y = std::min(bboxmin.y, pts[i].y);
		bboxmax.x = std::max(bboxmax.x, pts[i].x);
		bboxmax.y = std::max(bboxmax.y, pts[i].y);
	}

	// for every point in - we draw it
	Vector2i P;
	TGAColor color;
	for (P.x = (int)bboxmin.x; P.x <= (int)bboxmax.x; P.x++)
	{
		for (P.y = (int)bboxmin.y; P.y <= (int)bboxmax.y; P.y++)
		{
			// math which works
			Vector3f c = barycentric(proj(pts[0] / pts[0].w), proj(pts[1] / pts[1].w), proj(pts[2] / pts[2].w), Vector2f((float)P.x, (float)P.y));
			float z = pts[0].z * c.x + pts[1].z * c.y + pts[2].z * c.z;
			float w = pts[0].w * c.x + pts[1].w * c.y + pts[2].w * c.z;
			int frag_depth = std::max(0, std::min(255, int(z / w + 0.5)));

			// ignore if out of the screen bounds OR we already drew there (zbuffer check)
			if (c.x < 0 || c.y < 0 || c.z < 0 || zbuffer.get(P.x, P.y).b > frag_depth)
				continue;
			// check if we should discard. we don't in these shaders, but some shaders will discard vertices
			if (!bHasTextures)
				color = TGAColor(255, 255, 255, 255);
			bool discard = shader.fragment(c, color);
			if (!discard)
			{
				zbuffer.set(P.x, P.y, TGAColor(0, 0, frag_depth, 0)); // BE CAREFUL TGAIMAGE IS BGRA NOT RBGA GOD DAMN IT
				image.set(P.x, P.y, color);
			}
		}
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

Matrix viewport(int x, int y, int w, int h, int depth)
{
	Matrix out(DirectX::XMFLOAT4(w / 2.0f, 0.0f, 0.0f, x + w / 2.0f),
		DirectX::XMFLOAT4(0.0f, h / 2.0f, 0.0f, y + h / 2.0f),
		DirectX::XMFLOAT4(0.0f, 0.0f, depth / 2.0f, depth / 2.0f),
		DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	return out;
}

RasterLinesweep::RasterLinesweep(Model *model, int width, int height)
{
	this->model = model;
	this->width = width;
	this->height = height;
}

void RasterLinesweep::rasterize(Vector3f& A, Vector3f& B, Vector2f& uvA, Vector2f& uvB, float intensityA, float intensityB, TGAImage& image, int* zBuffer, bool bHasTextures)
{
	// sort by ascending - we draw from A to B
	if (A.x > B.x) { std::swap(A, B); std::swap(uvA, uvB); std::swap(intensityA, intensityB); }

	for (float x = A.x; x <= B.x; x++)
	{
		// interpolating coordinates between A.x and B.x
		float phi = A.x == B.x ? 1.0f : (x - A.x) / (float)(B.x - A.x);
		// all of out Vector3f are returning to integer screen coordinates
		Vector3i P = FloatToInt(A + (B - A) * phi);
		Vector2i uvP = FloatToInt(uvA + (uvB - uvA) * phi);
		float intensityP = intensityA + (intensityB - intensityA) * phi;

		// packing 2d array into 1d
		int idx = P.x + P.y * width;
		if (P.x >= width || P.y >= height || P.x < 0 || P.y < 0) continue;
		if (zBuffer[idx] < P.z)
		{
			zBuffer[idx] = P.z;
			if (!bHasTextures)
			{
				image.set(P.x, P.y, TGAColor((unsigned char)255, (unsigned char)255, (unsigned char)255, (unsigned char)255) * intensityP);
			}
			else
			{
				TGAColor color = model->diffuse(IntToFloat(uvP));
				image.set(P.x, P.y, color * intensityP);
			}
		}
	}
}

void RasterLinesweep::triangle(Vector3i& t0, Vector3i& t1, Vector3i& t2, Vector2f uv0, Vector2f uv1, Vector2f uv2, TGAImage& image, float* intensity, int* zBuffer, bool bHasTextures)
{
	// don't draw degenerate triangles
	if (t0.y == t1.y && t0.y == t2.y)
		return;
	// sort by ascending - we draw t0->t1->t2 by height
	if (t0.y > t1.y) { std::swap(t0, t1); std::swap(uv0, uv1); std::swap(intensity[0], intensity[1]); }
	if (t0.y > t2.y) { std::swap(t0, t2); std::swap(uv0, uv2); std::swap(intensity[0], intensity[2]); }
	if (t1.y > t2.y) { std::swap(t1, t2); std::swap(uv1, uv2); std::swap(intensity[1], intensity[2]); }

	int totalHeight = t2.y - t0.y;
	for (int yOffset = 0; yOffset < totalHeight; yOffset++)
	{
		// split in halves - from t0 to t1 and from t1 to t2
		bool isSecondHalf = yOffset > t1.y - t0.y || t1.y == t0.y;
		int segmentHeight = isSecondHalf ? t2.y - t1.y : t1.y - t0.y;

		// interpolating coordinates between minY and maxY
		float alpha = (float)yOffset / totalHeight;
		float beta = (float)(yOffset - (isSecondHalf ? t1.y - t0.y : 0.0)) / segmentHeight;
		Vector3f A = t0 + IntToFloat(t2 - t0) * alpha; // some int->float chicanery for precision
		Vector3f B = isSecondHalf ? (t1 + IntToFloat(t2 - t1) * beta) : (t0 + IntToFloat(t1 - t0) * beta);
		Vector2f uvA = uv0 + (uv2 - uv0) * alpha;
		Vector2f uvB = isSecondHalf ? (uv1 + (uv2 - uv1) * beta) : (uv0 + (uv1 - uv0) * beta);
		float intensityA = intensity[0] + (intensity[2] - intensity[0]) * alpha;
		float intensityB = isSecondHalf ? (intensity[1] + (intensity[2] - intensity[1]) * beta) : (intensity[0] + (intensity[1] - intensity[0]) * beta);
		// send a triangle for drawing
		rasterize(A, B, uvA, uvB, intensityA, intensityB, image, zBuffer, bHasTextures);
	}
}