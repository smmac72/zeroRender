#define NOMINMAX 

#include "tgaimage.h"
#include "model.h"
#include "dxvectors.h"
#include "render.h"
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>

/// /// /// /// /// /// /// ///
/// DEFINES BASIC VARIABLES ///
/// /// /// /// /// /// /// ///
Model* model = NULL;

// output image width
int width = 800;
// output image height
int height = 800;
// output image depth
int depth = 255;

// camera position
Vector3f eye;
// camera target
Vector3f center;
// up vector
Vector3f up;
// local object coordinates to world transformation
Matrix ModelView;
// world coordinates to camera transformation
Matrix Projection;
// camera coodrinates to screen transformation
Matrix Viewport;
// light direction
Vector3f lightDir;
// phong coefs - ambient/diffuse/specular/shininess
Vector4f PhongCoef;

// gouraud shading shader
struct GouraudShader : public IShader
{
	Vector3f intensity;
	Matrix uv; // can be 2x3 matrix (three pairs of uvs), but simplemath multiplication said no

	// get intensity and uv for each vertex in vertex shader
	virtual DirectX::SimpleMath::Vector4 vertex(int iface, int nthvert)
	{
		// check for each of the three vertices
		// probably could do better, but simplemath requires access by .xyz
		switch (nthvert)
		{
		case 0:
			intensity.x = std::max(0.f, model->normal(iface, nthvert).Dot(lightDir)); // get diffuse lighting intensity
			// get vertex uv
			uv(0, 0) = model->uv(iface, nthvert).x;
			uv(1, 0) = model->uv(iface, nthvert).y;
			break;
		case 1:
			intensity.y = std::max(0.f, model->normal(iface, nthvert).Dot(lightDir));
			uv(0, 1) = model->uv(iface, nthvert).x;
			uv(1, 1) = model->uv(iface, nthvert).y;
			break;
		case 2:
			intensity.z = std::max(0.f, model->normal(iface, nthvert).Dot(lightDir));
			uv(0, 2) = model->uv(iface, nthvert).x;
			uv(1, 2) = model->uv(iface, nthvert).y;
			break;

		}
		DirectX::SimpleMath::Vector4 gl_Vertex = embed(model->vert(iface, nthvert)); // read vertex
		return GetFirstColumn(Viewport * Projection * ModelView * VecToMatrix(gl_Vertex)); // transform it to screen coordinates
	}

	// interpolate intensity and uv in fragment shader
	virtual bool fragment(Vector3f bar, TGAColor& color)
	{
		float outIntensity = intensity.Dot(bar);
		Vector2f outUV = Vector2f(uv.Right().Dot(bar), uv.Up().Dot(bar));
		color = model->diffuse(outUV) * outIntensity;
		return false;
	}
};

// phong shading shader
struct PhongShader : public IShader
{
	Vector3f intensity;
	Vector3f camera; // camera point, no
	Matrix uv; // still just three pairs of uvs

	virtual DirectX::SimpleMath::Vector4 vertex(int iface, int nthvert) override
	{
		// get normals, light direction and look direction for each vertex
		Vector3f normal = model->normal(iface, nthvert);
		normal.Normalize();
		Vector3f vertexPos = model->vert(iface, nthvert);

		// calculate light intensity
		intensity.x = std::max(0.f, normal.Dot(lightDir));
		// Вектор для вычисления фрагмента (видимая компонента, направление на наблюдателя)
		camera = (eye - vertexPos);
		camera.Normalize();

		switch (nthvert)
		{
		case 0:
			uv(0, 0) = model->uv(iface, nthvert).x;
			uv(1, 0) = model->uv(iface, nthvert).y;
			break;
		case 1:
			uv(0, 1) = model->uv(iface, nthvert).x;
			uv(1, 1) = model->uv(iface, nthvert).y;
			break;
		case 2:
			uv(0, 2) = model->uv(iface, nthvert).x;
			uv(1, 2) = model->uv(iface, nthvert).y;
			break;
		}

		DirectX::SimpleMath::Vector4 gl_Vertex = embed(model->vert(iface, nthvert)); // read vertex
		return GetFirstColumn(Viewport * Projection * ModelView * VecToMatrix(gl_Vertex)); // transform it to screen coordinates
	}

