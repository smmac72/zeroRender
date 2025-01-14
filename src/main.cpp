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
// local object coordinates to world to view transformation
Matrix ModelView;
// world coordinates to camera transformation
Matrix Projection;
// camera coodrinates to screen transformation
Matrix Viewport;
// light direction
Vector3f lightDir;
// phong coefs - ambient/diffuse/specular/shininess
Vector3f PhongCoef;

// gouraud shading shader
struct GouraudShader : public IShader
{
	Vector3f intensity;
	Matrix uv; // can be 2x3 matrix (three pairs of uvs), but simplemath multiplication said no

	// get intensity and uv for each vertex in vertex shader
	virtual Vector4f vertex(int iface, int nthvert)
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
		Vector4f gl_Vertex = embed(model->vert(iface, nthvert)); // read vertex
        return MatrixToVec4(Viewport*Projection*ModelView*VecToMatrix(gl_Vertex)); // transform it to screen coordinates
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
	Matrix uv;
	Matrix Camera;   //  camera transformation for light calculations - Projection*ModelView
	Matrix Camera_IT; // camera transformation inverted and transposed for normal calculations (no idea why tbh) - (Projection*ModelView) Invert + Transpose
	bool bHasTextures;

    virtual Vector4f vertex(int iface, int nthvert)
	{
		switch (nthvert)
		{
		case 0:
			// get vertex uv
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
		Vector4f gl_Vertex = embed(model->vert(iface, nthvert)); // read vertex
        return MatrixToVec4(Viewport*Projection*ModelView*VecToMatrix(gl_Vertex)); // transform it to screen coordinates
    }

    virtual bool fragment(Vector3f bar, TGAColor &color)
	{
		Vector2f outUV = Vector2f(uv.Right().Dot(bar), uv.Up().Dot(bar));

		// rotated normal relative to camera
		Vector3f n = Vector3f(GetMatrixRow(Camera_IT, 0).Dot(embed(model->normal(outUV))),
					GetMatrixRow(Camera_IT, 1).Dot(embed(model->normal(outUV))),
					GetMatrixRow(Camera_IT, 2).Dot(embed(model->normal(outUV))));
		n.Normalize();

		// rotated lighting relative to camera 
		Vector3f l = Vector3f(GetMatrixRow(Camera, 0).Dot(embed(lightDir)),
					GetMatrixRow(Camera, 1).Dot(embed(lightDir)),
					GetMatrixRow(Camera, 2).Dot(embed(lightDir)));
		l.Normalize();

		Vector3f r = (n*(n.Dot(l)*2.f) - l); // reflected light
		r.Normalize();

		float spec = pow(std::max(r.z, 0.0f), model->specular(outUV)); // if reflects light, reflection^specularvalue
		float diff = std::max(0.f, n.Dot(l)); // calculates lighting

		TGAColor c = model->diffuse(outUV);
		color = c;
		// ambient + color(diffuse + specular)
		color.b = std::min<float>(PhongCoef.x + c.b*(PhongCoef.y*diff + PhongCoef.z*spec), 255);
		color.g = std::min<float>(PhongCoef.x + c.g*(PhongCoef.y*diff + PhongCoef.z*spec), 255);
		color.r = std::min<float>(PhongCoef.x + c.r*(PhongCoef.y*diff + PhongCoef.z*spec), 255);

        return false;
    }
};

// just render the model
struct NoneShader : public IShader
{
	Matrix uv;

