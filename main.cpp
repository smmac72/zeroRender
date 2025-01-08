#define NOMINMAX 
#include "tgaimage.h"
#include "model.h"
#include "dxvectors.h"
#include <iostream>
#include <algorithm>
#include <string>

typedef DirectX::SimpleMath::Matrix Matrix;

Model* model = NULL;
//int* zBuffer = NULL;
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
Vector2i FloatToInt(Vector2f fVector)
{
	return Vector2i((int)fVector.x, (int)fVector.y);
}
Vector3f IntToFloat(Vector3i iVector)
{
	return Vector3f((float)iVector.x, (float)iVector.y, (float)iVector.z);
}
Vector2f IntToFloat(Vector2i iVector)
{
	return Vector2f((float)iVector.x, (float)iVector.y);
}
Vector3f operator+(Vector3i a, Vector3f b)
{
	return Vector3f(a.x + b.x, a.y + b.y, a.z + b.z);
}
TGAColor operator*(TGAColor color, float alpha)
{
	return TGAColor((unsigned char)std::clamp(color.r * alpha, 0.0f, 255.0f),
		(unsigned char)std::clamp(color.g * alpha, 0.0f, 255.0f),
		(unsigned char)std::clamp(color.b * alpha, 0.0f, 255.0f), color.a);
}
Vector2f proj(DirectX::SimpleMath::Vector4 vec)
{
	return Vector2f(vec.x, vec.y);
}
DirectX::SimpleMath::Vector4 embed(Vector3f vec)
{
	return DirectX::SimpleMath::Vector4(vec.x, vec.y, vec.z, 1.0f);
}
DirectX::SimpleMath::Vector3 embed(Vector2f vec)
{
	return DirectX::SimpleMath::Vector3(vec.x, vec.y, 1.0f);
}
DirectX::SimpleMath::Vector4 Right(Matrix m) { return DirectX::SimpleMath::Vector4(m(0,0), m(1, 0), m(2, 0), m(3, 0)); } // todo change name
TGAColor white = TGAColor(255, 255, 255, 255);
TGAColor red = TGAColor(255, 0, 0, 255);
TGAColor green = TGAColor(0, 255, 0, 255);
TGAColor blue = TGAColor(0, 0, 255, 255);

const int width = 800;
const int height = 800;
const int depth = 255;

struct IShader
{
	//virtual ~IShader();
	virtual DirectX::SimpleMath::Vector4 vertex(int iface, int nthvert) = 0;
	virtual bool fragment(Vector3f bar, TGAColor& color) = 0;
};

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

Vector3f eye(0, 0, 1);
Vector3f center(0, 0, 0);
Vector3f up(0, 1, 0);
Matrix ModelView = lookat(eye, center, up);
Matrix Projection = Matrix::Identity;
Matrix Viewport = viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
Vector3f lightDir = Vector3f(0, 0, 1);

Matrix VecToMatrix(Vector3f vec)
{
	return Matrix(DirectX::SimpleMath::Vector4(vec.x, 0, 0, 0),
		DirectX::SimpleMath::Vector4(vec.y, 0, 0, 0),
		DirectX::SimpleMath::Vector4(vec.z, 0, 0, 0),
		DirectX::SimpleMath::Vector4(1, 0, 0, 0));
}
Matrix VecToMatrix(DirectX::SimpleMath::Vector4 vec)
{
	return Matrix(DirectX::SimpleMath::Vector4(vec.x, 0, 0, 0),
		DirectX::SimpleMath::Vector4(vec.y, 0, 0, 0),
		DirectX::SimpleMath::Vector4(vec.z, 0, 0, 0),
		DirectX::SimpleMath::Vector4(vec.w, 0, 0, 0));
}
Vector3f MatrixToVec(Matrix m)
{
	return Vector3f(m(0, 0) / m(3, 0), m(1, 0) / m(3, 0), m(2, 0) / m(3, 0));
}
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

void rasterize(Vector3f &A, Vector3f &B, Vector2f &uvA, Vector2f &uvB, float intensityA, float intensityB, TGAImage& image, int* zBuffer)
{
	if (A.x > B.x) { std::swap(A, B); std::swap(uvA, uvB); std::swap(intensityA, intensityB);}
		
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
			//TGAColor color = model->diffuse(uvP);
			//image.set(P.x, P.y, color * intensityP);
		}
	}
}

void triangle(Vector3i &t0, Vector3i &t1, Vector3i &t2, Vector2f uv0, Vector2f uv1, Vector2f uv2, TGAImage& image, float *intensity, int *zBuffer)
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

