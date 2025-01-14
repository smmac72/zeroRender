# Zero Renderer ![cpp win](https://github.com/smmac72/zeroRender/actions/workflows/cpp.yml/badge.svg)

  

  

A very small util for rendering .obj files in .tga format. Uses DXMath and DX SimpleMath (mostly targeting the latter, but seeing as SimpleMath sucks as a DXMath wrapper)

  

  

Supports two rasterization types - Line Sweep and via the Barycentric coordinates. Both algorithms support shading - Gouraud for the Line Sweep and Gouraud/Phong for Barycentric. Shading can be turned off.

  

Includes ability to set scene parameters (check **"Usage"** for more info)

  

  

## Usage

[DOWNLOAD BUILD](https://github.com/smmac72/basicRender/releases/tag/v1)

  

**./zr filename.obj** - you can include as many .obj files as you wish. At least one is required.

  

  

Automatically links **filename_diffuse.tga** as the diffuse texture. If not present, the result will be unlit.

Automatically links **filename_nm.tga** and **filename_spec.tga** as the normal/specular textures (for Phong shading only). If any of the two not present, shading will be switched to Gouraud.

**Options:**

  

-  **./zr -\-help** - Display help message

  

-  **-o fileName(.tga)** - Output filename. Can ignore the type, fills automatically. If not stated, saved as inputFilename.tga

  

-  **-\-light x y z** - Light source direction. Default: 0 0 1

  

-  **-\-center x y z** - Lookat position. Default: 0 0 0

  

-  **-\-camera x y z** - Camera position. Default: 0 0 1

  

-  **-\-up x y z** - Up vector direction. Default: 0 1 0

  

-  **-\-width x** - Width of the output. Default: 800

  

-  **-\-height x** - Height of the output. Default: 800

  

-  **-\-raster type** - Rasterization type (linesweep/barycentric). Default: barycentric
	- Linesweep algorithm does not support Phong shading

  

-  **-\-shader type (none/gouraud/phong)** - Sets active shader
	- None - no shading, just renders the model
	- Gouraud - default for Linesweep rasterization algorithm
	- Phong - default for Barycentric rasterization algorithm. Unavailable for Linesweep algorithm and models without the any of the three given textures
![model preview with different shader settings](https://i.imgur.com/SDF8JOb.png)
  

-  **-\-phongcoef x y z w** - Set Phong lighting coefficients in order - Ambient, Diffuse, Specular. Default: 5 0.5 0.35

  

-  **--dumpZBuffer file** - Dump Z-buffer file. If not stated, doesn't do the dumping

  

## Examples
Repo includes three test input files
- **model.obj** with **model_eye.obj** (stolen for education purposes) - includes diffuse, normal and specular textures
- **smmac.dff** (peak 3d design) - includes diffuse texture only

.obj files are not included in the build due to Windows recognizing them as a virus. lmao.

## Build

Windows only, just use make