    virtual Vector4f vertex(int iface, int nthvert)
	{
		switch (nthvert)
		{
		case 0:
			// get vertex uv
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
        return MatrixToVec4(Viewport*Projection*ModelView*VecToMatrix(gl_Vertex)); // transform it to screen coordinates
    }

    virtual bool fragment(Vector3f bar, TGAColor &color)
	{
        Vector2f outUV = Vector2f(uv.Right().Dot(bar), uv.Up().Dot(bar));
        color = model->diffuse(outUV);
        return false;
    }
};

// input arguments
struct Arguments
{ 
	std::vector<std::string> inputFile;
	// if not stated, output is inputFileName.tga
	std::string outputFile;
	float light[3] = { 0, 0, 1 };
	float center[3] = { 0, 0, 0 };
	float camera[3] = { 0, 0, 3 };
	float up[3] = { 0, 1, 0 };
	int width = 800;
	int height = 800;
	std::string raster = "barycentric";
	std::string shader = "phong";
	// coefs for phong - ambient/diffuse/specular/shininess
	Vector3f phongcoef = Vector3f(5.0f, 0.5f, 0.35f);
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
	if (shader != "none" && shader != "gouraud" && shader != "phong")
	{
		throw std::invalid_argument("Invalid shader type. Must be 'none', 'gouraud' or 'phong'.");
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
// gets file name from a path
std::string getFileName(const std::string& filePath)
{
    size_t pos = filePath.find_last_of("/\\");
    if (pos != std::string::npos)
	{
        return filePath.substr(pos + 1);
    }
    return filePath; // Return the whole string if no slash is found
}
// peak of c engineering help window
void displayHelp(char* programName)
{
	std::cout << "Usage: " << programName << " input_files [options]\n";
	std::cout << "Options:\n";
	std::cout << "  -o file              Output file\n";
	std::cout << "  --light x y z        Light position | default: 0 0 1\n";
	std::cout << "  --center x y z       Center position | default: 0 0 0\n";
	std::cout << "  --camera x y z       Camera position | default: 0 0 1\n";
	std::cout << "  --up x y z           Up vector direction | default: 0 1 0\n";
	std::cout << "  --width x            Width of the output | default: 800\n";
	std::cout << "  --height x           Height of the output | default: 800\n";
	std::cout << "  --raster type        Rasterization type (linesweep/barycentric) | default: barycentric\n";
	std::cout << "  --phongcoef x y z w  Set Phong lighting coefficients in order - Ambient, Diffuse, Specular, Shininess | default: 5 0.5 0.35\n";
	std::cout << "  --dumpZBuffer file   Dump Z-buffer file\n";
	std::cout << "  --help               Display this help message\n";
}

// parses argv
void parseArguments(int argc, char* argv[], Arguments& args)
{
	// don't allow ignoring the input name
	if (argc < 2)
	{
		std::cerr << "Usage: " << getFileName(argv[0]) << " input_file [options]\n";
		std::cerr << "Use: " << getFileName(argv[0]) << " --help for more options\n";
		exit(EXIT_FAILURE);
	}

	// --help is prioritized
	if (std::string(argv[1]) == "--help")
	{
		displayHelp(argv[0]);
		exit(EXIT_SUCCESS);
	}

	for (int i = 1; i < argc; i++)
	{
		std::string arg = argv[i];

		if (endsWith(arg, ".obj"))
		{
			args.inputFile.push_back(arg);
		}
		else if (arg == "-o" && i + 1 < argc)
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
		else if (arg == "--phongcoef" && i + 3 < argc)
		{
			args.phongcoef.x = parseFloat(argv[++i]);
			args.phongcoef.y = parseFloat(argv[++i]);
			args.phongcoef.z = parseFloat(argv[++i]);
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
	// if no input files are stated
	if (args.inputFile.size() == 0)
	{
		std::cerr << "No input files specified\n";
		exit(EXIT_FAILURE);
	}
	// if output name is not stated,output is firstInputFileName.tga
	if (args.outputFile.empty())
	{
		args.outputFile = args.inputFile[0].substr(0, args.inputFile[0].find_last_of('.')) + ".tga";
	}
}

int main(int argc, char** argv)
{
	// input arguments
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
	std::vector<Model*> models;
	for (const auto& inputFile : args.inputFile)
	{
		Model* temp = new Model(inputFile.c_str());
		if (!temp || temp->nfaces() == 0)
		{
			std::cerr << "Invalid model file: " << inputFile << "\n";
			return EXIT_FAILURE;
		}
		models.push_back(temp);
    }

	lightDir = Vector3f(args.light[0], args.light[1], args.light[2]);
	center = Vector3f(args.center[0], args.center[1], args.center[2]);
	eye = Vector3f(args.camera[0], args.camera[1], args.camera[2]);
	up = Vector3f(args.up[0], args.up[1], args.up[2]);
	width = args.width;
	height = args.height;
	PhongCoef = args.phongcoef;

	ModelView = lookat(eye, center, up);
	Projection = Matrix(Vector4f(1,0,0,0), Vector4f(0, 1, 0, 0), Vector4f(0, 0, 1, 0), Vector4f(0, 0, -1.0f/(center - eye).Length(), 1));
	Viewport = viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);

	if (args.shader == "phong" && args.raster == "linesweep")
	{
		std::cerr << "[Input] Phong shading is not supported on a Linesweep rasterization algorithm. Switching to Gouraud shading\n";
		args.shader = "gouraud";
	}
	
	// output image
	TGAImage image(width, height, TGAImage::RGB);
	TGAImage zBuffer(width, height, TGAImage::GRAYSCALE);
	
	for (const auto& tempmodel : models)
	{
		model = tempmodel;
		if (args.shader == "phong" && model->diffusemap().get_width() == 0)
		{
			std::cerr << "[Input - " << model->getFileName() << "] Phong shading is not supported without a diffuse texture. Switching to Gouraud shading\n";
			args.shader = "gouraud";
		}
		if (args.shader == "phong" && model->normalmap().get_width() == 0)
		{
			std::cerr << "[Input - " << model->getFileName() << "] Phong shading is not supported without a normal texture. Switching to Gouraud shading\n";
			args.shader = "gouraud";
		}
		if (args.shader == "phong" && model->specularmap().get_width() == 0)
		{
			std::cerr << "[Input - " << model->getFileName() << "] Phong shading is not supported without a specular texture. Switching to Gouraud shading\n";
			args.shader = "gouraud";
		}
		if (args.raster == "barycentric")
		{
			std::cerr << "[Render] Barycentric rasterization | ";
			RasterBarycentric* render = new RasterBarycentric();
			
			bool bHasTextures = model->diffusemap().get_width() != 0; // to allow unlit rendering
			if (!bHasTextures)
				std::cerr << "Unlit Mode | ";
			if (args.shader == "phong")
			{
				std::cerr << "Phong Shader\n";
				PhongShader shader;
				// we can pass constants into the shader if needed
				shader.Camera = Projection * ModelView;
				shader.Camera_IT = (Projection * ModelView).Invert().Transpose();
				shader.bHasTextures = bHasTextures;
				for (int i = 0; i < model->nfaces(); i++) // apply shader for each vertex of each face
				{
					DirectX::SimpleMath::Vector4 screenCoords[3];
					for (int j = 0; j < 3; j++)
						screenCoords[j] = shader.vertex(i, j);
					render->triangle(screenCoords, shader, image, zBuffer, bHasTextures); // draws each face
				}
			}
			else if (args.shader == "gouraud")
			{
				std::cerr << "Gouraud Shader\n";
				GouraudShader shader;
				for (int i = 0; i < model->nfaces(); i++) // apply shader for each vertex of each face
				{
					DirectX::SimpleMath::Vector4 screenCoords[3];
					for (int j = 0; j < 3; j++)
					{
						screenCoords[j] = shader.vertex(i, j);
						//std::cerr << screenCoords[j].x << " " << screenCoords[j].y << " " << screenCoords[j].z << " " << screenCoords[j].w << "\n";
					}
					render->triangle(screenCoords, shader, image, zBuffer, bHasTextures); // draws each face
				}
			}
			else if (args.shader == "none")
			{
				std::cerr << "No shader\n";
				NoneShader shader;
				for (int i = 0; i < model->nfaces(); i++) // apply shader for each vertex of each face
				{
					DirectX::SimpleMath::Vector4 screenCoords[3];
					for (int j = 0; j < 3; j++)
						screenCoords[j] = shader.vertex(i, j);
					render->triangle(screenCoords, shader, image, zBuffer, bHasTextures); // draws each face
				}
			}
		}
		else if (args.raster == "linesweep")
		{
			std::cerr << "[Render] Line Sweep rasterization with" << (args.shader == "gouraud" ? " Gouraud shading\n" : "out shading\n");
			RasterLinesweep* render = new RasterLinesweep(model, width, height);


			lightDir.Normalize(); // we're setting the light direction, not exact coordinates

			bool bHasTextures = model->diffusemap().get_width() != 0; // to allow unlit rendering
			if (!bHasTextures)
				std::cerr << "[Render] Rendering in unlit mode - no texture found\n";
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
					intensity[j] = args.shader == "gouraud" ? std::clamp(model->normal(i, j).Dot(lightDir), 0.0f, 1.0f) : 1.0f;
				}
				// draws each face
				render->triangle(screenCoords[0], screenCoords[1], screenCoords[2], uv[0], uv[1], uv[2], image, intensity, zBuffer, bHasTextures);
			}
		}
	}
	
	// if dumpZBuffer file is stated - dumps zBuffer duh
	if (!args.dumpZBufferFile.empty())
	{
		zBuffer.flip_vertically();
		zBuffer.write_tga_file(args.dumpZBufferFile.c_str());
		std::cerr << "[Z-Buffer] Dump saved at " << args.dumpZBufferFile << "\n";
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