Vector3f barycentric(Vector2f A, Vector2f B, Vector2f C, Vector2f P)
{
	Vector3f s[2] = { Vector3f(C.x - A.x, B.x - A.x, A.x - P.x),
						Vector3f(C.y - A.y, B.y - A.y, A.y - P.y) };

	Vector3f u = s[0].Cross(s[1]);
	if (std::abs(u.z) >= 1.0f)
		return Vector3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
	return Vector3f(-1, 1, 1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
}

void triangle_new(DirectX::SimpleMath::Vector4 *pts, IShader& shader, TGAImage& image, TGAImage& zbuffer)
{
	Vector2f bboxmin(INT_MAX, INT_MAX);
	Vector2f bboxmax(INT_MIN, INT_MIN);
	for (int i = 0; i < 3; i++)
	{
		bboxmin.x =  std::min(bboxmin.x, pts[i].x);
		bboxmin.y = std::min(bboxmin.y, pts[i].y);
		bboxmax.x = std::max(bboxmax.x, pts[i].x);
		bboxmax.y = std::max(bboxmax.y, pts[i].y);
	}
	Vector2i P;
	TGAColor color;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
	{
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
		{
			Vector3f c = barycentric(proj(pts[0] / pts[0].w), proj(pts[1] / pts[1].w), proj(pts[2] / pts[2].w), Vector2f(P.x, P.y));
			float z = pts[0].z * c.x + pts[1].z * c.y + pts[2].z * c.z;
			float w = pts[0].w * c.x + pts[1].w * c.y + pts[2].w * c.z;
			int frag_depth = std::max(0, std::min(255, int(z / w + 0.5)));
			if (c.x < 0 || c.y < 0 || c.z < 0 || zbuffer.get(P.x, P.y).b > frag_depth) continue;
			bool discard = shader.fragment(c, color);
			if (!discard)
			{
				zbuffer.set(P.x, P.y, TGAColor(0, 0, frag_depth, 0));
				image.set(P.x, P.y, color);
			}
		}
	}
}
void dumpZBuffer(TGAImage &image, int *zBuffer)
{
	TGAImage zbimage(width, height, TGAImage::GRAYSCALE);
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
			zbimage.set(i, j, TGAColor(zBuffer[i + j * width], 1));
	}
	zbimage.flip_vertically();
	zbimage.write_tga_file("zbuffer.tga");
}
struct GouraudShader : public IShader
{
	//virtual ~IShader() {}
	Vector3f varying_intensity; // written by vertex shader, read by fragment shader

	virtual DirectX::SimpleMath::Vector4 vertex(int iface, int nthvert)
	{
		DirectX::SimpleMath::Vector4 gl_Vertex = embed(model->vert(iface, nthvert)); // read the vertex from .obj file
		
		gl_Vertex = Right(Viewport * Projection * ModelView * VecToMatrix(gl_Vertex));     // transform it to screen coordinates
		switch (nthvert)
		{
		case 0:
			varying_intensity.x = std::max(0.f, model->normal(iface, nthvert).Dot(lightDir)); // get diffuse lighting intensity
			break;
		case 1:
			varying_intensity.y = std::max(0.f, model->normal(iface, nthvert).Dot(lightDir)); // get diffuse lighting intensity
			break;
		case 2:
			varying_intensity.z = std::max(0.f, model->normal(iface, nthvert).Dot(lightDir)); // get diffuse lighting intensity
			break;

		}
		return gl_Vertex;
	}

	virtual bool fragment(Vector3f bar, TGAColor& color)
	{
		float intensity = varying_intensity.Dot(bar);   // interpolate intensity for the current pixel
		color = TGAColor(255, 255, 255, 255) * intensity; // well duh
		return false;                              // no, we do not discard this pixel
	}
};

int main(int argc, char** argv)
{
	if (argc == 2)
		model = new Model(argv[1]);
	else
		model = new Model("testmodel.obj");


	TGAImage image(width, height, TGAImage::RGB);
	TGAImage zBuffer(width, height, TGAImage::GRAYSCALE);
	lightDir.Normalize();

	GouraudShader shader;
	for (int i = 0; i < model->nfaces(); i++)
	{
		DirectX::SimpleMath::Vector4 screenCoords[3];
		for (int j = 0; j < 3; j++)
			screenCoords[j] = shader.vertex(i, j);
		triangle_new(screenCoords, shader, image, zBuffer);
	}
	image.flip_vertically();
	image.write_tga_file("output.tga");
	zBuffer.flip_vertically();
	zBuffer.write_tga_file("zBuffer.tga");

	/*
	int *zbuf = new int[width * height];
	for (int i = 0; i < width * height; i++) {
		zbuf[i] = INT_MIN;
	}
	for (int i = 0; i < model->nfaces(); i++)
	{
		std::vector<int> face = model->face(i);
		Vector3i screenCoords[3];
		Vector3f worldCoords[3];
		for (int j = 0; j < 3; j++)
		{
			worldCoords[j] = model->vert(face[j]);
			// i have no idea how to use simplemath
			screenCoords[j] = FloatToInt(MatrixToVec(Viewport * Projection * ModelView * VecToMatrix(worldCoords[j])));
		}
		Vector2f uv[3];
		float intensity[3];
		for (int j = 0; j < 3; j++)
		{
			uv[j] = model->uv(i, j);
			intensity[j] = std::clamp(model->normal(i, j).Dot(lightDir), 0.0f, 1.0f);
		}
		triangle(screenCoords[0], screenCoords[1], screenCoords[2], uv[0], uv[1], uv[2], image, intensity, zbuf);
	}


	dumpZBuffer(image, zbuf);
	delete model;
	delete[] zbuf;*/
	return 0;
}