	virtual bool fragment(Vector3f bar, TGAColor& color)
	{
		float outIntensity = intensity.Dot(bar);
		Vector2f outUV = Vector2f(uv.Right().Dot(bar), uv.Up().Dot(bar));

		Vector3f normal = model->normal(outUV); // get normal to uv
		normal.Normalize();
		Vector3f lightReflection = lightDir * 2 * normal.Dot(lightDir) - lightDir;

		float ambient = PhongCoef.x; // get ambient
		float diffuse = PhongCoef.y * std::max(0.f, normal.Dot(lightDir)); // get diffuse
		float specular = PhongCoef.z * std::pow(std::max(0.f, lightReflection.Dot(camera)), PhongCoef.w); // get specular
		// final illumination by phong
		color = model->diffuse(outUV) * (diffuse + ambient + specular);
		return false;
	}
};

// input arguments
struct Arguments
{ 
	std::string inputFile;
	// if not stated, output is inputFileName.tga
	std::string outputFile;
	float light[3] = { 0, 0, 1 };
	float center[3] = { 0, 0, 0 };
	float camera[3] = { 0, 0, 1 };
	float up[3] = { 0, 1, 0 };
	int width = 800;
	int height = 800;
	std::string raster = "barycentric";
	std::string shader = "phong";
	// coefs for phong - ambient/diffuse/specular/shininess
	Vector4f phongcoef = Vector4f(0.5f, 0.5f, 0.5f, 0.5f);
	// if not stated, doesn't dump z-buffer
	std::string dumpZBufferFile;
};

