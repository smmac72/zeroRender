#define NOMINMAX 

#include "tgaimage.h"
#include "model.h"
#include "dxvectors.h"
#include "render.h"

Model* model = NULL;

const int width = 800;
const int height = 800;
const int depth = 255;

Vector3f eye(0, 0, -1);
Vector3f center(0, 0, 0);
Vector3f up(0, 1, 0);
Matrix ModelView = lookat(eye, center, up);
Matrix Projection = Matrix::Identity;
Matrix Viewport = viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
Vector3f lightDir = Vector3f(0, 0, -1);

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
	Vector3f          varying_intensity; // written by vertex shader, read by fragment shader
	Matrix varying_uv;

	virtual DirectX::SimpleMath::Vector4 vertex(int iface, int nthvert)
	{
		switch (nthvert)
		{
		case 0:
			varying_intensity.x = std::max(0.f, model->normal(iface, nthvert).Dot(lightDir)); // get diffuse lighting intensity
			varying_uv(0, 0) = model->uv(iface, nthvert).x;
			varying_uv(1, 0) = model->uv(iface, nthvert).y;
			break;
		case 1:
			varying_intensity.y = std::max(0.f, model->normal(iface, nthvert).Dot(lightDir)); // get diffuse lighting intensity
			varying_uv(0, 1) = model->uv(iface, nthvert).x;
			varying_uv(1, 1) = model->uv(iface, nthvert).y;
			break;
		case 2:
			varying_intensity.z = std::max(0.f, model->normal(iface, nthvert).Dot(lightDir)); // get diffuse lighting intensity
			varying_uv(0, 2) = model->uv(iface, nthvert).x;
			varying_uv(1, 2) = model->uv(iface, nthvert).y;
			break;

		}
		DirectX::SimpleMath::Vector4 gl_Vertex = embed(model->vert(iface, nthvert)); // read the vertex from .obj file
		return GetFirstColumn(Viewport * Projection * ModelView * VecToMatrix(gl_Vertex)); // transform it to screen coordinates
	}

	virtual bool fragment(Vector3f bar, TGAColor& color)
	{
		float intensity = varying_intensity.Dot(bar);   // interpolate intensity for the current pixel
		Vector2f uv = Vector2f(varying_uv.Right().Dot(bar), varying_uv.Up().Dot(bar));
		color = model->diffuse(uv) * intensity;
		return false;
	}
};

bool ParseArg(int argc, char** argv)
{
	for (int i = 0; i < argc; i++)
	{

	}
}

int main(int argc, char** argv)
{
	if (argc > 1)
		ParseArg(argc, argv);
	if (argc == 2)
		model = new Model(argv[1]);
	else
		model = new Model("smmac72.obj");

	bool bBarycentric = true;
	TGAImage image(width, height, TGAImage::RGB);
	lightDir.Normalize();

	if (bBarycentric)
	{
		TGAImage zBuffer(width, height, TGAImage::GRAYSCALE);
		RasterBarycentric* render = new RasterBarycentric();
		GouraudShader shader;
		for (int i = 0; i < model->nfaces(); i++)
		{
			DirectX::SimpleMath::Vector4 screenCoords[3];
			for (int j = 0; j < 3; j++)
				screenCoords[j] = shader.vertex(i, j);
			render->triangle(screenCoords, shader, image, zBuffer);
		}
	}
	else
	{
		RasterLinesweep* render = new RasterLinesweep(model, width, height);
		int* zbuf = new int[width * height];
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
				screenCoords[j] = FloatToInt(MatrixToVec(Viewport * Projection * ModelView * VecToMatrix(worldCoords[j])));
			}
			Vector2f uv[3];
			float intensity[3];
			for (int j = 0; j < 3; j++)
			{
				uv[j] = model->uv(i, j);
				intensity[j] = std::clamp(model->normal(i, j).Dot(lightDir), 0.0f, 1.0f);
			}
			render->triangle(screenCoords[0], screenCoords[1], screenCoords[2], uv[0], uv[1], uv[2], image, intensity, zbuf);
		}
	}

	image.flip_vertically();
	image.write_tga_file("output.tga");

	delete model;
	return 0;
}