// check if string ends with a required suffix - for checking extensions
bool endsWith(const std::string& str, const std::string& suffix)
{
	return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

// puts extension if not stated
void validateAndSetFile(std::string& file, const std::string& defaultExtension)
{
	if (!endsWith(file, defaultExtension))
		file += defaultExtension;
}
// checks for a valid rasterization type
void validateRaster(const std::string& raster)
{
	if (raster != "linesweep" && raster != "barycentric")
	{
		throw std::invalid_argument("Invalid rasterization type. Must be 'linesweep' or 'barycentric'.");
	}
}
// checks for a valid shader type
void validateShader(const std::string& shader)
{
	if (shader != "gouraud" && shader != "phong")
	{
		throw std::invalid_argument("Invalid shader type. Must be 'gouraud' or 'phong'.");
	}
}
// processes non-valid floats
float parseFloat(const std::string& str)
{
	try
	{
		return std::stof(str);
	}
	catch (std::invalid_argument&)
	{
		throw std::invalid_argument("Invalid numerical value: " + str);
	}
}
// processes non-valid integers
int parseInt(const std::string& str)
{
	try
	{
		return std::stoi(str);
	}
	catch (std::invalid_argument&)
	{
		throw std::invalid_argument("Invalid integer value: " + str);
	}
}
// peak of c engineering help window
void displayHelp(char* programName)
{
	std::cout << "Usage: " << programName << " input_file [options]\n";
	std::cout << "Options:\n";
	std::cout << "  -o file              Output file\n";
	std::cout << "  --light x y z        Light position | default: 0 0 1\n";
	std::cout << "  --center x y z       Center position | default: 0 0 0\n";
	std::cout << "  --camera x y z       Camera position | default: 0 0 1\n";
	std::cout << "  --up x y z           Up vector direction | default: 0 1 0\n";
	std::cout << "  --width x            Width of the output | default: 800\n";
	std::cout << "  --height x           Height of the output | default: 800\n";
	std::cout << "  --raster type        Rasterization type (linesweep/barycentric) | default: barycentric\n";
	std::cout << "  --shader type        Shader type (gouraud/phong) - always gouraud for linesweep | default for barycentric: phong\n";
	std::cout << "  --phongcoef x y z w  Set Phong lighting coefficients in order - Ambient, Diffuse, Specular, Shininess | default: 0.5 0.5 0.5 0.5\n";
	std::cout << "  --dumpZBuffer file   Dump Z-buffer file\n";
	std::cout << "  --help               Display this help message\n";
}

// parses argv
void parseArguments(int argc, char* argv[], Arguments& args)
{
	// don't allow ignoring the input name
	if (argc < 1)
	{
		std::cerr << "Usage: " << argv[0] << " input_file [options]\n";
		std::cerr << "Use: " << argv[0] << " --help for more options\n";
		exit(EXIT_FAILURE);
	}
	if (argc < 2)
		return;

	// --help is prioritized
	if (std::string(argv[1]) == "--help")
	{
		displayHelp(argv[0]);
		exit(EXIT_SUCCESS);
	}

	args.inputFile = argv[1];
	if (!endsWith(args.inputFile, ".obj"))
	{
		throw std::invalid_argument("Input file must have a .obj extension.");
	}

	for (int i = 2; i < argc; i++)
	{
		std::string arg = argv[i];

		if (arg == "-o" && i + 1 < argc)
		{
			args.outputFile = argv[++i];
			validateAndSetFile(args.outputFile, ".tga");
		}
		else if (arg == "--light" && i + 3 < argc)
		{
			args.light[0] = parseFloat(argv[++i]);
			args.light[1] = parseFloat(argv[++i]);
			args.light[2] = parseFloat(argv[++i]);
		}
		else if (arg == "--center" && i + 3 < argc)
		{
			args.center[0] = parseFloat(argv[++i]);
			args.center[1] = parseFloat(argv[++i]);
			args.center[2] = parseFloat(argv[++i]);
		}
		else if (arg == "--camera" && i + 3 < argc)
		{
			args.camera[0] = parseFloat(argv[++i]);
			args.camera[1] = parseFloat(argv[++i]);
			args.camera[2] = parseFloat(argv[++i]);
		}
		else if (arg == "--up" && i + 3 < argc)
		{
			args.up[0] = parseFloat(argv[++i]);
			args.up[1] = parseFloat(argv[++i]);
			args.up[2] = parseFloat(argv[++i]);
		}
		else if (arg == "--width" && i + 1 < argc)
		{
			args.width = parseInt(argv[++i]);
		}
		else if (arg == "--height" && i + 1 < argc)
		{
			args.height = parseInt(argv[++i]);
		}
		else if (arg == "--raster" && i + 1 < argc)
		{
			args.raster = argv[++i];
			validateRaster(args.raster);
		}
		else if (arg == "--shader" && i + 1 < argc)
		{
			args.shader = argv[++i];
			validateShader(args.shader);
		}
		else if (arg == "--phongcoef" && i + 4 < argc)
		{
			args.phongcoef.x = parseFloat(argv[++i]);
			args.phongcoef.y = parseFloat(argv[++i]);
			args.phongcoef.z = parseFloat(argv[++i]);
			args.phongcoef.w = parseFloat(argv[++i]);
		}
		else if (arg == "--dumpZBuffer" && i + 1 < argc)
		{
			args.dumpZBufferFile = argv[++i];
			validateAndSetFile(args.dumpZBufferFile, ".tga");
		}
		else
		{
			std::cerr << "Unknown option: " << arg << "\n";
			exit(EXIT_FAILURE);
		}
	}
	// if output name is not stated,output is inputFileName.tga
	if (args.outputFile.empty())
	{
		args.outputFile = args.inputFile.substr(0, args.inputFile.find_last_of('.')) + ".tga";
	}
}

int main(int argc, char** argv)
{
	Arguments args;

	try
	{
		parseArguments(argc, argv, args);
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << "\n";
		return EXIT_FAILURE;
	}

	/// /// /// /// /// /// /// /// ///
	/// INITIALIZED BASIC VARIABLES ///
	/// /// /// /// /// /// /// /// ///
	model = new Model(args.inputFile.c_str());
	lightDir = Vector3f(args.light[0], args.light[1], args.light[2]);
	center = Vector3f(args.center[0], args.center[1], args.center[2]);
	eye = Vector3f(args.camera[0], args.camera[1], args.camera[2]);
	up = Vector3f(args.up[0], args.up[1], args.up[2]);
	width = args.width;
	height = args.height;
	PhongCoef = args.phongcoef;

	ModelView = lookat(eye, center, up);
	Projection = Matrix::Identity;
	Viewport = viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);

	// output image
	TGAImage image(width, height, TGAImage::RGB);

	if (args.raster == "barycentric")
	{
		std::cerr << "[Render] Barycentric rasterization | ";
		RasterBarycentric* render = new RasterBarycentric();
		TGAImage zBuffer(width, height, TGAImage::GRAYSCALE);
		
		if (args.shader == "gouraud")
		{
			std::cerr << "Gouraud Shader\n";
			GouraudShader shader;
			lightDir.Normalize(); // we're setting the light direction, not exact coordinates
			// apply shader for each vertex of each face
			for (int i = 0; i < model->nfaces(); i++)
			{
				DirectX::SimpleMath::Vector4 screenCoords[3];
				for (int j = 0; j < 3; j++)
					screenCoords[j] = shader.vertex(i, j);
				// draws each face
				render->triangle(screenCoords, shader, image, zBuffer);
			}
		}
		else if (args.shader == "phong")
		{
			std::cerr << "Phong Shader\n";
			PhongShader shader;
			// apply shader for each vertex of each face
			for (int i = 0; i < model->nfaces(); i++)
			{
				DirectX::SimpleMath::Vector4 screenCoords[3];
				for (int j = 0; j < 3; j++)
					screenCoords[j] = shader.vertex(i, j);
				// draws each face
				render->triangle(screenCoords, shader, image, zBuffer);
			}
		}
		// if dumpZBuffer file is states - dumps zBuffer duh
		if (!args.dumpZBufferFile.empty())
		{
			zBuffer.flip_vertically();
			zBuffer.write_tga_file(args.dumpZBufferFile.c_str());
			std::cerr << "[Z-Buffer] Dump saved at " << args.dumpZBufferFile << "\n";
		}
	}
	else if (args.raster == "linesweep")
	{
		std::cerr << "[Render] Line Sweep rasterization\n";
		RasterLinesweep* render = new RasterLinesweep(model, width, height);

		// no idea why i'm still keeping it int
		// todo change to TGAImage later
		int* zBuffer = new int[width * height];
		for (int i = 0; i < width * height; i++)
			zBuffer[i] = INT_MIN;

		lightDir.Normalize(); // we're setting the light direction, not exact coordinates

		for (int i = 0; i < model->nfaces(); i++)
		{
			std::vector<int> face = model->face(i);

			// transforms vertices to screen coordinates
			Vector3i screenCoords[3];
			Vector3f worldCoords[3];
			for (int j = 0; j < 3; j++)
			{
				worldCoords[j] = model->vert(face[j]);
				screenCoords[j] = FloatToInt(MatrixToVec(Viewport * Projection * ModelView * VecToMatrix(worldCoords[j])));
			}
			// applies gouraud shading (finds normals for each vertex)
			Vector2f uv[3];
			float intensity[3];
			for (int j = 0; j < 3; j++)
			{
				uv[j] = model->uv(i, j);
				intensity[j] = std::clamp(model->normal(i, j).Dot(lightDir), 0.0f, 1.0f);
			}
			// draws each face
			render->triangle(screenCoords[0], screenCoords[1], screenCoords[2], uv[0], uv[1], uv[2], image, intensity, zBuffer);
		}
		// if dumpZBuffer file is states - dumps zBuffer duh
		if (!args.dumpZBufferFile.empty())
		{
			TGAImage zbImage(width, height, TGAImage::GRAYSCALE);

			for (int i = 0; i < width; i++)
			{
				for (int j = 0; j < height; j++)
					zbImage.set(i, j, TGAColor(zBuffer[i + j * width], 1));
			}
			zbImage.flip_vertically();
			zbImage.write_tga_file(args.dumpZBufferFile.c_str());
			std::cerr << "[Z-Buffer] Dump saved at " << args.dumpZBufferFile << "\n";
		}
	}

	// flip the image, so it starts in the bottom left corner
	image.flip_vertically();
	// writing our image to a tga file
	image.write_tga_file(args.outputFile.c_str());
	std::cerr << "[Render] Output saved at " << args.outputFile << "\n";
	// some cleanup
	delete model;
	return 0;
}

